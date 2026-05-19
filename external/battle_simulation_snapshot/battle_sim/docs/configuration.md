# Configuration

The app-level scenario config controls feature selection and deterministic replay settings.

The CLI accepts:

```bash
./build/battle_sim commands_example.txt
./build/battle_sim commands_example.txt config/default.cfg
```

With no config argument, the app uses built-in defaults equivalent to `config/default.cfg`.

## Format

The config file is line-oriented text. Blank lines and `#` comments are ignored.

```text
feature battle.basic
feature units.classic
feature io.legacy debugSummary=true
rngSeed 123
archetypeFile config/archetypes/duelist.json
```

Supported directives:

| Directive | Meaning |
| --- | --- |
| `feature <id>` | Enables a built-in feature pack. Current scenario runtime requires `battle.basic`, `units.classic`, and `io.legacy`. |
| `feature <id> enabled=false` | Explicitly disables a feature. Required features may not be disabled for the current CLI path. |
| `feature io.legacy debugSummary=true` | Emits a short debug summary to stderr after the run. Legacy stdout event lines are unchanged. |
| `rngSeed <number>` | Uses a fixed RNG seed for deterministic replay. |
| `rngSeed random` | Leaves RNG selection random, matching historical command-file behavior. |
| `maxTicks <number>` | Parses into the generic config model for callers that use scheduled engine settings. |
| `archetypeFile <path>` | Loads one minimal JSON archetype file before command parsing. |

Unknown directives, unknown feature packs, missing required feature packs, and missing config files produce runtime diagnostics.

## Current Scope

Config selection validates the feature set used by the current scenario runtime. Data-driven archetypes are limited to the schema in `docs/data-driven-archetypes.md`.
