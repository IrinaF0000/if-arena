#!/usr/bin/env python3
"""Local HTTP helper for running battle_sim from the replay viewer."""

from __future__ import annotations

import argparse
import json
import os
import random
import re
import subprocess
import threading
from datetime import datetime
from http.server import BaseHTTPRequestHandler, ThreadingHTTPServer
from pathlib import Path
from typing import Any
from urllib.parse import unquote, urlparse


HOST = "127.0.0.1"
DEFAULT_PORT = 8765
MAX_BODY_BYTES = 64 * 1024
MAX_MAP_WIDTH = 64
MAX_MAP_HEIGHT = 64
MAX_TOTAL_UNITS = 128
RUN_ID_RE = re.compile(r"[A-Za-z0-9][A-Za-z0-9_.-]*")
RUN_ID_MAX_LENGTH = 80
RUN_DIR_LOCK = threading.Lock()

UNIT_SPECS = {
    "Swordsman": "hp=12 strength=3",
    "Hunter": "hp=9 agility=4 strength=1 range=3",
    "Tower": "hp=10 power=2",
    "Healer": "hp=8 spirit=2",
    "Mine": "power=2",
    "Raven": "hp=6 agility=2",
}
MOBILE_UNITS = {"Swordsman", "Hunter", "Healer", "Raven"}


def is_relative_to(path: Path, root: Path) -> bool:
    try:
        path.relative_to(root)
        return True
    except ValueError:
        return False


def json_bytes(payload: dict[str, Any]) -> bytes:
    return json.dumps(payload, indent=2, sort_keys=True).encode("utf-8")


def process_text(value: str | bytes | None) -> str:
    if value is None:
        return ""
    if isinstance(value, bytes):
        return value.decode("utf-8", errors="replace")
    return value


def read_json_object_file(path: Path) -> tuple[dict[str, Any], str | None]:
    try:
        payload = json.loads(path.read_text(encoding="utf-8")) if path.is_file() else {}
    except (OSError, UnicodeDecodeError, json.JSONDecodeError) as exc:
        return {}, f"Could not read {path.name}: {exc}"
    if not isinstance(payload, dict):
        return {}, f"{path.name} does not contain a JSON object."
    return payload, None


def parse_int(value: Any, name: str, minimum: int, maximum: int) -> int:
    if isinstance(value, bool):
        raise ValueError(f"{name} must be a number.")
    try:
        parsed = int(value)
    except (TypeError, ValueError) as exc:
        raise ValueError(f"{name} must be a number.") from exc
    if parsed < minimum or parsed > maximum:
        raise ValueError(f"{name} must be between {minimum} and {maximum}.")
    return parsed


def sanitize_slug(value: Any) -> str:
    if not isinstance(value, str):
        return ""
    slug = re.sub(r"[^A-Za-z0-9_.-]+", "-", value.strip()).strip(".-")
    return slug[:40]


def normalize_path(value: str, base: Path) -> Path:
    path = Path(value).expanduser()
    if not path.is_absolute():
        path = base / path
    return path.resolve()


