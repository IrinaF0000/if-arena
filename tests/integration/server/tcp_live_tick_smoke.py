#!/usr/bin/env python3
"""Regression smoke for TCP match ticking while clients are idle."""

from __future__ import annotations

import json
import socket
import struct
import subprocess
import sys
import tempfile
import threading
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
            "tickRate": 60,
            "snapshotRate": 30,
            "maxConnections": 8,
            "maxMatches": 4,
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
            "handshakeTimeoutMs": 2000,
            "idleTimeoutMs": 70000,
            "maxInputCommandsPerSecond": 60,
            "maxPendingWriteBytesPerSession": 1048576,
            "maxPendingOutboundMessages": 256,
            "maxPendingCommandsPerMatch": 256,
            "maxCommandsPerTick": 64,
            "maxPendingCommandsPerSession": 64,
        },
        "metrics": {"logEverySeconds": 30},
    }
    path.write_text(json.dumps(config), encoding="utf-8")


def connect_retry(port: int, timeout_seconds: float = 5.0) -> socket.socket:
    deadline = time.monotonic() + timeout_seconds
    last_error: OSError | None = None
    while time.monotonic() < deadline:
        try:
            sock = socket.create_connection(("127.0.0.1", port), timeout=1.0)
            sock.settimeout(30.0)
            return sock
        except OSError as exc:
            last_error = exc
            time.sleep(0.05)
    raise AssertionError(f"failed to connect to local server: {last_error}")


def send_frame(sock: socket.socket, payload: dict[str, object]) -> None:
    encoded = json.dumps(payload, separators=(",", ":")).encode("utf-8")
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


def read_until_type(sock: socket.socket, expected_type: str, timeout_seconds: float = 4.0) -> dict[str, object]:
    deadline = time.monotonic() + timeout_seconds
    while time.monotonic() < deadline:
        frame = recv_frame(sock)
        if frame.get("type") == expected_type:
            return frame
    raise AssertionError(f"did not receive {expected_type}")


def auth(sock: socket.socket, name: str) -> str:
    send_frame(sock, {"version": 1, "type": "auth_request", "payload": {"mode": "demo", "displayName": name}})
    frame = expect_type(sock, "auth_result")
    return str(frame["payload"]["sessionId"])


def read_snapshot_at_or_after(sock: socket.socket, minimum_tick: int) -> dict[str, object]:
    deadline = time.monotonic() + 30.0
    last_tick = -1
    while time.monotonic() < deadline:
        frame = recv_frame(sock)
        if frame.get("type") != "snapshot":
            continue
        payload = frame["payload"]
        last_tick = int(payload["tick"])
        if last_tick >= minimum_tick:
            return payload
    raise AssertionError(f"snapshot did not reach tick {minimum_tick}; last tick={last_tick}")


def read_snapshots_until(sock: socket.socket, minimum_tick: int, timeout_seconds: float = 5.0) -> list[dict[str, object]]:
    deadline = time.monotonic() + timeout_seconds
    snapshots: list[dict[str, object]] = []
    last_tick = -1
    while time.monotonic() < deadline:
        frame = recv_frame(sock)
        if frame.get("type") != "snapshot":
            continue
        payload = frame["payload"]
        last_tick = int(payload["tick"])
        snapshots.append(payload)
        if last_tick >= minimum_tick:
            return snapshots
    raise AssertionError(f"snapshot stream did not reach tick {minimum_tick}; last tick={last_tick}")


def player_position(snapshot: dict[str, object], player_id: str) -> tuple[float, float]:
    for player in snapshot["players"]:
        if str(player["playerId"]) == player_id:
            return float(player["x"]), float(player["y"])
    raise AssertionError(f"player {player_id} missing from snapshot")


def drain_frames(sock: socket.socket, stop: threading.Event) -> None:
    sock.settimeout(0.5)
    while not stop.is_set():
        try:
            recv_frame(sock)
        except TimeoutError:
            continue
        except OSError:
            break
        except AssertionError:
            break


def live_match_ticks_after_idle_and_accepts_late_input(port: int) -> None:
    create = connect_retry(port)
    join = connect_retry(port)
    drain_stop = threading.Event()
    drain_thread: threading.Thread | None = None
    try:
        auth(create, "live-one")
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

        join_session = auth(join, "live-two")
        send_frame(join, {"version": 1, "type": "join_match", "payload": {"matchCode": code}})
        joined = expect_type(join, "match_joined")
        match_id = str(joined["payload"]["matchId"])

        drain_thread = threading.Thread(target=drain_frames, args=(create, drain_stop), daemon=True)
        drain_thread.start()
        early_snapshots = read_snapshots_until(join, 20)
        early_ticks = [int(snapshot["tick"]) for snapshot in early_snapshots]
        require(len(early_ticks) >= 2, f"expected multiple early snapshots, got {early_ticks}")
        require(
            any(next_tick - tick > 1 for tick, next_tick in zip(early_ticks, early_ticks[1:])),
            f"snapshotRate should not broadcast every simulation tick: {early_ticks}",
        )
        idle_snapshot = read_snapshot_at_or_after(join, 1201)
        before = player_position(idle_snapshot, join_session)
        send_frame(
            join,
            {
                "version": 1,
                "type": "input_command",
                "sessionSeq": 1,
                "payload": {"matchId": match_id, "command": {"kind": "move", "direction": {"x": 1, "y": 0}}},
            },
        )
        ack = read_until_type(join, "input_ack")
        require(ack["payload"]["accepted"] is True, f"late input command rejected: {ack}")
        deadline = time.monotonic() + 4.0
        after = before
        while time.monotonic() < deadline and after == before:
            frame = recv_frame(join)
            if frame.get("type") == "snapshot":
                after = player_position(frame["payload"], join_session)
        require(after != before, f"late input after tick 1200 did not change authoritative position: {before}")
    finally:
        drain_stop.set()
        create.close()
        join.close()
        if drain_thread is not None:
            drain_thread.join(timeout=2.0)


def main() -> int:
    if not SERVER_EXE.exists():
        print(f"missing server executable: {SERVER_EXE}", file=sys.stderr)
        return 2

    port = free_port()
    with tempfile.TemporaryDirectory(prefix="if-arena-live-tick-") as temp:
        config = Path(temp) / "server.local.json"
        write_config(config, port)
        server = subprocess.Popen(
            [str(SERVER_EXE), "--config", str(config), "--max-clients", "2"],
            cwd=ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
        try:
            live_match_ticks_after_idle_and_accepts_late_input(port)
            stdout, stderr = server.communicate(timeout=10)
            require(server.returncode == 0, f"server exited {server.returncode}\n{stdout}\n{stderr}")
        finally:
            if server.poll() is None:
                server.terminate()
                server.communicate(timeout=5)

    print("[PASS] tcp_live_tick_smoke")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
