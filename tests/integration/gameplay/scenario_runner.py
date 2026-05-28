#!/usr/bin/env python3
"""Config-driven local gameplay scenario runner for TCP and WebSocket paths."""

from __future__ import annotations

import base64
import json
import os
import socket
import struct
import subprocess
import sys
import tempfile
import time
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Protocol


ROOT = Path(__file__).resolve().parents[3]
SERVER_EXE = ROOT / "build" / ("battle_server_app.exe" if sys.platform == "win32" else "battle_server_app")


def require(condition: bool, message: str) -> None:
    if not condition:
        raise AssertionError(message)


def free_port() -> int:
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as probe:
        probe.bind(("127.0.0.1", 0))
        return int(probe.getsockname()[1])


def write_server_config(path: Path, scenario: dict[str, Any], transport: str, port: int) -> None:
    server = scenario.get("server", {})
    max_frame_bytes = 65536
    path.write_text(
        json.dumps(
            {
                "mode": "local",
                "server": {
                    "tickRate": int(server.get("tickRate", 20)),
                    "snapshotRate": int(server.get("snapshotRate", 10)),
                    "maxConnections": 8,
                    "maxMatches": 4,
                },
                "game": {"scenarioPath": str(scenario["gameScenario"])},
                "transports": {
                    "tcp": {
                        "enabled": transport == "desktop",
                        "host": "127.0.0.1",
                        "port": port,
                        "maxFrameBytes": max_frame_bytes,
                    },
                    "websocket": {
                        "enabled": transport == "mobile",
                        "host": "127.0.0.1",
                        "port": port,
                        "path": "/ws",
                        "maxMessageBytes": max_frame_bytes,
                        "requireTls": False,
                    },
                },
                "security": {
                    "demoAuthEnabled": True,
                    "telegramAuthEnabled": False,
                    "handshakeTimeoutMs": 2000,
                    "idleTimeoutMs": 70000,
                    "maxInputCommandsPerSecond": int(server.get("maxInputCommandsPerSecond", 60)),
                    "maxPendingWriteBytesPerSession": 1048576,
                    "maxPendingOutboundMessages": 512,
                    "maxPendingCommandsPerMatch": 512,
                    "maxCommandsPerTick": 128,
                    "maxPendingCommandsPerSession": int(server.get("maxPendingCommandsPerSession", 128)),
                },
                "metrics": {"logEverySeconds": 30},
            }
        ),
        encoding="utf-8",
    )


class ScenarioClient(Protocol):
    session_id: str
    session_seq: int

    def send_json(self, value: dict[str, Any]) -> None: ...
    def recv_json(self) -> dict[str, Any]: ...
    def close(self) -> None: ...


@dataclass
class TcpClient:
    sock: socket.socket
    session_id: str = ""
    session_seq: int = 1

    def send_json(self, value: dict[str, Any]) -> None:
        encoded = json.dumps(value, separators=(",", ":")).encode("utf-8")
        self.sock.sendall(struct.pack(">I", len(encoded)) + encoded)

    def recv_json(self) -> dict[str, Any]:
        header = self.sock.recv(4)
        require(len(header) == 4, "connection closed before TCP frame header")
        (size,) = struct.unpack(">I", header)
        payload = b""
        while len(payload) < size:
            chunk = self.sock.recv(size - len(payload))
            require(bool(chunk), "connection closed before TCP frame payload")
            payload += chunk
        decoded = json.loads(payload.decode("utf-8"))
        require(isinstance(decoded, dict), "TCP frame JSON is an object")
        return decoded

    def close(self) -> None:
        self.sock.close()


@dataclass
class WebSocketClient:
    sock: socket.socket
    session_id: str = ""
    session_seq: int = 1

    def send_json(self, value: dict[str, Any]) -> None:
        payload = json.dumps(value, separators=(",", ":")).encode("utf-8")
        mask = b"\x01\x02\x03\x04"
        header = bytearray([0x81])
        if len(payload) < 126:
            header.append(0x80 | len(payload))
        else:
            header.extend([0x80 | 126, (len(payload) >> 8) & 0xFF, len(payload) & 0xFF])
        masked = bytes(byte ^ mask[index % 4] for index, byte in enumerate(payload))
        self.sock.sendall(bytes(header) + mask + masked)

    def recv_json(self) -> dict[str, Any]:
        header = self.sock.recv(2)
        require(len(header) == 2, "missing WebSocket frame header")
        length = header[1] & 0x7F
        if length == 126:
            length = struct.unpack(">H", self.sock.recv(2))[0]
        elif length == 127:
            length = struct.unpack(">Q", self.sock.recv(8))[0]
        payload = b""
        while len(payload) < length:
            payload += self.sock.recv(length - len(payload))
        decoded = json.loads(payload.decode("utf-8"))
        require(isinstance(decoded, dict), "WebSocket frame JSON is an object")
        return decoded

    def close(self) -> None:
        self.sock.close()


