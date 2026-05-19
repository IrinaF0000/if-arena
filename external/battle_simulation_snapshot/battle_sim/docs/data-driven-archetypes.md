# Data-Driven Archetypes

The project supports a small JSON-defined archetype slice for deterministic gameplay experiments.

The current slice is intentionally small:

- one flat JSON object per archetype file;
- required base component fields only;
- action behavior selected from a small built-in template list or a simple `rules` array;
- registration through scenario config `archetypeFile`;
- spawning through the existing generic `SPAWN` command.

The slice focuses on a small, explicit archetype schema rather than a general JSON parser, scripting system, or arbitrary action graph format.

## Example

`config/archetypes/duelist.json`:

```json
{
  "id": "Duelist",
  "actionTemplate": "adjacent-striker",
  "hp": 6,
  "strength": 2,
  "mobile": true,
  "blocksCell": true,
  "attackable": true,
  "actionBudget": 1
}
```

`config/data-driven-duelist.cfg`:

```text
feature battle.basic
feature units.classic
feature io.legacy

archetypeFile config/archetypes/duelist.json
```

The scenario can then use:

```text
SPAWN Duelist 1 0 0
```

Rule-array example:

```bash
./build/battle_sim commands_example.txt config/data-driven-spearman.cfg
```

## Schema

Supported fields:

| Field | Type | Required | Meaning |
| --- | --- | --- | --- |
| `id` | string | yes | Archetype id used by generic `SPAWN`. |
| `hp` | positive integer | yes | Base health component. |
| `actionTemplate` | string | no | Built-in action template. Defaults to `adjacent-striker`. |
| `strength` | integer | no | Base strength component. Defaults to `0`. |
| `agility` | integer | no | Base agility component. Defaults to `0`. |
| `range` | integer | no | Base range component. Defaults to `0`. |
| `spirit` | integer | no | Base spirit component. Defaults to `0`. |
| `power` | integer | no | Base power component. Defaults to `0`. |
| `mobile` | boolean | no | Whether the entity gets movement capability. Defaults to `true`. |
| `blocksCell` | boolean | no | Whether the entity occupies a blocking map cell. Defaults to `true`. |
| `attackable` | boolean | no | Whether the entity can be targeted by attackable-only rules. Defaults to `true`. |
| `actionBudget` | positive integer | no | Points per turn. Defaults to `1`. |
| `rules` | array | no | Simple action-rule array. When present, it replaces `actionTemplate`. |

Supported `actionTemplate` values:

| Template | Behavior |
| --- | --- |
| `adjacent-striker` | Swordsman-like adjacent strength attack, then move. |
| `lancer-reach` | Lancer-like reach attack, adjacent fallback, then move. |

## Limits

Rule arrays support existing registered handler ids only. They do not evaluate expressions, run scripts, or define new handlers.

Supported rule fields:

| Field | Type | Meaning |
| --- | --- | --- |
| `priority` | integer | Higher priority rules run first. |
| `condition` | string | Registered condition handler id. |
| `selector` | string | Registered target selector handler id. |
| `targetPicker` | string | Registered target picker handler id. |
| `effect` | string | Registered effect handler id. Required. |
| `amount` | integer | Literal effect amount. |
| `amountValueResolver` | string | Registered value resolver for amount. |
| `minDistance` / `maxDistance` | integer | Distance selector parameters. |
| `stepDistance` | integer | Movement distance. |
| `stepDistanceValueResolver` | string | Registered value resolver for movement distance. |
| `cost` | positive integer | Action budget cost. Defaults to `1`. |
| `allowEmptyTargets` | boolean | Allows effects like movement without targets. |
| `requireAttackableTarget` | boolean | Filters target list to attackable entities. |

`config/archetypes/spearman.json` is the rule-array example.
