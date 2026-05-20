#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${BUILD_DIR:-build}"
HOST="${IF_ARENA_HOST:-127.0.0.1}"
PORT="${IF_ARENA_PORT:-5555}"

cmake -S . -B "${BUILD_DIR}" -DBATTLE_BUILD_TESTS=ON -DBATTLE_BUILD_QT_CLIENT=OFF
cmake --build "${BUILD_DIR}" --target battle_server_app --parallel

echo "Starting battle_server_app on ${HOST}:${PORT}."
echo "Current status: foundation executable only; playable match loop starts in later tasks."
exec "${BUILD_DIR}/battle_server_app" --host "${HOST}" --port "${PORT}" "$@"
