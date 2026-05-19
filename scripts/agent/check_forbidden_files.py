#!/usr/bin/env python3
"""Advisory helper for agents.

Usage:
  python3 scripts/agent/check_forbidden_files.py file1 file2 ...

This does not replace review. It prints warnings for protected paths.
"""
from pathlib import Path
import sys

PROTECTED_PREFIXES = [
    '.github/workflows/',
    'scripts/ci/',
    'deploy/',
    'docs/ci/',
    'external/battle_simulation_snapshot/',
]

hit = False
for raw in sys.argv[1:]:
    path = raw.replace('\\', '/')
    for prefix in PROTECTED_PREFIXES:
        if path == prefix.rstrip('/') or path.startswith(prefix):
            print(f'Protected path touched: {path}')
            hit = True
            break
if hit:
    print('Ensure the task packet explicitly allows these paths and requires the right review gates.')
    sys.exit(2)
print('No protected paths detected in provided file list.')
