#!/usr/bin/env python3
"""Raw TCP vertical-slice smoke checks for the local server executable."""

from __future__ import annotations

import json
import socket
import struct
import subprocess
import sys
import tempfile
import time
from pathlib import Path


ROOT = Path(__file__).resolve().parents[3]
SERVER_EXE = ROOT / "build" / ("battle_server_app.exe" if sys.platform == "win32" else "battle_server_app")
MAX_FRAME_BYTES = 8192


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def free_port() -> int:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as probe:
        probe.bind(("127.0.0.1", 0))
        return int(probe.getsockname()[1])


def write_config(path: Path, port: int) -> None:
    config = {
        "mode": "local",
        "server": {
            "tickRate": 20,
            "snapshotRate": 10,
            "maxConnections": 16,
            "maxMatches": 8,
        },
        "transports": {
            "tcp": {
                "enabled": True,
                "host": "127.0.0.1",
                "port": port,
                "maxFrameBytes": MAX_FRAME_BYTES,
            },
            "websocket": {
                "enabled": False,
                "host": "127.0.0.1",
                "port": 8081,
                "path": "/ws",
                "maxMessageBytes": MAX_FRAME_BYTES,
                "requireTls": False,
            },
        },
        "security": {
            "demoAuthEnabled": True,
            "telegramAuthEnabled": False,
            "handshakeTimeoutMs": 500,
            "idleTimeoutMs": 70000,
            "maxInputCommandsPerSecond": 30,
            "maxPendingWriteBytesPerSession": 1048576,
            "maxPendingOutboundMessages": 256,
            "maxPendingCommandsPerMatch": 16,
            "maxCommandsPerTick": 16,
            "maxPendingCommandsPerSession": 8,
        },
        "metrics": {"logEverySeconds": 5},
    }
    path.write_text(json.dumps(config), encoding="utf-8")


def connect_retry(port: int, timeout_seconds: float = 5.0) -> socket.socket:
    deadline = time.monotonic() + timeout_seconds
    last_error: OSError | None = None
    while time.monotonic() < deadline:
        try:
            sock = socket.create_connection(("127.0.0.1", port), timeout=1.0)
            sock.settimeout(2.0)
            return sock
        except OSError as exc:
            last_error = exc
            time.sleep(0.05)
    raise AssertionError(f"failed to connect to local server: {last_error}")


def send_frame(sock: socket.socket, payload: str | dict[str, object]) -> None:
    if not isinstance(payload, str):
        payload = json.dumps(payload, separators=(",", ":"))
    encoded = payload.encode("utf-8")
    sock.sendall(struct.pack(">I", len(encoded)) + encoded)


def recv_frame(sock: socket.socket) -> dict[str, object]:
    header = sock.recv(4)
    require(len(header) == 4, "connection closed before frame header")
    (size,) = struct.unpack(">I", header)
    payload = b""
    while len(payload) < size:
        chunk = sock.recv(size - len(payload))
        require(bool(chunk), "connection closed before frame payload")
        payload += chunk
    return json.loads(payload.decode("utf-8"))


def expect_type(sock: socket.socket, expected_type: str) -> dict[str, object]:
    frame = recv_frame(sock)
    require(frame.get("type") == expected_type, f"expected {expected_type}, got {frame}")
    return frame


def auth(sock: socket.socket, name: str) -> None:
    send_frame(sock, {"version": 1, "type": "auth_request", "payload": {"mode": "demo", "displayName": name}})
    expect_type(sock, "auth_result")


def read_until(sock: socket.socket, expected_type: str, attempts: int = 4) -> dict[str, object]:
    for _ in range(attempts):
        frame = recv_frame(sock)
        if frame.get("type") == expected_type:
            return frame
    raise AssertionError(f"did not receive {expected_type}")


def happy_two_client_flow(port: int) -> None:
    create = connect_retry(port)
    join = connect_retry(port)
    try:
        auth(create, "tcp-one")
        send_frame(
            create,
            {
                "version": 1,
                "type": "create_match",
                "payload": {"mode": "objective_run", "scenario": "arena_small_objective_run"},
            },
        )
        created = expect_type(create, "match_joined")
        code = str(created["payload"]["matchCode"])

        auth(join, "tcp-two")
        send_frame(join, {"version": 1, "type": "join_match", "payload": {"matchCode": code}})
        joined = expect_type(join, "match_joined")
        match_id = str(joined["payload"]["matchId"])
        read_until(create, "snapshot")
        read_until(join, "snapshot")

        send_frame(
            join,
            {
                "version": 1,
                "type": "input_command",
                "sessionSeq": 1,
                "payload": {"matchId": match_id, "command": {"kind": "move", "direction": {"x": -1, "y": 0}}},
            },
        )
        ack = expect_type(join, "input_ack")
        require(ack["payload"]["accepted"] is True, f"input command rejected: {ack}")
        read_until(join, "snapshot")
    finally:
        create.close()
        join.close()


def malformed_json_is_rejected(port: int) -> None:
    with connect_retry(port) as sock:
        send_frame(sock, "{not-json")
        error = expect_type(sock, "error")
        require(error["payload"]["code"] == "protocol_error", f"unexpected malformed JSON error: {error}")


def unknown_type_is_rejected(port: int) -> None:
    with connect_retry(port) as sock:
        send_frame(sock, {"version": 1, "type": "unknown_type", "payload": {}})
        error = expect_type(sock, "error")
        require(error["payload"]["code"] == "protocol_error", f"unexpected unknown type error: {error}")


def oversized_frame_is_closed_before_payload(port: int) -> None:
    with connect_retry(port) as sock:
        sock.sendall(struct.pack(">I", MAX_FRAME_BYTES + 1))
        closed = sock.recv(1)
        require(closed == b"", "oversized frame did not close connection")


def handshake_timeout_is_rejected(port: int) -> None:
    with connect_retry(port) as sock:
        error = expect_type(sock, "error")
        require(error["payload"]["code"] == "handshake_timeout", f"unexpected handshake timeout error: {error}")


def main() -> int:
    if not SERVER_EXE.exists():
        print(f"missing server executable: {SERVER_EXE}", file=sys.stderr)
        return 2

    port = free_port()
    with tempfile.TemporaryDirectory(prefix="if-arena-tcp-") as temp:
        config = Path(temp) / "server.local.json"
        write_config(config, port)
        server = subprocess.Popen(
            [str(SERVER_EXE), "--config", str(config), "--max-clients", "6"],
            cwd=ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
        try:
            happy_two_client_flow(port)
            malformed_json_is_rejected(port)
            unknown_type_is_rejected(port)
            oversized_frame_is_closed_before_payload(port)
            handshake_timeout_is_rejected(port)
            stdout, stderr = server.communicate(timeout=10)
            require(server.returncode == 0, f"server exited {server.returncode}\n{stdout}\n{stderr}")
        finally:
            if server.poll() is None:
                server.terminate()
                server.communicate(timeout=5)

    print("[PASS] tcp_vertical_slice_smoke")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
