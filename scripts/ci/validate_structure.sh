#!/usr/bin/env bash
set -euo pipefail

required_paths=(
  "CMakeLists.txt"
  "README.md"
  "PROJECT_MAP.md"
  "docs/agent-seeds/AGENT_RULES_TEMPLATE.md"
  "docs/agent-seeds/LLM_TOKEN_BUDGET.md"
  "docs/ci/CI_CD_GUARDRAILS.md"
  "docs/review/QUALITY_GATES.md"
  "docs/security/SECRETS_MANAGEMENT.md"
  "scripts/ci/scan_secrets.py"
  ".gitleaks.toml"
  "frontend/telegram_mini_app/.env.example"
  ".env.example"
  "docs/security/SECURITY_REQUIREMENTS.md"
  "docs/architecture/TECHNICAL_ARCHITECTURE.md"
  "docs/architecture/TRANSPORT_ABSTRACTION.md"
  "docs/telegram/TELEGRAM_SECURITY.md"
  "src/battle_core"
  "src/battle_backend"
  "src/battle_protocol"
  "src/battle_transport_tcp"
  "src/battle_transport_ws"
  "src/battle_qt_client"
  "frontend/telegram_mini_app"
  "external/battle_simulation_snapshot"
  ".github/workflows/pr-ci.yml"
  ".github/workflows/main-ci.yml"
)

for path in "${required_paths[@]}"; do
  if [ ! -e "$path" ]; then
    echo "Missing required path: $path" >&2
    exit 1
  fi
done

if find . -type d \( -name build -o -name node_modules -o -name .git \) -prune -false -o -type f -size +5M | grep -q .; then
  echo "Large files over 5 MB are present. Keep binaries, dependencies, traces, and build outputs out of the repository." >&2
  find . -type d \( -name build -o -name node_modules -o -name .git \) -prune -false -o -type f -size +5M >&2
  exit 1
fi

python3 scripts/ci/scan_secrets.py

echo "Repository structure looks valid."

python3 scripts/agent/validate_agent_harness.py
