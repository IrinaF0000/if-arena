#!/usr/bin/env python3
from __future__ import annotations

import json
import os
import socket
import subprocess
import sys
import tempfile
import time
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "tests" / "load"))

from local_tcp_load_scenarios import TcpClient, wait_for_server  # noqa: E402


SERVER = ROOT / "build" / "battle_server_app.exe"
SENSITIVE_SENTINEL = "SENSITIVE_LOG_SENTINEL_DO_NOT_PRINT"


def start_server(port: int) -> tuple[subprocess.Popen[str], Path]:
    config = {
        "mode": "local",
        "server": {"tickRate": 20, "snapshotRate": 10, "maxConnections": 16, "maxMatches": 8},
        "transports": {
            "tcp": {"enabled": True, "host": "127.0.0.1", "port": port, "maxFrameBytes": 65536},
            "websocket": {
                "enabled": False,
                "host": "127.0.0.1",
                "port": 8081,
                "path": "/ws",
                "maxMessageBytes": 65536,
                "requireTls": False,
            },
        },
        "security": {
            "demoAuthEnabled": True,
            "telegramAuthEnabled": False,
            "handshakeTimeoutMs": 5000,
            "idleTimeoutMs": 30000,
            "maxInputCommandsPerSecond": 30,
            "maxPendingWriteBytesPerSession": 1048576,
            "maxPendingOutboundMessages": 64,
            "maxPendingCommandsPerMatch": 128,
            "maxCommandsPerTick": 64,
            "maxPendingCommandsPerSession": 64,
        },
        "metrics": {"logEverySeconds": 5},
    }
    handle = tempfile.NamedTemporaryFile("w", encoding="utf-8", suffix=".json", delete=False)
    with handle:
        json.dump(config, handle)
    process = subprocess.Popen(
        [str(SERVER), "--config", handle.name, "--max-clients", "7"],
        cwd=ROOT,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )
    wait_for_server("127.0.0.1", port)
    return process, Path(handle.name)


def stop_server(process: subprocess.Popen[str], config_path: Path) -> str:
    output = ""
    try:
        output, _ = process.communicate(timeout=1.0)
    except subprocess.TimeoutExpired:
        process.terminate()
        try:
            output, _ = process.communicate(timeout=2.0)
        except subprocess.TimeoutExpired:
            process.kill()
            output, _ = process.communicate(timeout=2.0)
    try:
        os.unlink(config_path)
    except OSError:
        pass
    return output


def expect_error(client: TcpClient, expected_fragment: str, label: str) -> None:
    try:
        received = client.read_envelope()
    except socket.timeout as exc:
        raise AssertionError(f"{label}: timed out waiting for error") from exc
    if received.get("type") != "error":
        raise AssertionError(f"{label}: expected error envelope, got {received}")
    payload = received.get("payload", {})
    message = str(payload.get("message", "")) + " " + str(payload.get("code", ""))
    if expected_fragment not in message:
        raise AssertionError(f"{label}: expected {expected_fragment!r} in {message!r}")


def send_raw_json(client: TcpClient, payload: dict[str, object]) -> None:
    client.send_raw(json.dumps(payload, separators=(",", ":")).encode("utf-8"))


def main() -> int:
    if not SERVER.exists():
        print(f"server executable not found: {SERVER}", file=sys.stderr)
        return 1

    port = 5566
    process, config_path = start_server(port)
    stopped = False
    step = "starting"
    try:
        step = "create before auth"
        before_auth = TcpClient("127.0.0.1", port)
        before_auth.send_envelope("create_match", {"mode": "objective_run", "scenario": "arena_small_objective_run"})
        expect_error(before_auth, "current session phase", "create before auth")
        before_auth.close()

        step = "missing version"
        missing_version = TcpClient("127.0.0.1", port)
        send_raw_json(missing_version, {"type": "auth_request", "payload": {"mode": "demo", "displayName": "missing-version"}})
        expect_error(missing_version, "protocol_error", "missing version")
        missing_version.close()

        step = "wrong payload type"
        wrong_payload = TcpClient("127.0.0.1", port)
        send_raw_json(wrong_payload, {"version": 1, "type": "auth_request", "payload": SENSITIVE_SENTINEL})
        expect_error(wrong_payload, "protocol_error", "wrong payload type")
        wrong_payload.close()

        step = "bad sessionSeq"
        bad_sequence = TcpClient("127.0.0.1", port)
        send_raw_json(
            bad_sequence,
            {
                "version": 1,
                "type": "input_command",
                "sessionSeq": -1,
                "payload": {"matchId": "1", "command": {"kind": "stop"}},
            }
        )
        expect_error(bad_sequence, "protocol_error", "bad sessionSeq")
        bad_sequence.close()

        step = "command before match"
        before_match = TcpClient("127.0.0.1", port)
        before_match.send_envelope("auth_request", {"mode": "demo", "displayName": "before-match"})
        auth_result = before_match.read_envelope()
        if auth_result.get("type") != "auth_result":
            raise AssertionError(f"auth failed before match: {auth_result}")
        before_match.send_envelope(
            "input_command",
            {"matchId": "1", "command": {"kind": "stop"}},
            session_seq=1,
        )
        expect_error(before_match, "current session phase", "command before match")
        before_match.close()

        step = "authority field"
        authority = TcpClient("127.0.0.1", port)
        authority.send_envelope("auth_request", {"mode": "demo", "displayName": "authority-test"})
        step = "authority field auth"
        auth_result = authority.read_envelope()
        if auth_result.get("type") != "auth_result":
            raise AssertionError(f"auth failed: {auth_result}")
        step = "authority field create"
        authority.send_envelope("create_match", {"mode": "objective_run", "scenario": "arena_small_objective_run"})
        joined = authority.read_envelope()
        if joined.get("type") != "match_joined":
            raise AssertionError(f"join failed: {joined}")
        step = "authority field command"
        authority.send_envelope(
            "input_command",
            {
                "matchId": joined["payload"]["matchId"],
                "command": {
                    "kind": "move",
                    "direction": {"x": 0, "y": -1},
                    "playerId": "999",
                    "hp": 999,
                },
            },
            session_seq=1,
        )
        expect_error(authority, "authority field", "authority field")
        authority.close()

        time.sleep(0.1)
        output = stop_server(process, config_path)
        stopped = True
        if SENSITIVE_SENTINEL in output:
            raise AssertionError("server output leaked raw sensitive client payload")
        print("[PASS] tcp_protocol_negative")
        return 0
    except (AssertionError, OSError, socket.timeout, ConnectionError) as exc:
        print(f"[FAIL] tcp_protocol_negative ({step}): {exc}", file=sys.stderr)
        return 1
    finally:
        if not stopped:
            stop_server(process, config_path)


if __name__ == "__main__":
    raise SystemExit(main())
