#!/usr/bin/env python3
"""Create a compact agent run note from git status.

This script is intentionally simple and advisory.
"""
from pathlib import Path
import subprocess
import datetime

ROOT = Path(__file__).resolve().parents[2]
out_dir = ROOT / 'docs' / 'agent-runs'
out_dir.mkdir(parents=True, exist_ok=True)
changed = subprocess.run(['git', 'status', '--short'], cwd=ROOT, text=True, capture_output=True)
now = datetime.datetime.utcnow().strftime('%Y%m%d-%H%M%S')
path = out_dir / f'agent-run-{now}.md'
path.write_text(f"""# Agent Run {now} UTC

## Changed files

```text
{changed.stdout.strip() or 'No git status output.'}
```

## Checks run

- TODO

## Decisions

- TODO

## Risks or follow-ups

- TODO
""", encoding='utf-8')
print(path)