class RunnerState:
    def __init__(self, exe: str | None, runs_dir: str | None, timeout_seconds: int) -> None:
        self.repo_root = Path(__file__).resolve().parents[2]
        self.viewer_root = self.repo_root / "tools" / "replay-viewer"
        self.runner_root = self.repo_root / "tools" / "local-runner"
        self.configured_exe = normalize_path(exe, self.repo_root) if exe else None
        self.runs_root = normalize_path(runs_dir, self.repo_root) if runs_dir else (self.runner_root / "runs").resolve()
        self.timeout_seconds = timeout_seconds

    def find_executable(self) -> Path | None:
        candidates: list[Path] = []
        if self.configured_exe:
            candidates.append(self.configured_exe)
        env_exe = os.environ.get("BATTLE_SIM_EXE")
        if env_exe:
            candidates.append(normalize_path(env_exe, self.repo_root))

        candidates.extend(
            [
                self.repo_root / "build" / "battle_sim",
                self.repo_root / "build" / "battle_sim.exe",
                self.repo_root / "build" / "Debug" / "battle_sim.exe",
                self.repo_root / "build" / "Release" / "battle_sim.exe",
            ]
        )
        candidates.extend(self.repo_root.glob("out/build/*/battle_sim"))
        candidates.extend(self.repo_root.glob("out/build/*/battle_sim.exe"))
        candidates.extend(self.repo_root.glob("build-*/*/battle_sim.exe"))
        candidates.extend(self.repo_root.glob("build-*/battle_sim.exe"))

        seen: set[Path] = set()
        for candidate in candidates:
            resolved = candidate.resolve()
            if resolved in seen:
                continue
            seen.add(resolved)
            if resolved.is_file():
                return resolved
        return None

    def executable_payload(self) -> dict[str, Any]:
        executable = self.find_executable()
        message = "battle_sim executable found." if executable else (
            "battle_sim executable not found. Pass --exe or set BATTLE_SIM_EXE."
        )
        return {
            "found": executable is not None,
            "path": str(executable) if executable else None,
            "configuredPath": str(self.configured_exe) if self.configured_exe else None,
            "message": message,
        }


def normalize_run_options(payload: dict[str, Any]) -> dict[str, Any]:
    width = parse_int(payload.get("mapWidth", 8), "mapWidth", 1, MAX_MAP_WIDTH)
    height = parse_int(payload.get("mapHeight", 5), "mapHeight", 1, MAX_MAP_HEIGHT)
    seed = parse_int(payload.get("seed", 1), "seed", 0, 2_147_483_647)
    run_name = sanitize_slug(payload.get("runName", ""))

    counts_source = payload.get("unitCounts")
    if not isinstance(counts_source, dict):
        counts_source = payload

    unit_counts: dict[str, int] = {}
    for unit_type in UNIT_SPECS:
        lower = unit_type[:1].lower() + unit_type[1:]
        value = counts_source.get(unit_type)
        if value is None:
            value = counts_source.get(lower)
        if value is None:
            value = counts_source.get(f"{lower}Count", 0)
        unit_counts[unit_type] = parse_int(value, f"{unit_type} count", 0, MAX_TOTAL_UNITS)

    total_units = sum(unit_counts.values())
    if total_units == 0:
        raise ValueError("At least one unit must be requested.")
    if total_units > MAX_TOTAL_UNITS:
        raise ValueError(f"Total unit count must be {MAX_TOTAL_UNITS} or less.")
    if total_units > width * height:
        raise ValueError("Too many units for the selected map size.")

    return {
        "mapWidth": width,
        "mapHeight": height,
        "seed": seed,
        "runName": run_name,
        "unitCounts": unit_counts,
    }


def choose_target(width: int, height: int, x: int, y: int, rng: random.Random) -> tuple[int, int] | None:
    preferred = [
        (width - 1 - x, y),
        (x, height - 1 - y),
        (width - 1 - x, height - 1 - y),
    ]
    for target in preferred:
        if target != (x, y):
            return target

    cells = [(cx, cy) for cy in range(height) for cx in range(width) if (cx, cy) != (x, y)]
    return rng.choice(cells) if cells else None


def generate_scenario(options: dict[str, Any]) -> str:
    width = int(options["mapWidth"])
    height = int(options["mapHeight"])
    rng = random.Random(int(options["seed"]))

    cells = [(x, y) for y in range(height) for x in range(width)]
    rng.shuffle(cells)

    lines = [f"CREATE_MAP {width} {height}", ""]
    mobile_units: list[tuple[int, int, int]] = []
    unit_id = 1
    for unit_type, count in options["unitCounts"].items():
        for _ in range(count):
            x, y = cells.pop()
            lines.append(f"SPAWN {unit_type} {unit_id} {x} {y} {UNIT_SPECS[unit_type]}")
            if unit_type in MOBILE_UNITS:
                mobile_units.append((unit_id, x, y))
            unit_id += 1

    if mobile_units:
        lines.append("")
    for entity_id, x, y in mobile_units:
        target = choose_target(width, height, x, y, rng)
        if target:
            lines.append(f"MARCH {entity_id} {target[0]} {target[1]}")

    return "\n".join(lines) + "\n"


