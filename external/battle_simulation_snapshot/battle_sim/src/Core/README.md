# Core

Generic engine infrastructure only.

Allowed:
- ECS world and component storage;
- resources;
- type-erased event bus and event-sink interface;
- scheduler;
- handler registries;
- mutation pipeline;
- feature-pack interfaces.

Kept outside Core:
- battle mechanics;
- unit archetypes;
- map-cell rules;
- health/damage semantics;
- legacy command parsing or output.

Battle-owned components, events, map resource, systems, mutations, and archetype recipes live under `Features/Battle` or `Features/UnitsClassic`.
