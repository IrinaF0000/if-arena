#!/usr/bin/env python3
"""Local browser-path smoke for the C++ WebSocket listener."""

from __future__ import annotations

import base64
import json
import os
import socket
import struct
import subprocess
import tempfile
import time
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
SERVER = ROOT / "build" / "battle_server_app.exe"


def free_port() -> int:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as probe:
        probe.bind(("127.0.0.1", 0))
        return int(probe.getsockname()[1])


def write_config(path: Path, port: int) -> None:
    path.write_text(
        json.dumps(
            {
                "mode": "local",
                "server": {"tickRate": 20, "snapshotRate": 10, "maxConnections": 16, "maxMatches": 8},
                "transports": {
                    "tcp": {"enabled": False, "host": "127.0.0.1", "port": 5555, "maxFrameBytes": 65536},
                    "websocket": {
                        "enabled": True,
                        "host": "127.0.0.1",
                        "port": port,
                        "path": "/ws",
                        "maxMessageBytes": 65536,
                        "requireTls": False,
                    },
                },
                "security": {
                    "demoAuthEnabled": True,
                    "telegramAuthEnabled": False,
                    "handshakeTimeoutMs": 1000,
                    "idleTimeoutMs": 1000,
                    "maxInputCommandsPerSecond": 30,
                    "maxPendingWriteBytesPerSession": 1048576,
                    "maxPendingOutboundMessages": 64,
                    "maxPendingCommandsPerMatch": 128,
                    "maxCommandsPerTick": 64,
                    "maxPendingCommandsPerSession": 64,
                },
                "metrics": {"logEverySeconds": 5},
            }
        ),
        encoding="utf-8",
    )


def connect_ws(port: int) -> socket.socket:
    deadline = time.monotonic() + 5.0
    last_error: OSError | None = None
    while time.monotonic() < deadline:
        try:
            sock = socket.create_connection(("127.0.0.1", port), timeout=1.0)
            break
        except OSError as exc:
            last_error = exc
            time.sleep(0.05)
    else:
        raise AssertionError(f"failed to connect to WebSocket listener: {last_error}")

    key = base64.b64encode(os.urandom(16)).decode("ascii")
    request = (
        f"GET /ws HTTP/1.1\r\n"
        f"Host: 127.0.0.1:{port}\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        f"Sec-WebSocket-Key: {key}\r\n"
        "Sec-WebSocket-Version: 13\r\n\r\n"
    )
    sock.sendall(request.encode("ascii"))
    response = sock.recv(4096).decode("latin1")
    assert "101 Switching Protocols" in response, response
    sock.settimeout(0.75)
    return sock


def send_json(sock: socket.socket, value: dict[str, object]) -> None:
    payload = json.dumps(value, separators=(",", ":")).encode("utf-8")
    mask = b"\x01\x02\x03\x04"
    header = bytearray([0x81])
    if len(payload) < 126:
        header.append(0x80 | len(payload))
    else:
        header.extend([0x80 | 126, (len(payload) >> 8) & 0xFF, len(payload) & 0xFF])
    masked = bytes(byte ^ mask[index % 4] for index, byte in enumerate(payload))
    sock.sendall(bytes(header) + mask + masked)


def recv_json(sock: socket.socket) -> dict[str, object]:
    header = sock.recv(2)
    assert len(header) == 2, "missing WebSocket frame header"
    length = header[1] & 0x7F
    if length == 126:
        length = struct.unpack(">H", sock.recv(2))[0]
    elif length == 127:
        length = struct.unpack(">Q", sock.recv(8))[0]
    payload = b""
    while len(payload) < length:
        payload += sock.recv(length - len(payload))
    decoded = json.loads(payload.decode("utf-8"))
    assert isinstance(decoded, dict)
    return decoded