def make_run_id(run_name: str) -> str:
    timestamp = datetime.now().strftime("%Y%m%d-%H%M%S")
    return f"{timestamp}-{run_name}" if run_name else timestamp


def ensure_unique_run_dir(runs_root: Path, run_id: str) -> tuple[str, Path]:
    runs_root = runs_root.resolve()
    if len(run_id) > RUN_ID_MAX_LENGTH:
        run_id = run_id[:RUN_ID_MAX_LENGTH].rstrip(".-_") or "run"

    with RUN_DIR_LOCK:
        runs_root.mkdir(parents=True, exist_ok=True)
        candidate_id = run_id
        suffix = 2
        while True:
            run_dir = (runs_root / candidate_id).resolve()
            if not is_relative_to(run_dir, runs_root):
                raise ValueError("Invalid run id.")
            try:
                run_dir.mkdir()
                return candidate_id, run_dir
            except FileExistsError:
                candidate_id = f"{run_id}-{suffix}"
                suffix += 1


def run_file_response(run_dir: Path, name: str, content_type: str) -> tuple[int, bytes, str]:
    path = run_dir / name
    if not path.is_file():
        return 404, json_bytes({"ok": False, "error": f"{name} is not available for this run."}), "application/json"
    return 200, path.read_bytes(), content_type


def list_run_ids(state: RunnerState) -> list[str]:
    state.runs_root.mkdir(parents=True, exist_ok=True)
    return sorted(
        [entry.name for entry in state.runs_root.iterdir() if entry.is_dir() and (entry / "run-options.json").is_file()],
        reverse=True,
    )


def run_metadata(state: RunnerState, run_id: str) -> dict[str, Any]:
    run_dir = safe_run_dir(state, run_id)
    options_path = run_dir / "run-options.json"
    exit_code_path = run_dir / "exit-code.txt"
    options, metadata_error = read_json_object_file(options_path)
    try:
        exit_code = exit_code_path.read_text(encoding="utf-8").strip() if exit_code_path.is_file() else None
    except (OSError, UnicodeDecodeError) as exc:
        exit_code = None
        metadata_error = metadata_error or f"Could not read exit-code.txt: {exc}"
    trace_exists = (run_dir / "trace.json").is_file()
    result = {
        "runId": run_id,
        "createdAt": options.get("generatedAt"),
        "runName": options.get("runName", ""),
        "options": options,
        "exitCode": exit_code,
        "traceExists": trace_exists,
        "traceUrl": f"/api/runs/{run_id}/trace" if trace_exists else None,
        "stdoutUrl": f"/api/runs/{run_id}/stdout",
        "stderrUrl": f"/api/runs/{run_id}/stderr",
        "scenarioUrl": f"/api/runs/{run_id}/scenario",
    }
    if metadata_error:
        result["metadataError"] = metadata_error
    return result


def safe_run_dir(state: RunnerState, run_id: str) -> Path:
    if len(run_id) > RUN_ID_MAX_LENGTH or not RUN_ID_RE.fullmatch(run_id):
        raise ValueError("Invalid run id.")
    runs_root = state.runs_root.resolve()
    run_dir = (runs_root / run_id).resolve()
    if not is_relative_to(run_dir, runs_root):
        raise ValueError("Invalid run path.")
    if not run_dir.is_dir():
        raise FileNotFoundError(run_id)
    return run_dir