def connect_tcp(port: int) -> TcpClient:
    deadline = time.monotonic() + 5.0
    last_error: OSError | None = None
    while time.monotonic() < deadline:
        try:
            sock = socket.create_connection(("127.0.0.1", port), timeout=1.0)
            sock.settimeout(10.0)
            return TcpClient(sock)
        except OSError as exc:
            last_error = exc
            time.sleep(0.05)
    raise AssertionError(f"failed to connect TCP client: {last_error}")


def connect_websocket(port: int) -> WebSocketClient:
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
        raise AssertionError(f"failed to connect WebSocket client: {last_error}")
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
    require("101 Switching Protocols" in response, response)
    sock.settimeout(10.0)
    return WebSocketClient(sock)


def connect_client(transport: str, port: int) -> ScenarioClient:
    return connect_tcp(port) if transport == "desktop" else connect_websocket(port)


def handle_control(client: ScenarioClient, message: dict[str, Any]) -> None:
    if message.get("type") == "ping":
        client.send_json({"version": 1, "type": "pong", "payload": {}})
    if message.get("type") == "error":
        raise AssertionError(f"server error: {message}")


def expect_type(client: ScenarioClient, expected_type: str, attempts: int = 64) -> dict[str, Any]:
    for _ in range(attempts):
        message = client.recv_json()
        handle_control(client, message)
        if message.get("type") == expected_type:
            return message
    raise AssertionError(f"missing message type {expected_type}")


def auth(client: ScenarioClient, name: str) -> None:
    client.send_json({"version": 1, "type": "auth_request", "payload": {"mode": "demo", "displayName": name}})
    result = expect_type(client, "auth_result")
    client.session_id = str(result["payload"]["sessionId"])


def make_command(step: dict[str, Any]) -> dict[str, Any]:
    command = {"kind": step["command"]}
    if "direction" in step:
        command["direction"] = step["direction"]
    return command


def send_command(client: ScenarioClient, match_id: str, step: dict[str, Any]) -> None:
    client.send_json(
        {
            "version": 1,
            "type": "input_command",
            "sessionSeq": client.session_seq,
            "payload": {"matchId": match_id, "command": make_command(step)},
        }
    )
    client.session_seq += 1
    ack = expect_type(client, "input_ack")
    require(ack["payload"]["accepted"] is True, f"input rejected: {ack}")


def latest_snapshots_until(client: ScenarioClient, target_tick: int) -> list[dict[str, Any]]:
    snapshots: list[dict[str, Any]] = []
    last_tick = -1
    deadline = time.monotonic() + 10.0
    while time.monotonic() < deadline:
        message = client.recv_json()
        handle_control(client, message)
        if message.get("type") != "snapshot":
            continue
        snapshot = message["payload"]
        snapshots.append(snapshot)
        last_tick = int(snapshot["tick"])
        if last_tick >= target_tick:
            return snapshots
    raise AssertionError(f"snapshot stream did not reach tick {target_tick}; last tick={last_tick}")


def player(snapshot: dict[str, Any], player_id: str) -> dict[str, Any]:
    for candidate in snapshot["players"]:
        if str(candidate["playerId"]) == player_id:
            return candidate
    raise AssertionError(f"player {player_id} missing from snapshot")


def score(snapshot: dict[str, Any], team: str) -> int:
    for candidate in snapshot["scores"]:
        if candidate["team"] == team:
            return int(candidate["score"])
    raise AssertionError(f"score for {team} missing from snapshot")