def handle_control_message(sock: socket.socket, message: dict[str, object]) -> None:
    if message.get("type") == "ping":
        send_json(sock, {"version": 1, "type": "pong", "payload": {}})
    if message.get("type") == "error":
        payload = message.get("payload")
        if isinstance(payload, dict):
            raise AssertionError(f"server error: {payload.get('code')} {payload.get('message')}")
        raise AssertionError(f"server error: {message}")


def expect_type(sock: socket.socket, message_type: str) -> dict[str, object]:
    for _ in range(32):
        message = recv_json(sock)
        handle_control_message(sock, message)
        if message.get("type") == message_type:
            return message
    raise AssertionError(f"missing message type {message_type}")


def read_during_active_match(sockets: list[socket.socket], duration_seconds: float) -> dict[socket.socket, int]:
    deadline = time.monotonic() + duration_seconds
    snapshots = {sock: 0 for sock in sockets}
    while time.monotonic() < deadline:
        for sock in sockets:
            try:
                message = recv_json(sock)
            except TimeoutError:
                continue
            except socket.timeout:
                continue
            handle_control_message(sock, message)
            if message.get("type") == "snapshot":
                snapshots[sock] += 1
    return snapshots


def main() -> int:
    port = free_port()
    with tempfile.TemporaryDirectory(prefix="if-arena-ws-") as temp_dir:
        config = Path(temp_dir) / "server.ws.json"
        write_config(config, port)
        server = subprocess.Popen(
            [str(SERVER), "--config", str(config), "--max-clients", "2"],
            cwd=ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
        try:
            creator = connect_ws(port)
            joiner = connect_ws(port)
            send_json(creator, {"version": 1, "type": "auth_request", "payload": {"mode": "demo", "displayName": "ws-one"}})
            expect_type(creator, "auth_result")
            send_json(
                creator,
                {
                    "version": 1,
                    "type": "create_match",
                    "payload": {"mode": "objective_run", "scenario": "arena_small_objective_run"},
                },
            )
            created = expect_type(creator, "match_joined")
            match_code = str(created["payload"]["matchCode"])  # type: ignore[index]

            send_json(joiner, {"version": 1, "type": "auth_request", "payload": {"mode": "demo", "displayName": "ws-two"}})
            expect_type(joiner, "auth_result")
            send_json(joiner, {"version": 1, "type": "join_match", "payload": {"matchCode": match_code}})
            joined = expect_type(joiner, "match_joined")
            match_id = str(joined["payload"]["matchId"])  # type: ignore[index]
            expect_type(creator, "snapshot")
            snapshot = expect_type(joiner, "snapshot")
            assert isinstance(snapshot["payload"]["players"], list)  # type: ignore[index]

            send_json(
                creator,
                {
                    "version": 1,
                    "type": "input_command",
                    "sessionSeq": 1,
                    "payload": {"matchId": match_id, "command": {"kind": "move", "direction": {"x": -1, "y": 0}}},
                },
            )
            ack = expect_type(creator, "input_ack")
            assert ack["payload"]["accepted"] is True  # type: ignore[index]
            snapshots = read_during_active_match([creator, joiner], duration_seconds=2.5)
            assert snapshots[creator] > 0, "creator did not receive live snapshots beyond idle timeout"
            assert snapshots[joiner] > 0, "passive joiner did not receive live snapshots beyond idle timeout"

            send_json(
                creator,
                {
                    "version": 1,
                    "type": "input_command",
                    "sessionSeq": 2,
                    "payload": {"matchId": match_id, "command": {"kind": "stop"}},
                },
            )
            stop_ack = expect_type(creator, "input_ack")
            assert stop_ack["payload"]["accepted"] is True  # type: ignore[index]
            creator.close()
            joiner.close()
            stdout, stderr = server.communicate(timeout=10)
            assert server.returncode == 0, (server.returncode, stdout, stderr)
        finally:
            if server.poll() is None:
                server.terminate()
                server.communicate(timeout=5)

    print("[PASS] websocket_local_smoke")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
