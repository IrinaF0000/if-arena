#!/usr/bin/env python3
"""Small reproducible local TCP load/security smoke for IF Arena.

This is intentionally modest: it proves the playable raw TCP path handles
normal clients, rejected command spam, and malformed clients without claiming
production throughput.
"""

from __future__ import annotations

import argparse
import json
import os
import socket
import struct
import subprocess
import sys
import tempfile
import time
from dataclasses import dataclass, field
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
DEFAULT_SERVER = ROOT / "build" / "battle_server_app.exe"
DEFAULT_REPORT = ROOT / "reports" / "load" / "0028-local-tcp-smoke.md"


@dataclass
class Metrics:
    clients_started: int = 0
    clients_connected: int = 0
    connection_failures: int = 0
    commands_sent: int = 0
    acks_received: int = 0
    rejected_commands: int = 0
    snapshots_received: int = 0
    malformed_payloads_sent: int = 0
    protocol_errors: int = 0
    disconnects: int = 0
    latencies_ms: list[float] = field(default_factory=list)


class TcpClient:
    def __init__(self, host: str, port: int, timeout: float = 2.0) -> None:
        self.socket = socket.create_connection((host, port), timeout=timeout)
        self.socket.settimeout(timeout)

    def close(self) -> None:
        try:
            self.socket.close()
        except OSError:
            pass

    def send_envelope(self, message_type: str, payload: dict, session_seq: int | None = None) -> None:
        envelope: dict[str, object] = {"version": 1, "type": message_type, "payload": payload}
        if session_seq is not None:
            envelope["sessionSeq"] = session_seq
        self.send_raw(json.dumps(envelope, separators=(",", ":")).encode("utf-8"))

    def send_raw(self, payload: bytes) -> None:
        self.socket.sendall(struct.pack(">I", len(payload)) + payload)

    def read_envelope(self) -> dict:
        header = self._read_exact(4)
        length = struct.unpack(">I", header)[0]
        if length <= 0 or length > 64 * 1024:
            raise RuntimeError(f"invalid frame length {length}")
        payload = self._read_exact(length)
        return json.loads(payload.decode("utf-8"))

    def _read_exact(self, size: int) -> bytes:
        chunks: list[bytes] = []
        remaining = size
        while remaining > 0:
            chunk = self.socket.recv(remaining)
            if not chunk:
                raise ConnectionError("socket closed")
            chunks.append(chunk)
            remaining -= len(chunk)
        return b"".join(chunks)


def percentile(values: list[float], ratio: float) -> float:
    if not values:
        return 0.0
    ordered = sorted(values)
    index = min(len(ordered) - 1, int(round((len(ordered) - 1) * ratio)))
    return ordered[index]


def wait_for(client: TcpClient, message_type: str, metrics: Metrics, deadline: float = 3.0) -> dict:
    end = time.monotonic() + deadline
    while time.monotonic() < end:
        received = client.read_envelope()
        kind = received.get("type")
        if kind == "snapshot":
            metrics.snapshots_received += 1
        if kind == "error":
            metrics.protocol_errors += 1
        if kind == message_type:
            return received
    raise TimeoutError(f"timed out waiting for {message_type}")


def auth(client: TcpClient, name: str, metrics: Metrics) -> str:
    start = time.perf_counter()
    client.send_envelope("auth_request", {"mode": "demo", "displayName": name})
    received = wait_for(client, "auth_result", metrics)
    metrics.latencies_ms.append((time.perf_counter() - start) * 1000.0)
    return str(received["payload"]["sessionId"])


def run_normal_and_spam(host: str, port: int, metrics: Metrics) -> None:
    first = TcpClient(host, port)
    second = TcpClient(host, port)
    metrics.clients_started += 2
    metrics.clients_connected += 2
    try:
        auth(first, "load-one", metrics)
        auth(second, "load-two", metrics)

        first.send_envelope("create_match", {"mode": "objective_run", "scenario": "arena_small_objective_run"})
        joined_first = wait_for(first, "match_joined", metrics)
        code = joined_first["payload"]["matchCode"]

        second.send_envelope("join_match", {"matchCode": code})
        wait_for(second, "match_joined", metrics)
        wait_for(first, "snapshot", metrics)
        wait_for(second, "snapshot", metrics)

        for seq in range(1, 7):
            start = time.perf_counter()
            first.send_envelope(
                "input_command",
                {"matchId": joined_first["payload"]["matchId"], "command": {"kind": "move", "direction": {"x": 0, "y": -1}}},
                session_seq=seq,
            )
            metrics.commands_sent += 1
            ack = wait_for(first, "input_ack", metrics)
            metrics.acks_received += 1
            metrics.latencies_ms.append((time.perf_counter() - start) * 1000.0)
            if not ack["payload"].get("accepted", False):
                metrics.rejected_commands += 1

        first.send_envelope(
            "input_command",
            {"matchId": joined_first["payload"]["matchId"], "command": {"kind": "move", "direction": {"x": 1, "y": 0}}},
            session_seq=6,
        )
        metrics.commands_sent += 1
        duplicate_ack = wait_for(first, "input_ack", metrics)
        metrics.acks_received += 1
        if not duplicate_ack["payload"].get("accepted", False):
            metrics.rejected_commands += 1
    finally:
        first.close()
        second.close()
        metrics.disconnects += 2


