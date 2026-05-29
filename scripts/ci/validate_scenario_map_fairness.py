#!/usr/bin/env python3
"""Validate config-authored playable maps for fairness and pathability."""

from __future__ import annotations

import json
from collections import deque
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Iterable


ROOT = Path(__file__).resolve().parents[2]
SCENARIO_DIR = ROOT / "config" / "scenarios"


Cell = tuple[int, int]

MIN_ROUTE_LENGTHS = {
    "blue spawn to objective": 6,
    "red spawn to objective": 6,
    "objective to blue base": 7,
    "objective to red base": 7,
}


@dataclass(frozen=True)
class ValidationResult:
    path: Path
    errors: tuple[str, ...]


def point(value: dict[str, Any], name: str) -> Cell:
    item = value[name]
    return int(item["x"]), int(item["y"])


def dimensions(scenario: dict[str, Any]) -> tuple[int, int]:
    map_config = scenario["map"]
    return int(map_config["width"]), int(map_config["height"])


def rotate180(cell: Cell, width: int, height: int) -> Cell:
    x, y = cell
    return width - 1 - x, height - 1 - y


def cell_set(items: Iterable[dict[str, Any]]) -> set[Cell]:
    return {(int(item["x"]), int(item["y"])) for item in items}


def in_bounds(cell: Cell, width: int, height: int) -> bool:
    x, y = cell
    return 0 <= x < width and 0 <= y < height


def neighbors(cell: Cell) -> Iterable[Cell]:
    x, y = cell
    yield x + 1, y
    yield x - 1, y
    yield x, y + 1
    yield x, y - 1


def shortest_path(width: int, height: int, blocked: set[Cell], start: Cell, goal: Cell) -> list[Cell] | None:
    if start in blocked or goal in blocked or not in_bounds(start, width, height) or not in_bounds(goal, width, height):
        return None
    frontier: deque[Cell] = deque([start])
    previous: dict[Cell, Cell | None] = {start: None}
    while frontier:
        current = frontier.popleft()
        if current == goal:
            path: list[Cell] = []
            cursor: Cell | None = goal
            while cursor is not None:
                path.append(cursor)
                cursor = previous[cursor]
            path.reverse()
            return path
        for next_cell in neighbors(current):
            if (
                next_cell not in previous
                and in_bounds(next_cell, width, height)
                and next_cell not in blocked
            ):
                previous[next_cell] = current
                frontier.append(next_cell)
    return None


def has_required_choke(width: int, height: int, blocked: set[Cell], start: Cell, goal: Cell) -> bool:
    for y in range(height):
        for x in range(width):
            candidate = (x, y)
            if candidate in blocked or candidate == start or candidate == goal:
                continue
            if shortest_path(width, height, blocked | {candidate}, start, goal) is None:
                return True
    return False


def hazard_key(hazard: dict[str, Any]) -> tuple[Any, ...]:
    return (
        hazard.get("kind"),
        float(hazard.get("radius", 0)),
        float(hazard.get("range", 0)),
        int(hazard.get("damage", 0)),
        int(hazard.get("cooldownTicks", 0)),
        int(hazard.get("seed", 0)),
        hazard.get("effect"),
        hazard.get("trigger"),
        hazard.get("icon"),
    )


def validate_scenario(path: Path) -> ValidationResult:
    scenario = json.loads(path.read_text(encoding="utf-8"))
    errors: list[str] = []
    map_config = scenario.get("map", {})
    players = scenario.get("players", {})
    width, height = dimensions(scenario)
    center = point(map_config, "center")
    objective = point(map_config, "objectiveSpawn")
    blue_spawn = point(players, "blueSpawn")
    red_spawn = point(players, "redSpawn")
    blue_base = point(map_config, "blueBase")
    red_base = point(map_config, "redBase")
    obstacles = cell_set(map_config.get("obstacles", []))
    hazards = list(scenario.get("hazards", []))

    if (width, height) != (21, 13):
        errors.append("playable MVP map must stay 21x13")
    if map_config.get("symmetry") != "rotational_180":
        errors.append("map.symmetry must be rotational_180")
    if rotate180(center, width, height) != center:
        errors.append("map.center must be the rotational center cell")
    if objective != center or rotate180(objective, width, height) != objective:
        errors.append("objectiveSpawn must be central")
    if rotate180(blue_spawn, width, height) != red_spawn:
        errors.append("player spawns must use rotational symmetry")
    if rotate180(blue_base, width, height) != red_base:
        errors.append("bases must use rotational symmetry")

    for obstacle in obstacles:
        if not in_bounds(obstacle, width, height):
            errors.append(f"obstacle out of bounds: {obstacle}")
        if rotate180(obstacle, width, height) not in obstacles:
            errors.append(f"obstacle missing rotational pair: {obstacle}")

    hazard_by_cell = {(int(hazard["x"]), int(hazard["y"])): hazard for hazard in hazards}
    hazard_cells = set(hazard_by_cell)
    if len(hazard_by_cell) != len(hazards):
        errors.append("hazard cells must be unique")
    for hazard in hazards:
        hazard_cell = int(hazard["x"]), int(hazard["y"])
        mirrored = rotate180(hazard_cell, width, height)
        mirror = hazard_by_cell.get(mirrored)
        if mirror is None or hazard_key(mirror) != hazard_key(hazard):
            errors.append(f"hazard missing equivalent rotational pair: {hazard.get('id', hazard_cell)}")
        if hazard_cell in obstacles:
            errors.append(f"hazard overlaps obstacle: {hazard.get('id', hazard_cell)}")

    route_pairs = {
        "blue spawn to objective": (blue_spawn, objective),
        "red spawn to objective": (red_spawn, objective),
        "objective to blue base": (objective, blue_base),
        "objective to red base": (objective, red_base),
    }
    route_lengths: dict[str, int] = {}
    for label, (start, goal) in route_pairs.items():
        route_blocked = obstacles | (hazard_cells - {start, goal})
        path_cells = shortest_path(width, height, route_blocked, start, goal)
        if path_cells is None:
            errors.append(f"missing legal path: {label}")
            continue
        if any(cell in route_blocked or not in_bounds(cell, width, height) for cell in path_cells):
            errors.append(f"path crosses illegal cell: {label}")
        if has_required_choke(width, height, route_blocked, start, goal):
            errors.append(f"route has a required one-cell choke: {label}")
        route_lengths[label] = len(path_cells) - 1
        if route_lengths[label] < MIN_ROUTE_LENGTHS[label]:
            errors.append(f"route is too direct: {label}={route_lengths[label]}, minimum={MIN_ROUTE_LENGTHS[label]}")

    comparable_pairs = (
        ("blue spawn to objective", "red spawn to objective"),
        ("objective to blue base", "objective to red base"),
    )
    for left, right in comparable_pairs:
        if left in route_lengths and right in route_lengths and abs(route_lengths[left] - route_lengths[right]) > 1:
            errors.append(f"route lengths are not comparable: {left}={route_lengths[left]}, {right}={route_lengths[right]}")

    return ValidationResult(path=path, errors=tuple(errors))


def validate_all() -> list[ValidationResult]:
    results: list[ValidationResult] = []
    for path in sorted(SCENARIO_DIR.glob("*.json")):
        results.append(validate_scenario(path))
    return results


def main() -> int:
    results = validate_all()
    failures = [result for result in results if result.errors]
    if failures:
        print("Scenario map fairness validation failed:")
        for result in failures:
            print(f"- {result.path.relative_to(ROOT)}")
            for error in result.errors:
                print(f"  - {error}")
        return 1
    print("Scenario map fairness validation OK.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
