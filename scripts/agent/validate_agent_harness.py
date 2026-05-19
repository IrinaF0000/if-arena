#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]
required = [
    'AGENTS.md',
    '.agents/skills/cpp-secure-networking/SKILL.md',
    '.agents/skills/highload-backend/SKILL.md',
    '.agents/skills/qt-client/SKILL.md',
    '.agents/skills/telegram-miniapp-security/SKILL.md',
    '.agents/skills/websocket-gateway/SKILL.md',
    '.agents/skills/cmake-ci/SKILL.md',
    '.agents/skills/agent-manager/SKILL.md',
    'docs/agent-rules/README.md',
    'docs/agent-rules/process/TOKEN_ECONOMY.md',
    'docs/agent-rules/security/NETWORK_INPUT.md',
    'docs/agent-rules/security/TELEGRAM_AUTH.md',
    'docs/agent-rules/quality/CPP.md',
    'docs/agent-manager/AGENT_MANAGER_ROLE.md',
    'src/battle_core/AGENTS.md',
    'src/battle_transport_tcp/AGENTS.md',
    'src/battle_transport_ws/AGENTS.md',
    'src/battle_qt_client/AGENTS.md',
    'frontend/telegram_mini_app/AGENTS.md',
    '.github/AGENTS.md',
]
missing = [p for p in required if not (ROOT / p).exists()]
if missing:
    print('Missing agent harness files:')
    for p in missing:
        print(f'  - {p}')
    sys.exit(1)
print('Agent harness structure OK')
