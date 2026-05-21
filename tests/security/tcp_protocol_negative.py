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


def start_server(port: int) -> tuple[subprocess.Popen[str], Path]:
    config = {
        "mode": "local",
        "server": {"tickRate": 20, "snapshotRate": 10, "maxConnections": 4, "maxMatches": 8},
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
        [str(SERVER), "--config", handle.name, "--max-clients", "4"],
        cwd=ROOT,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )
    wait_for_server("127.0.0.1", port)
    return process, Path(handle.name)


def stop_server(process: subprocess.Popen[str], config_path: Path) -> None:
    try:
        process.wait(timeout=1.0)
    except subprocess.TimeoutExpired:
        process.terminate()
        try:
            process.wait(timeout=2.0)
        except subprocess.TimeoutExpired:
            process.kill()
    try:
        os.unlink(config_path)
    except OSError:
        pass


def expect_error(client: TcpClient, expected_fragment: str) -> None:
    received = client.read_envelope()
    if received.get("type") != "error":
        raise AssertionError(f"expected error envelope, got {received}")
    payload = received.get("payload", {})
    message = str(payload.get("message", "")) + " " + str(payload.get("code", ""))
    if expected_fragment not in message:
        raise AssertionError(f"expected {expected_fragment!r} in {message!r}")


def main() -> int:
    if not SERVER.exists():
        print(f"server executable not found: {SERVER}", file=sys.stderr)
        return 1

    port = 5566
    process, config_path = start_server(port)
    try:
        before_auth = TcpClient("127.0.0.1", port)
        before_auth.send_envelope("create_match", {"mode": "objective_run", "scenario": "arena_small_objective_run"})
        expect_error(before_auth, "current session phase")
        before_auth.close()

        authority = TcpClient("127.0.0.1", port)
        authority.send_envelope("auth_request", {"mode": "demo", "displayName": "authority-test"})
        auth_result = authority.read_envelope()
        if auth_result.get("type") != "auth_result":
            raise AssertionError(f"auth failed: {auth_result}")
        authority.send_envelope("create_match", {"mode": "objective_run", "scenario": "arena_small_objective_run"})
        joined = authority.read_envelope()
        if joined.get("type") != "match_joined":
            raise AssertionError(f"join failed: {joined}")
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
        expect_error(authority, "authority field")
        authority.close()

        time.sleep(0.1)
        print("[PASS] tcp_protocol_negative")
        return 0
    except (AssertionError, OSError, socket.timeout, ConnectionError) as exc:
        print(f"[FAIL] tcp_protocol_negative: {exc}", file=sys.stderr)
        return 1
    finally:
        stop_server(process, config_path)


if __name__ == "__main__":
    raise SystemExit(main())
