# Local Runner

`tools/local-runner/server.py` is a small local-only HTTP helper for running `battle_sim` from the replay viewer.

The browser cannot launch an executable directly, so this helper binds to `127.0.0.1`, generates a command file, runs the simulator, stores run artifacts, and returns the produced `trace.json` to the viewer.

## CLI Contract

The simulator runner uses the current command line:

```bash
battle_sim <commands-file> [config-file] [--trace-json <trace-file>]
```

Trace inspection remains separate:

```bash
battle_sim inspect <trace-file>
```

The local runner MVP generates a command file and calls:

```bash
battle_sim <generated-scenario.commands> --trace-json <run-dir>/trace.json
```

## Usage

Build the simulator:

```bash
cmake -S . -B build
cmake --build build --parallel
```

Start the helper:

```bash
python tools/local-runner/server.py --exe build/battle_sim
```

On Windows multi-config builds, the executable may be under `build/Debug/battle_sim.exe` or `build/Release/battle_sim.exe`:

```bash
python tools/local-runner/server.py --exe build/Debug/battle_sim.exe
```

Then open:

```text
http://127.0.0.1:8765
```

Open the viewer through the helper URL for local runs. Direct `file://` opening is still supported for manual `trace.json` loading only.

## Executable Discovery

The helper checks, in order:

- `--exe <path>`
- `BATTLE_SIM_EXE`
- common build outputs such as `build/battle_sim`, `build/battle_sim.exe`, `build/Debug/battle_sim.exe`, `build/Release/battle_sim.exe`, and `out/build/*/battle_sim.exe`

If no executable is found, the browser shows a clear error and the run artifacts include `stderr.txt`.

## Run Artifacts

Each browser run writes:

```text
tools/local-runner/runs/<run-id>/
  run-options.json
  scenario.commands
  trace.json
  stdout.txt
  stderr.txt
  exit-code.txt
```

`tools/local-runner/runs/` is ignored by Git.

## API

- `GET /api/health`
- `GET /api/executable`
- `POST /api/run`
- `GET /api/runs`
- `GET /api/runs/latest`
- `GET /api/runs/<run-id>/trace`
- `GET /api/runs/<run-id>/stdout`
- `GET /api/runs/<run-id>/stderr`

## Safety

The helper is intentionally local and narrow:

- binds only to `127.0.0.1`
- does not use `shell=True`
- validates map sizes and unit counts
- does not accept arbitrary output paths from the browser
- writes run artifacts only under the configured runs directory
- exposes stdout and stderr for debugging failed runs
