# Documentation

This index maps the public-facing docs for the C++20 gameplay architecture sandbox.

## Start Here

- `../README.md`: quick start, sample output, project highlights.
- `architecture.md`: module boundaries and dependency direction.
- `game-loop.md`: CLI scenario flow and scheduled runtime loop.
- `system-order.md`: command setup order, scheduler phases, and feature-pack order.

## Gameplay And Data

- `configuration.md`: scenario config format and feature selection.
- `data-driven-archetypes.md`: JSON archetypes and simple rule arrays.
- `json-trace.md`: optional trace output and `inspect` summaries.
- `../tools/replay-viewer/README.md`: static browser replay viewer for trace files.
- `../tools/local-runner/README.md`: local-only browser helper for generating and replaying traces.
- `deterministic-simulation.md`: replay and golden-output validation.

## Engineering Process

- `ai-assisted-development.md`: AI-assisted development principles and quality gates.
- `agent-workflow.md`: scoped local agent workflow.
- `case-studies/add-json-trace-agent-run.md`: concrete agent-run case study.

## Roadmap And Notes

- `architecture-roadmap.md`: completed architecture milestones and optional future work.
- `performance-notes.md`: current runtime shape and measurement plan.

## Examples

- `../examples/add-new-mechanic.md`: adding a feature-owned unit mechanic.
- `../config/archetypes/spearman.json`: JSON-defined unit mechanic.
