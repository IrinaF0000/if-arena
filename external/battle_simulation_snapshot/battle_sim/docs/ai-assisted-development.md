# AI-Assisted Development

This project treats AI assistance as an engineering support tool under human-owned design and final-code accountability.

## Principles

- AI may help with decomposition, code navigation, boilerplate, refactoring suggestions, test ideas, and review checklists.
- Architecture decisions remain human-owned.
- Generated or suggested code is treated as untrusted until reviewed, built, tested, and validated against project boundaries.
- Small task packets are preferred over broad, open-ended edits.
- Behavior compatibility is protected by deterministic scenario tests and command-output validation.

## Workflow

1. Define a small task packet.
2. State allowed files, forbidden files, non-goals, and acceptance criteria.
3. Implement the smallest useful change.
4. Run build, tests, and scenario validation.
5. Review architecture boundaries and public-facing docs.
6. Update documentation only after behavior is verified.

## Quality gates

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure
./build/battle_sim commands_example.txt
./build/battle_sim commands_validation.txt
git diff --check
```

Additional checks before publication:

```bash
find . \( -path './build*' -o -name '*.exe' -o -name '*.a' -o -name 'CMakeCache.txt' -o -name 'build.ninja' \) -print
grep -RInE "internal-marker|unpublished-note|local-only" . --exclude-dir=.git --exclude-dir=build --exclude-dir=build-mingw
```

## Agent workflow notes

Role boundaries and review gates are documented in `docs/agent-workflow.md`.