def execute_run(state: RunnerState, payload: dict[str, Any]) -> tuple[int, dict[str, Any]]:
    options = normalize_run_options(payload)
    options["generatedAt"] = datetime.now().isoformat(timespec="seconds")
    options["cliContract"] = "battle_sim <commands-file> [config-file] [--trace-json <trace-file>]"

    run_id, run_dir = ensure_unique_run_dir(state.runs_root, make_run_id(str(options["runName"])))
    scenario_path = run_dir / "scenario.commands"
    trace_path = run_dir / "trace.json"
    stdout_path = run_dir / "stdout.txt"
    stderr_path = run_dir / "stderr.txt"
    exit_code_path = run_dir / "exit-code.txt"

    scenario_path.write_text(generate_scenario(options), encoding="utf-8")
    (run_dir / "run-options.json").write_bytes(json_bytes(options))

    executable = state.find_executable()
    if not executable:
        stdout_path.write_text("", encoding="utf-8")
        stderr_path.write_text("battle_sim executable not found. Pass --exe or set BATTLE_SIM_EXE.\n", encoding="utf-8")
        exit_code_path.write_text("not-run\n", encoding="utf-8")
        return 200, {
            "ok": False,
            "runId": run_id,
            "error": "battle_sim executable not found. Pass --exe or set BATTLE_SIM_EXE.",
            "stdoutUrl": f"/api/runs/{run_id}/stdout",
            "stderrUrl": f"/api/runs/{run_id}/stderr",
            "scenarioUrl": f"/api/runs/{run_id}/scenario",
        }

    command = [str(executable), str(scenario_path), "--trace-json", str(trace_path)]
    try:
        completed = subprocess.run(
            command,
            cwd=str(state.repo_root),
            capture_output=True,
            text=True,
            timeout=state.timeout_seconds,
            shell=False,
            encoding="utf-8",
            errors="replace",
        )
        stdout_path.write_text(completed.stdout, encoding="utf-8")
        stderr_path.write_text(completed.stderr, encoding="utf-8")
        exit_code_path.write_text(f"{completed.returncode}\n", encoding="utf-8")
    except OSError as exc:
        stdout_path.write_text("", encoding="utf-8")
        stderr_path.write_text(f"Could not launch battle_sim: {exc}\n", encoding="utf-8")
        exit_code_path.write_text("launch-error\n", encoding="utf-8")
        return 200, {
            "ok": False,
            "runId": run_id,
            "error": "Could not launch battle_sim. Check the executable path and permissions.",
            "stdoutUrl": f"/api/runs/{run_id}/stdout",
            "stderrUrl": f"/api/runs/{run_id}/stderr",
            "scenarioUrl": f"/api/runs/{run_id}/scenario",
        }
    except subprocess.TimeoutExpired as exc:
        stdout_path.write_text(process_text(exc.stdout), encoding="utf-8")
        stderr_path.write_text(
            process_text(exc.stderr) + f"\nTimed out after {state.timeout_seconds} seconds.\n",
            encoding="utf-8",
        )
        exit_code_path.write_text("timeout\n", encoding="utf-8")
        return 200, {
            "ok": False,
            "runId": run_id,
            "error": f"Simulation timed out after {state.timeout_seconds} seconds.",
            "stdoutUrl": f"/api/runs/{run_id}/stdout",
            "stderrUrl": f"/api/runs/{run_id}/stderr",
            "scenarioUrl": f"/api/runs/{run_id}/scenario",
        }

    if completed.returncode != 0:
        return 200, {
            "ok": False,
            "runId": run_id,
            "exitCode": completed.returncode,
            "error": "Simulation returned a non-zero exit code.",
            "stdoutUrl": f"/api/runs/{run_id}/stdout",
            "stderrUrl": f"/api/runs/{run_id}/stderr",
            "scenarioUrl": f"/api/runs/{run_id}/scenario",
        }

    if not trace_path.is_file() or trace_path.stat().st_size == 0:
        return 200, {
            "ok": False,
            "runId": run_id,
            "exitCode": completed.returncode,
            "error": "Simulation completed, but trace.json was not produced.",
            "stdoutUrl": f"/api/runs/{run_id}/stdout",
            "stderrUrl": f"/api/runs/{run_id}/stderr",
            "scenarioUrl": f"/api/runs/{run_id}/scenario",
        }

    try:
        json.loads(trace_path.read_text(encoding="utf-8"))
    except json.JSONDecodeError as exc:
        return 200, {
            "ok": False,
            "runId": run_id,
            "exitCode": completed.returncode,
            "error": f"trace.json is invalid JSON: {exc.msg}.",
            "stdoutUrl": f"/api/runs/{run_id}/stdout",
            "stderrUrl": f"/api/runs/{run_id}/stderr",
            "scenarioUrl": f"/api/runs/{run_id}/scenario",
        }

    return 200, {
        "ok": True,
        "runId": run_id,
        "exitCode": completed.returncode,
        "executable": str(executable),
        "traceUrl": f"/api/runs/{run_id}/trace",
        "stdoutUrl": f"/api/runs/{run_id}/stdout",
        "stderrUrl": f"/api/runs/{run_id}/stderr",
        "scenarioUrl": f"/api/runs/{run_id}/scenario",
    }


