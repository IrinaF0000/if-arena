#!/usr/bin/env python3
from pathlib import Path
import sys

ROOT = Path(__file__).resolve().parents[2]

expected_tasks = [
    '0016-clean-foundation-before-playable-slice.md',
    '0017-core-arena-map-and-config.md',
    '0018-core-player-movement-and-collision.md',
    '0019-core-objective-run-rules.md',
    '0020-core-combat-dash-and-hazards.md',
    '0021-protocol-playable-game-messages.md',
    '0022-backend-match-loop-and-fake-sessions.md',
    '0023-server-app-and-local-config.md',
    '0024-cli-two-player-playable-flow.md',
    '0025-tcp-vertical-slice-integration.md',
    '0026-websocket-and-telegram-playable-slice.md',
    '0027-qt-playable-client.md',
    '0028-load-security-hardening-and-docs.md',
]

required_files = [
    'docs/project/PLAYABLE_GAME_AGENT_PLAN.md',
    'docs/agent-rules/process/SEQUENTIAL_AGENT_PIPELINE.md',
    'docs/agent-seeds/CODEX_ORCHESTRATOR_PROMPT.md',
    'docs/review/QUALITY_GATES.md',
    'docs/agent-tasks/README.md',
]

errors: list[str] = []

for rel in required_files:
    if not (ROOT / rel).exists():
        errors.append(f'missing required file: {rel}')

for task in expected_tasks:
    path = ROOT / 'docs' / 'agent-tasks' / task
    if not path.exists():
        errors.append(f'missing playable task packet: {task}')
        continue
    text = path.read_text(encoding='utf-8')
    for marker in [
        '## Required quality gates',
        'Gate K: sequential agent pipeline',
        '## Sequential agent pipeline',
        'docs/agent-rules/process/SEQUENTIAL_AGENT_PIPELINE.md',
        'Implementation-Agent must not commit',
        '## Required reviews',
        '## Acceptance criteria',
    ]:
        if marker not in text:
            errors.append(f'{task}: missing marker {marker!r}')

readme = ROOT / 'docs' / 'agent-tasks' / 'README.md'
if readme.exists():
    readme_text = readme.read_text(encoding='utf-8')
    last_pos = -1
    for task in expected_tasks:
        pos = readme_text.find(task)
        if pos < 0:
            errors.append(f'docs/agent-tasks/README.md does not reference {task}')
            continue
        if pos < last_pos:
            errors.append(f'docs/agent-tasks/README.md references {task} out of order')
        last_pos = pos

plan = ROOT / 'docs' / 'project' / 'PLAYABLE_GAME_AGENT_PLAN.md'
if plan.exists():
    plan_text = plan.read_text(encoding='utf-8')
    for scenario in ['Scenario A', 'Scenario B', 'Scenario C', 'Scenario D', 'Scenario E']:
        if scenario not in plan_text:
            errors.append(f'playable plan missing {scenario}')
    for task in expected_tasks:
        if task[:4] not in plan_text:
            errors.append(f'playable plan missing task id {task[:4]}')

pipeline = ROOT / 'docs' / 'agent-rules' / 'process' / 'SEQUENTIAL_AGENT_PIPELINE.md'
if pipeline.exists():
    pipeline_text = pipeline.read_text(encoding='utf-8')
    for role in ['Coordinator preflight', 'Implementation-Agent', 'Verification-Agent', 'Review-Agent', 'Fix-Agent', 'Commit-Agent']:
        if role not in pipeline_text:
            errors.append(f'sequential pipeline missing role {role}')

quality = ROOT / 'docs' / 'review' / 'QUALITY_GATES.md'
if quality.exists():
    quality_text = quality.read_text(encoding='utf-8')
    for gate in ['Gate K. Sequential agent pipeline', 'Gate L. Playable-game scenario integrity']:
        if gate not in quality_text:
            errors.append(f'quality gates missing {gate}')

if errors:
    print('Playable task sequence validation failed:')
    for error in errors:
        print(f'  - {error}')
    sys.exit(1)

print('Playable task sequence OK')