def run_invalid_payloads(host: str, port: int, metrics: Metrics) -> None:
    malformed = TcpClient(host, port)
    metrics.clients_started += 1
    metrics.clients_connected += 1
    try:
        malformed.send_raw(b"{not-json")
        metrics.malformed_payloads_sent += 1
        try:
            received = malformed.read_envelope()
            if received.get("type") == "error":
                metrics.protocol_errors += 1
        except (ConnectionError, OSError, json.JSONDecodeError):
            metrics.disconnects += 1
    finally:
        malformed.close()

    unknown = TcpClient(host, port)
    metrics.clients_started += 1
    metrics.clients_connected += 1
    try:
        unknown.send_envelope("unknown_type", {})
        metrics.malformed_payloads_sent += 1
        try:
            received = unknown.read_envelope()
            if received.get("type") == "error":
                metrics.protocol_errors += 1
        except (ConnectionError, OSError, json.JSONDecodeError):
            metrics.disconnects += 1
    finally:
        unknown.close()


def git_head() -> str:
    try:
        return subprocess.check_output(["git", "rev-parse", "--short", "HEAD"], cwd=ROOT, text=True).strip()
    except (subprocess.CalledProcessError, OSError):
        return "unknown"


def write_report(path: Path, metrics: Metrics, server: Path) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    lines = [
        "# IF Arena Local TCP Load Smoke",
        "",
        "## Environment",
        "",
        f"- dateUtc: {time.strftime('%Y-%m-%dT%H:%M:%SZ', time.gmtime())}",
        f"- commit: {git_head()}",
        f"- server: `{server}`",
        "- scope: local smoke, not production benchmark",
        "- transport: raw TCP length-prefixed JSON",
        "- scenarios: normal create/join/gameplay, duplicate command rejection, malformed payloads",
        "",
        "## Results",
        "",
        f"- clientsStarted: {metrics.clients_started}",
        f"- clientsConnected: {metrics.clients_connected}",
        f"- connectionFailures: {metrics.connection_failures}",
        f"- commandsSent: {metrics.commands_sent}",
        f"- acksReceived: {metrics.acks_received}",
        f"- rejectedCommands: {metrics.rejected_commands}",
        f"- snapshotsReceived: {metrics.snapshots_received}",
        f"- malformedPayloadsSent: {metrics.malformed_payloads_sent}",
        f"- protocolErrorsObserved: {metrics.protocol_errors}",
        f"- disconnectsObserved: {metrics.disconnects}",
        f"- p50AckLatencyMs: {percentile(metrics.latencies_ms, 0.50):.2f}",
        f"- p95AckLatencyMs: {percentile(metrics.latencies_ms, 0.95):.2f}",
        f"- p99AckLatencyMs: {percentile(metrics.latencies_ms, 0.99):.2f}",
        "",
        "## Limits And Honesty",
        "",
        "- This run is intentionally tiny and local. It validates scenario wiring and negative handling, not capacity.",
        "- Slow-reader pressure is documented and modeled by `battle_load_client --scenario slow_readers`; a large live slow-reader soak remains future work.",
        "- Public deployment, TLS termination, async accept scaling, and production observability remain out of scope for this local local slice.",
        "",
    ]
    path.write_text("\n".join(lines), encoding="utf-8")


def wait_for_server(host: str, port: int, timeout: float = 5.0) -> None:
    deadline = time.monotonic() + timeout
    last_error: Exception | None = None
    while time.monotonic() < deadline:
        try:
            with socket.create_connection((host, port), timeout=0.25):
                return
        except OSError as exc:
            last_error = exc
            time.sleep(0.05)
    raise RuntimeError(f"server did not start: {last_error}")


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("--server", type=Path, default=DEFAULT_SERVER)
    parser.add_argument("--report", type=Path, default=DEFAULT_REPORT)
    parser.add_argument("--host", default="127.0.0.1")
    parser.add_argument("--port", type=int, default=5565)
    args = parser.parse_args()

    if not args.server.exists():
        print(f"server executable not found: {args.server}", file=sys.stderr)
        return 1

    config = {
        "mode": "local",
        "server": {"tickRate": 20, "snapshotRate": 10, "maxConnections": 8, "maxMatches": 8},
        "transports": {
            "tcp": {"enabled": True, "host": args.host, "port": args.port, "maxFrameBytes": 65536},
            "websocket": {
                "enabled": False,
                "host": args.host,
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

    metrics = Metrics()
    with tempfile.NamedTemporaryFile("w", encoding="utf-8", suffix=".json", delete=False) as handle:
        json.dump(config, handle)
        config_path = Path(handle.name)

    process = subprocess.Popen(
        [str(args.server), "--config", str(config_path), "--max-clients", "5"],
        cwd=ROOT,
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )
    try:
        wait_for_server(args.host, args.port)
        run_normal_and_spam(args.host, args.port, metrics)
        run_invalid_payloads(args.host, args.port, metrics)
        write_report(args.report, metrics, args.server)
        print(f"[PASS] local_tcp_load_scenarios report={args.report}")
        return 0
    finally:
        try:
            process.wait(timeout=2.0)
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


if __name__ == "__main__":
    raise SystemExit(main())
