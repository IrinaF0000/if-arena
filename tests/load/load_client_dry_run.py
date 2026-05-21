#!/usr/bin/env python3
from __future__ import annotations

import subprocess
import sys
import tempfile
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]
LOAD_CLIENT = ROOT / "build" / "battle_load_client.exe"


def run(*args: str) -> subprocess.CompletedProcess[str]:
    return subprocess.run([str(LOAD_CLIENT), *args], cwd=ROOT, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)


def main() -> int:
    if not LOAD_CLIENT.exists():
        print(f"load client not found: {LOAD_CLIENT}", file=sys.stderr)
        return 1

    scenarios = ["connect_only", "create_and_join", "gameplay", "slow_readers", "command_spam", "invalid_payloads"]
    with tempfile.TemporaryDirectory() as tmp:
        for scenario in scenarios:
            output = Path(tmp) / f"{scenario}.md"
            result = run(
                "--dry-run",
                "--endpoint",
                "127.0.0.1:5555",
                "--clients",
                "4",
                "--duration",
                "3",
                "--command-rate",
                "2",
                "--seed",
                "42",
                "--scenario",
                scenario,
                "--output",
                str(output),
            )
            if result.returncode != 0:
                print(result.stderr, file=sys.stderr)
                return 1
            text = output.read_text(encoding="utf-8")
            required = [f"- scenario: {scenario}", "- seed: 42", "- clientsStarted: 4", "- commandsPlanned:"]
            if any(item not in text for item in required):
                print(f"missing report fields for {scenario}", file=sys.stderr)
                return 1

        invalid = run("--dry-run", "--scenario", "not_a_scenario")
        if invalid.returncode == 0 or "unknown scenario" not in invalid.stderr:
            print("invalid scenario was not rejected", file=sys.stderr)
            return 1

        too_large = run("--dry-run", "--clients", "100001")
        if too_large.returncode == 0 or "exceeds configured bound" not in too_large.stderr:
            print("oversized client count was not rejected", file=sys.stderr)
            return 1

    print("[PASS] load_client_dry_run")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
