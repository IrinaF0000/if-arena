#!/usr/bin/env python3
"""Validate lightweight IF Arena architecture boundaries."""

from __future__ import annotations

import argparse
import re
import sys
from pathlib import Path

CODE_SUFFIXES = {".c", ".cc", ".cpp", ".cxx", ".h", ".hh", ".hpp", ".hxx"}
SKIP_DIRS = {".git", "build", "build-qt-mingw", "dist", "node_modules", "__pycache__"}
INCLUDE_RE = re.compile(r"^\s*#\s*include\s+[<\"]([^>\"]+)[>\"]", re.MULTILINE)
TARGET_LINK_RE = re.compile(r"target_link_libraries\s*\(\s*([^\s\)]+)(.*?)\)", re.IGNORECASE | re.DOTALL)
CMAKE_KEYWORDS = {
    "PUBLIC",
    "PRIVATE",
    "INTERFACE",
    "debug",
    "optimized",
    "general",
}


def read_text(path: Path) -> str:
    try:
        return path.read_text(encoding="utf-8")
    except UnicodeDecodeError:
        return path.read_text(encoding="utf-8", errors="ignore")


def normalized(path: Path) -> str:
    return path.as_posix()


def iter_files(root: Path, relative: str, suffixes: set[str]) -> list[Path]:
    base = root / relative
    if not base.exists():
        return []

    files: list[Path] = []
    for path in base.rglob("*"):
        if any(part in SKIP_DIRS for part in path.relative_to(root).parts):
            continue
        if path.is_file() and path.suffix.lower() in suffixes:
            files.append(path)
    return files


def includes_in(path: Path) -> list[str]:
    return INCLUDE_RE.findall(read_text(path))


def is_qt_include(include: str) -> bool:
    lowered = include.lower()
    return lowered.startswith("qt") or (include.startswith("Q") and "/" not in include)


def check_core_includes(root: Path) -> list[str]:
    errors: list[str] = []
    forbidden_modules = [
        "battle_protocol",
        "battle_backend",
        "battle_transport_tcp",
        "battle_transport_ws",
        "battle_qt_client",
    ]
    socket_headers = {
        "winsock.h",
        "winsock2.h",
        "ws2tcpip.h",
        "asio.hpp",
        "boost/asio.hpp",
        "sys/socket.h",
        "netinet/in.h",
        "arpa/inet.h",
    }
    forbidden_std_io = {"filesystem", "fstream", "iostream"}
    server_terms = {"battle_server", "server_app", "environment", "secret"}

    for path in iter_files(root, "src/battle_core", CODE_SUFFIXES):
        rel = normalized(path.relative_to(root))
        for include in includes_in(path):
            lowered = include.lower().replace("\\", "/")
            if any(module in lowered for module in forbidden_modules):
                errors.append(f"{rel}: forbidden battle_core include {include!r}")
            if is_qt_include(include):
                errors.append(f"{rel}: forbidden Qt include {include!r}")
            if lowered in socket_headers or lowered.startswith("boost/asio/"):
                errors.append(f"{rel}: forbidden socket/asio include {include!r}")
            if lowered in forbidden_std_io:
                errors.append(f"{rel}: forbidden IO include {include!r}")
            if any(term in lowered for term in server_terms):
                errors.append(f"{rel}: forbidden server/environment include {include!r}")
    return errors


def parse_target_links(cmake_text: str) -> dict[str, set[str]]:
    links: dict[str, set[str]] = {}
    for match in TARGET_LINK_RE.finditer(cmake_text):
        target = match.group(1)
        body = re.sub(r"#.*", " ", match.group(2))
        tokens = re.split(r"[\s\r\n]+", body.strip())
        deps = {token for token in tokens if token and token not in CMAKE_KEYWORDS}
        links.setdefault(target, set()).update(deps)
    return links