def content_type_for(path: Path) -> str:
    suffix = path.suffix.lower()
    if suffix == ".html":
        return "text/html; charset=utf-8"
    if suffix == ".json":
        return "application/json; charset=utf-8"
    if suffix == ".svg":
        return "image/svg+xml"
    if suffix == ".txt" or suffix == ".commands":
        return "text/plain; charset=utf-8"
    if suffix == ".css":
        return "text/css; charset=utf-8"
    if suffix == ".js":
        return "text/javascript; charset=utf-8"
    return "application/octet-stream"


def make_handler(state: RunnerState) -> type[BaseHTTPRequestHandler]:
    class LocalRunnerHandler(BaseHTTPRequestHandler):
        server_version = "BattleLocalRunner/0.1"

        def end_headers(self) -> None:
            self.send_header("Cache-Control", "no-store")
            super().end_headers()

        def do_OPTIONS(self) -> None:
            self.send_response(405)
            self.send_header("Allow", "GET, POST")
            self.end_headers()

        def do_GET(self) -> None:
            self.handle_request(self._do_GET)

        def do_POST(self) -> None:
            self.handle_request(self._do_POST)

        def _do_GET(self) -> None:
            parsed = urlparse(self.path)
            path = unquote(parsed.path)
            if path.startswith("/api/"):
                self.handle_api_get(path)
                return
            self.serve_viewer_file(path)

        def _do_POST(self) -> None:
            parsed = urlparse(self.path)
            path = unquote(parsed.path)
            if path != "/api/run":
                self.send_json(404, {"ok": False, "error": "Endpoint not found."})
                return

            try:
                payload = self.read_json_body()
                status, response = execute_run(state, payload)
                self.send_json(status, response)
            except ValueError as exc:
                self.send_json(400, {"ok": False, "error": str(exc)})

        def handle_request(self, handler: Any) -> None:
            try:
                handler()
            except (BrokenPipeError, ConnectionResetError):
                raise
            except Exception as exc:  # local development helper: keep API failures structured.
                self.log_error("Unhandled local runner error: %s", exc)
                try:
                    self.send_json(500, {"ok": False, "error": "Internal local runner error."})
                except (BrokenPipeError, ConnectionResetError):
                    pass

        def handle_api_get(self, path: str) -> None:
            try:
                if path == "/api/health":
                    self.send_json(
                        200,
                        {
                            "ok": True,
                            "service": "battle-local-runner",
                            "bind": HOST,
                            "runsDir": str(state.runs_root),
                            "executable": state.executable_payload(),
                            "limits": {
                                "maxMapWidth": MAX_MAP_WIDTH,
                                "maxMapHeight": MAX_MAP_HEIGHT,
                                "maxTotalUnits": MAX_TOTAL_UNITS,
                            },
                        },
                    )
                    return
                if path == "/api/executable":
                    self.send_json(200, {"ok": True, "executable": state.executable_payload()})
                    return
                if path == "/api/runs":
                    run_ids = list_run_ids(state)
                    self.send_json(200, {"ok": True, "runs": [run_metadata(state, run_id) for run_id in run_ids]})
                    return
                if path == "/api/runs/latest":
                    run_ids = list_run_ids(state)
                    if not run_ids:
                        self.send_json(404, {"ok": False, "error": "No runs are available yet."})
                        return
                    self.send_json(200, {"ok": True, "run": run_metadata(state, run_ids[0])})
                    return

                parts = path.strip("/").split("/")
                if len(parts) == 4 and parts[0] == "api" and parts[1] == "runs":
                    run_id = parts[2]
                    artifact = parts[3]
                    run_dir = safe_run_dir(state, run_id)
                    mapping = {
                        "trace": ("trace.json", "application/json; charset=utf-8"),
                        "stdout": ("stdout.txt", "text/plain; charset=utf-8"),
                        "stderr": ("stderr.txt", "text/plain; charset=utf-8"),
                        "scenario": ("scenario.commands", "text/plain; charset=utf-8"),
                    }
                    if artifact not in mapping:
                        self.send_json(404, {"ok": False, "error": "Run artifact not found."})
                        return
                    filename, content_type = mapping[artifact]
                    status, body, response_type = run_file_response(run_dir, filename, content_type)
                    self.send_bytes(status, body, response_type)
                    return
            except FileNotFoundError:
                self.send_json(404, {"ok": False, "error": "Run not found."})
                return
            except ValueError as exc:
                self.send_json(400, {"ok": False, "error": str(exc)})
                return

            self.send_json(404, {"ok": False, "error": "Endpoint not found."})

        def serve_viewer_file(self, path: str) -> None:
            if path in ("", "/", "/index.html"):
                target = state.viewer_root / "index.html"
            else:
                relative = path.lstrip("/")
                if relative.startswith("replay-viewer/"):
                    relative = relative[len("replay-viewer/") :]
                target = (state.viewer_root / relative).resolve()
                if not is_relative_to(target, state.viewer_root.resolve()):
                    self.send_json(403, {"ok": False, "error": "Forbidden path."})
                    return

            if not target.is_file():
                self.send_json(404, {"ok": False, "error": "File not found."})
                return
            self.send_bytes(200, target.read_bytes(), content_type_for(target))

        def read_json_body(self) -> dict[str, Any]:
            raw_length = self.headers.get("Content-Length")
            length = parse_int(raw_length or 0, "Content-Length", 0, MAX_BODY_BYTES)
            raw_body = self.rfile.read(length)
            if not raw_body:
                return {}
            try:
                payload = json.loads(raw_body.decode("utf-8"))
            except json.JSONDecodeError as exc:
                raise ValueError(f"Request body must be JSON: {exc.msg}.") from exc
            if not isinstance(payload, dict):
                raise ValueError("Request JSON must be an object.")
            return payload

        def send_json(self, status: int, payload: dict[str, Any]) -> None:
            self.send_bytes(status, json_bytes(payload), "application/json; charset=utf-8")

        def send_bytes(self, status: int, body: bytes, content_type: str) -> None:
            self.send_response(status)
            self.send_header("Content-Type", content_type)
            self.send_header("Content-Length", str(len(body)))
            self.end_headers()
            self.wfile.write(body)

        def log_message(self, format: str, *args: Any) -> None:
            print(f"{self.address_string()} - {format % args}")

    return LocalRunnerHandler


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description="Serve the local battle_sim runner UI helper.")
    parser.add_argument("--port", type=int, default=DEFAULT_PORT, help=f"Port on {HOST} to listen on.")
    parser.add_argument("--exe", help="Path to battle_sim executable. Overrides auto-detection.")
    parser.add_argument("--runs-dir", help="Directory for generated run artifacts.")
    parser.add_argument("--timeout", type=int, default=30, help="Simulation timeout in seconds.")
    return parser.parse_args()


def main() -> None:
    args = parse_args()
    if args.port < 1 or args.port > 65535:
        raise SystemExit("--port must be between 1 and 65535.")
    if args.timeout < 1:
        raise SystemExit("--timeout must be at least 1 second.")

    state = RunnerState(args.exe, args.runs_dir, args.timeout)
    server = ThreadingHTTPServer((HOST, args.port), make_handler(state))
    print(f"Battle local runner listening on http://{HOST}:{args.port}")
    print(f"Serving replay viewer from {state.viewer_root}")
    print(state.executable_payload()["message"])
    try:
        server.serve_forever()
    except KeyboardInterrupt:
        print("\nStopping local runner.")
    finally:
        server.server_close()


if __name__ == "__main__":
    main()
