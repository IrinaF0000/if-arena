#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${BUILD_DIR:-build}"
CONFIG_PATH="${IF_ARENA_CONFIG:-config/examples/server.local.json}"

cmake -S . -B "${BUILD_DIR}" -DBATTLE_BUILD_TESTS=ON -DBATTLE_BUILD_QT_CLIENT=OFF
cmake --build "${BUILD_DIR}" --target battle_server_app --parallel

echo "Starting battle_server_app with ${CONFIG_PATH}."
echo "Current status: config/backend initialization is available; listener startup reports clear not-implemented errors until transport integration tasks land."
exec "${BUILD_DIR}/battle_server_app" --config "${CONFIG_PATH}" --local "$@"