def check_cmake_targets(root: Path) -> list[str]:
    cmake = root / "CMakeLists.txt"
    if not cmake.exists():
        return []

    errors: list[str] = []
    links = parse_target_links(read_text(cmake))
    forbidden_by_target = {
        "battle_core": {
            "battle_protocol",
            "battle_backend",
            "battle_backend_api",
            "battle_transport_tcp",
            "battle_transport_ws",
            "battle_qt_client",
            "ws2_32",
        },
        "battle_protocol": {
            "battle_core",
            "battle_backend",
            "battle_backend_api",
            "battle_transport_tcp",
            "battle_transport_ws",
            "battle_qt_client",
        },
        "battle_qt_client": {"battle_core", "battle_backend", "battle_backend_api"},
        "battle_transport_tcp": {"battle_core", "battle_qt_client"},
        "battle_transport_ws": {"battle_core", "battle_qt_client"},
    }

    for target, forbidden in forbidden_by_target.items():
        deps = links.get(target, set())
        for dep in sorted(deps):
            if dep in forbidden or (target == "battle_core" and dep.startswith("Qt")):
                errors.append(f"CMakeLists.txt: target {target} links forbidden dependency {dep}")

    for target, deps in links.items():
        if target == "battle_server_app":
            continue
        links_transport = any(dep.startswith("battle_transport_") for dep in deps)
        links_backend = "battle_backend" in deps
        if links_transport and links_backend:
            errors.append(f"CMakeLists.txt: target {target} combines backend and transports outside battle_server_app")

    return errors


def check_client_includes(root: Path) -> list[str]:
    errors: list[str] = []
    client_roots = ["src/battle_qt_client", "src/battle_cli_client", "tools/load_client"]
    forbidden_terms = {"battle_backend", "src/battle_backend"}

    for client_root in client_roots:
        for path in iter_files(root, client_root, CODE_SUFFIXES):
            rel = normalized(path.relative_to(root))
            for include in includes_in(path):
                lowered = include.lower().replace("\\", "/")
                if any(term in lowered for term in forbidden_terms):
                    errors.append(f"{rel}: client includes backend internals via {include!r}")
                if client_root == "src/battle_qt_client" and "battle_core" in lowered:
                    errors.append(f"{rel}: Qt client includes battle_core via {include!r}")
    return errors


def check_transport_includes(root: Path) -> list[str]:
    errors: list[str] = []
    forbidden_terms = {"battle_qt_client", "frontend", "telegram_mini_app"}

    for transport_root in ["src/battle_transport_tcp", "src/battle_transport_ws"]:
        for path in iter_files(root, transport_root, CODE_SUFFIXES):
            rel = normalized(path.relative_to(root))
            for include in includes_in(path):
                lowered = include.lower().replace("\\", "/")
                if any(term in lowered for term in forbidden_terms) or is_qt_include(include):
                    errors.append(f"{rel}: transport includes UI/client header {include!r}")
    return errors


def check_svg_assets(root: Path) -> list[str]:
    errors: list[str] = []
    forbidden_patterns = [
        ("script", re.compile(r"<\s*script\b", re.IGNORECASE)),
        ("foreignObject", re.compile(r"<\s*foreignObject\b", re.IGNORECASE)),
        ("external URL", re.compile(r"(?:href|xlink:href)\s*=\s*['\"]https?://", re.IGNORECASE)),
    ]

    for path in root.rglob("*.svg"):
        parts = set(path.relative_to(root).parts)
        if parts & (SKIP_DIRS | {"external"}):
            continue
        text = read_text(path)
        rel = normalized(path.relative_to(root))
        for label, pattern in forbidden_patterns:
            if pattern.search(text):
                errors.append(f"{rel}: SVG contains forbidden {label}")
    return errors


def validate(root: Path) -> list[str]:
    root = root.resolve()
    errors: list[str] = []
    errors.extend(check_core_includes(root))
    errors.extend(check_cmake_targets(root))
    errors.extend(check_client_includes(root))
    errors.extend(check_transport_includes(root))
    errors.extend(check_svg_assets(root))
    return errors


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--root", default=Path(__file__).resolve().parents[2], type=Path)
    args = parser.parse_args(argv)

    errors = validate(args.root)
    if errors:
        print("Architecture boundary validation failed:", file=sys.stderr)
        for error in errors:
            print(f"  - {error}", file=sys.stderr)
        return 1

    print("Architecture boundaries OK")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
