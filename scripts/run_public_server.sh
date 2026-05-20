#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${BUILD_DIR:-build}"
CONFIG_PATH="${IF_ARENA_CONFIG:-config/examples/server.public.json}"

cmake -S . -B "${BUILD_DIR}" -DBATTLE_BUILD_TESTS=ON -DBATTLE_BUILD_QT_CLIENT=OFF
cmake --build "${BUILD_DIR}" --target battle_server_app --parallel

echo "Starting battle_server_app with ${CONFIG_PATH}."
echo "Public status: config validation is available; public listener deployment is not ready until transport/auth integration tasks land."
exec "${BUILD_DIR}/battle_server_app" --config "${CONFIG_PATH}" "$@"
