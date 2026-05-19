# Repository Structure

```text
if_arena/
  external/
    battle_simulation_snapshot/       empty slot for old project copy, read-only after import

  src/
    battle_core/                      extracted static library from old simulation
    battle_protocol/                  length-prefixed JSON protocol and validation
    battle_server/                    authoritative async TCP backend
    battle_cli_client/                debug client
    battle_qt_client/                 Qt Widgets desktop client

  tools/
    load_client/                      synthetic multi-client load generator
    replay_viewer/                    replay/debug visualization tools

  tests/
    unit/core/                        extracted core tests
    unit/protocol/                    frame codec and message validation tests
    integration/server/               multi-client server tests
    security/                         hostile input and abuse tests
    load/                             repeatable load scenarios

  docs/
    project/                          PRD, implementation plan, import plan, backlog
    architecture/                     architecture, protocol, ADRs
    security/                         security requirements and threat model
    review/                           code review checklist, quality gates, security review prompt
    quality/                          test plan
    performance/                      load testing and reports
    operations/                       observability and deployment operations
    agent-seeds/                      transferred agent development templates

  deploy/
    docker/                           container files
    oracle-cloud/                     public VM deployment notes
    railway/                          TCP proxy deployment notes

  config/
    examples/                         local/public server configs
    scenarios/                        game scenarios
    feature_packs/                    unit/action definitions

  reports/
    load/                             load test outputs
  replays/                            saved match traces
```
```