def assert_smooth(snapshots: list[dict[str, Any]], player_id: str) -> None:
    positions = [(float(player(snapshot, player_id)["x"]), float(player(snapshot, player_id)["y"])) for snapshot in snapshots]
    moved = any(before != after for before, after in zip(positions, positions[1:]))
    require(moved, "scenario player did not move across snapshots")
    for before, after in zip(positions, positions[1:]):
        dx = abs(after[0] - before[0])
        dy = abs(after[1] - before[1])
        require(dx <= 1.0 and dy <= 1.0, f"unexpected movement jump {before} -> {after}")


def assert_step(step: dict[str, Any], snapshot: dict[str, Any], actors: dict[str, ScenarioClient]) -> None:
    if step["assert"] == "objective_picked":
        carrier = actors[step["by"]].session_id
        objective = snapshot["objective"]
        require(objective["state"] == "carried", f"objective is not carried: {objective}")
        require(str(objective["carrierPlayerId"]) == carrier, f"unexpected carrier: {objective}")
        return
    if step["assert"] == "capture":
        require(score(snapshot, step["by"]) >= 1, f"capture score missing: {snapshot['scores']}")
        return
    if step["assert"] == "score":
        require(score(snapshot, step["team"]) == int(step["equals"]), f"unexpected score: {snapshot['scores']}")
        return
    raise AssertionError(f"unsupported assertion {step['assert']}")


def run_loaded_scenario(scenario: dict[str, Any], transport: str) -> None:
    require(transport in {"desktop", "mobile"}, "transport must be desktop or mobile")
    require(SERVER_EXE.exists(), f"missing server executable: {SERVER_EXE}")
    port = free_port()
    with tempfile.TemporaryDirectory(prefix=f"if-arena-{transport}-scenario-") as temp:
        config = Path(temp) / "server.local.json"
        write_server_config(config, scenario, transport, port)
        server = subprocess.Popen(
            [str(SERVER_EXE), "--config", str(config), "--max-clients", "2"],
            cwd=ROOT,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
        )
        clients: list[ScenarioClient] = []
        try:
            blue = connect_client(transport, port)
            red = connect_client(transport, port)
            clients.extend([blue, red])
            auth(blue, f"{transport}-blue")
            blue.send_json(
                {
                    "version": 1,
                    "type": "create_match",
                    "payload": {"mode": "objective_run", "scenario": "arena_small_objective_run"},
                }
            )
            created = expect_type(blue, "match_joined")
            match_code = str(created["payload"]["matchCode"])
            auth(red, f"{transport}-red")
            red.send_json({"version": 1, "type": "join_match", "payload": {"matchCode": match_code}})
            joined = expect_type(red, "match_joined")
            match_id = str(joined["payload"]["matchId"])
            actors = {"blue": blue, "red": red}
            snapshot = expect_type(blue, "snapshot")["payload"]
            expect_type(red, "snapshot")
            snapshots_for_assertions: list[dict[str, Any]] = [snapshot]
            for step in scenario["steps"]:
                if "command" in step:
                    actor = actors[step["actor"]]
                    send_command(actor, match_id, step)
                    target_tick = int(snapshot["tick"]) + int(step.get("ticks", 1))
                    snapshots = latest_snapshots_until(blue, target_tick)
                    snapshots_for_assertions.extend(snapshots)
                    snapshot = snapshots[-1]
                    if step["command"] == "move" and scenario.get("movementAssertions", {}).get("smooth", False):
                        assert_smooth(snapshots, blue.session_id)
                    continue
                if "assert" in step:
                    assert_step(step, snapshot, actors)
            for client in clients:
                client.close()
            stdout, stderr = server.communicate(timeout=10)
            require(server.returncode == 0, f"server exited {server.returncode}\n{stdout}\n{stderr}")
        finally:
            for client in clients:
                client.close()
            if server.poll() is None:
                server.terminate()
                server.communicate(timeout=5)


def run_scenario_file(path: Path, transport: str) -> None:
    scenario = json.loads(path.read_text(encoding="utf-8"))
    run_loaded_scenario(scenario, transport)


def main(argv: list[str]) -> int:
    require(len(argv) == 3, "usage: scenario_runner.py <scenario.json> <desktop|mobile>")
    run_scenario_file(ROOT / argv[1], argv[2])
    print(f"[PASS] {Path(argv[1]).stem}_{argv[2]}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv))
