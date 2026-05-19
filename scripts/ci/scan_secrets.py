#!/usr/bin/env python3
"""Lightweight repository secret scanner for CI and local agent checks.

This is not a replacement for a dedicated secret-scanning service. It is a
portable guardrail intended to catch obvious accidental commits in this
portfolio repository before review.
"""
from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[2]

SKIP_DIRS = {
    ".git",
    "build",
    "cmake-build-debug",
    "cmake-build-release",
    "node_modules",
    "dist",
    ".vite",
    "out",
    "bin",
    "lib",
}

SKIP_SUFFIXES = {
    ".zip", ".png", ".jpg", ".jpeg", ".gif", ".webp", ".ico", ".pdf",
    ".exe", ".dll", ".so", ".dylib", ".a", ".lib", ".o", ".obj",
}

ALLOW_MARKERS = (
    "REPLACE_ME",
    "PLACEHOLDER",
    "EXAMPLE",
    "example.com",
    "DO_NOT_COMMIT_REAL_TOKEN",
    "GENERATE_32_BYTES_OR_MORE",
)

PATTERNS: list[tuple[str, re.Pattern[str]]] = [
    ("telegram_bot_token", re.compile(r"\b\d{8,12}:[A-Za-z0-9_-]{30,}\b")),
    ("private_key", re.compile(r"-----BEGIN (?:RSA |EC |OPENSSH |DSA |)?PRIVATE KEY-----")),
    ("github_pat", re.compile(r"\bgh[pousr]_[A-Za-z0-9_]{36,}\b")),
    ("generic_secret_assignment", re.compile(
        r"(?i)\b(?:secret|token|password|passwd|api[_-]?key|private[_-]?key)\b\s*[:=]\s*['\"]?[A-Za-z0-9_./+=:@-]{20,}"
    )),
    ("aws_access_key_id", re.compile(r"\bAKIA[0-9A-Z]{16}\b")),
]


def should_skip(path: Path) -> bool:
    rel_parts = path.relative_to(ROOT).parts
    if any(part in SKIP_DIRS for part in rel_parts):
        return True
    if path.suffix.lower() in SKIP_SUFFIXES:
        return True
    return False


def is_allowed(line: str) -> bool:
    return any(marker in line for marker in ALLOW_MARKERS)


def main() -> int:
    findings: list[str] = []
    for path in ROOT.rglob("*"):
        if not path.is_file() or should_skip(path):
            continue
        try:
            text = path.read_text(encoding="utf-8")
        except UnicodeDecodeError:
            continue
        for lineno, line in enumerate(text.splitlines(), start=1):
            if is_allowed(line):
                continue
            for name, pattern in PATTERNS:
                if pattern.search(line):
                    rel = path.relative_to(ROOT)
                    findings.append(f"{rel}:{lineno}: possible {name}")
                    break
    if findings:
        print("Potential secrets detected. Remove the secret, rotate it if it was real, and commit only placeholders.", file=sys.stderr)
        for finding in findings:
            print(f"  - {finding}", file=sys.stderr)
        return 1
    print("Secret scan passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
