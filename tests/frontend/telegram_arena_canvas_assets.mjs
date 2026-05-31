import assert from "node:assert/strict";
import { mkdir, readFile, writeFile } from "node:fs/promises";
import { fileURLToPath, pathToFileURL } from "node:url";
import path from "node:path";
import ts from "../../frontend/telegram_mini_app/node_modules/typescript/lib/typescript.js";

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "../..");
const sourcePath = path.join(root, "frontend/telegram_mini_app/src/game/ArenaCanvas.ts");
const outDir = path.join(root, "build/frontend-tests/game");
const outPath = path.join(outDir, "ArenaCanvas.mjs");

const source = await readFile(sourcePath, "utf8");
const transpiled = ts.transpileModule(source, {
  compilerOptions: {
    module: ts.ModuleKind.ES2022,
    target: ts.ScriptTarget.ES2022,
    strict: true
  }
});
await mkdir(outDir, { recursive: true });
await writeFile(outPath, transpiled.outputText, "utf8");

class FakeImage {
  complete = true;
  naturalWidth = 64;
  decoding = "auto";
  src = "";

  addEventListener() {}
}

class FakeContext {
  calls = [];
  fillStyle = "";
  strokeStyle = "";
  lineWidth = 1;
  font = "";
  textAlign = "start";
  globalAlpha = 1;

  record(name, ...args) {
    this.calls.push({ name, args });
  }

  clearRect(...args) { this.record("clearRect", ...args); }
  fillRect(...args) { this.record("fillRect", ...args); }
  strokeRect(...args) { this.record("strokeRect", ...args); }
  beginPath(...args) { this.record("beginPath", ...args); }
  moveTo(...args) { this.record("moveTo", ...args); }
  lineTo(...args) { this.record("lineTo", ...args); }
  closePath(...args) { this.record("closePath", ...args); }
  fill(...args) { this.record("fill", ...args); }
  stroke(...args) { this.record("stroke", ...args); }
  arc(...args) { this.record("arc", ...args); }
  drawImage(...args) { this.record("drawImage", ...args); }
  fillText(...args) { this.record("fillText", ...args); }
  roundRect(...args) { this.record("roundRect", ...args); }
  save(...args) { this.record("save", ...args); }
  restore(...args) { this.record("restore", ...args); }
  translate(...args) { this.record("translate", ...args); }
  rotate(...args) { this.record("rotate", ...args); }
}

class FakeCanvas {
  width = 960;
  height = 540;
  context = new FakeContext();

  getContext(kind) {
    assert.equal(kind, "2d");
    return this.context;
  }
}

globalThis.Image = FakeImage;

const { ArenaCanvas, arenaObjectSpritePaths, playerSpritePath } = await import(pathToFileURL(outPath).href);

assert.equal(playerSpritePath, "/players/swordsman.svg");
assert.equal(arenaObjectSpritePaths.hazard_crow, "/svg/hazard_crow.svg");
assert.equal(arenaObjectSpritePaths.obstacle_block, "/svg/obstacle_block.svg");

const canvas = new FakeCanvas();
const arena = new ArenaCanvas(canvas);
assert.equal(arena.playerSprite.src, "/players/swordsman.svg");

arena.setAimDirection({ x: 1, y: 0 });
arena.showActionFeedback("attack", { x: 1, y: 0 });
arena.setSnapshot(
  {
    matchId: "1",
    tick: 4,
    serverTick: 4,
    finished: true,
    scenario: { id: "arena_small_objective_run", mode: "objective_run", version: 1, source: "server_config" },
    map: { width: 21, height: 13 },
    obstacles: [
      {
        id: "obstacle_7_5",
        kind: "blocking_obstacle",
        visualId: "obstacle_block",
        x: 7,
        y: 5,
        blocksMovement: true,
        damage: 0,
        causesDrop: false,
        rangeRadius: 0,
        cooldownTicks: 0,
        cooldown: 0,
        team: "neutral"
      },
      {
        id: "obstacle_13_7",
        kind: "blocking_obstacle",
        visualId: "obstacle_block",
        x: 13,
        y: 7,
        blocksMovement: true,
        damage: 0,
        causesDrop: false,
        rangeRadius: 0,
        cooldownTicks: 0,
        cooldown: 0,
        team: "neutral"
      }
    ],
    players: [
      {
        playerId: "local",
        heroId: "swordsman",
        team: "blue",
        x: 10,
        y: 10,
        hp: 100,
        attackCooldown: 1,
        dashCooldown: 3,
        inOwnBase: true
      },
      {
        playerId: "enemy",
        heroId: "swordsman",
        team: "red",
        x: 10,
        y: 2,
        hp: 100,
        attackCooldown: 0,
        dashCooldown: 0,
        inOwnBase: true
      }
    ],
    objective: {
      state: "carried",
      x: 10,
      y: 10,
      carrierPlayerId: "local",
      pickupLockTicks: 0,
      respawnTicks: 0
    },
    scores: [
      { team: "blue", score: 1 },
      { team: "red", score: 0 }
    ],
    hazards: [
      {
        id: "center_crow",
        kind: "crow",
        visualId: "hazard_crow",
        x: 10,
        y: 5,
        radius: 0.65,
        range: 1.5,
        damage: 6,
        effect: "damage_drop_objective",
        trigger: "proximity",
        icon: "hazard_crow",
        blocksMovement: false,
        causesDrop: true,
        rangeRadius: 0.65,
        team: "neutral",
        cooldownTicks: 8,
        cooldown: 0,
        triggered: false
      }
    ]
  },
  "local"
);
arena.showEventFeedback([
  { type: "attack_hit", targetPlayerId: "enemy", to: { x: 10, y: 2 } },
  { type: "objective_captured", to: { x: 10, y: 10 } },
  { type: "score_changed", team: "blue", score: 1, to: { x: 10, y: 10 } }
]);

const drawImages = canvas.context.calls.filter((call) => call.name === "drawImage");
assert.ok(drawImages.length >= 5, "players, obstacles, and hazards should render through SVG images");
assert.ok(drawImages.some((call) => call.args[0].src === "/players/swordsman.svg"), "players use the SVG sprite");
assert.ok(drawImages.some((call) => call.args[0].src === "/svg/obstacle_block.svg"), "obstacles use the blocker SVG");
assert.ok(drawImages.some((call) => call.args[0].src === "/svg/hazard_crow.svg"), "hazards use the visualId SVG");

const rotations = canvas.context.calls.filter((call) => call.name === "rotate").map((call) => call.args[0]);
assert.ok(rotations.some((angle) => Math.abs(angle - Math.PI / 2) < 0.0001), "local sprite rotates by aim direction");

const carrierArcs = canvas.context.calls.filter((call) => call.name === "arc" && call.args[2] > 14);
assert.ok(carrierArcs.length > 0, "carrier overlay remains visible");

const fillRects = canvas.context.calls.filter((call) => call.name === "fillRect");
assert.ok(fillRects.length >= 4, "authoritative obstacle cells render as filled blockers");

const rangeArcs = canvas.context.calls.filter((call) => call.name === "arc" && call.args[2] > 70);
assert.ok(rangeArcs.length > 0, "attack range indicator renders around the local player");

const textLabels = canvas.context.calls.filter((call) => call.name === "fillText").map((call) => call.args[0]);
assert.ok(textLabels.includes("hit"), "hit feedback label renders from event batch");
assert.ok(textLabels.some((label) => String(label).includes("captured")), "capture feedback label renders from event batch");
assert.ok(textLabels.some((label) => String(label).includes("Blue scores! 1")), "score feedback label renders from event batch");
assert.ok(textLabels.includes("Match over"), "match-over overlay renders from finished snapshot");
assert.ok(textLabels.some((label) => String(label).includes("Blue wins 1-0")), "winner score overlay renders");
assert.ok(textLabels.includes("6"), "hazard damage marker renders from semantic metadata");
assert.ok(!textLabels.includes("Blue 1 - 0 Red"), "score HUD is not drawn over the active arena");
assert.ok(!textLabels.some((label) => String(label).includes("Tick 4")), "snapshot status HUD is not drawn over the active arena");
assert.ok(!textLabels.some((label) => String(label).startsWith("crow:")), "hazard legend is not drawn over the active arena");

console.log("[PASS] telegram_arena_canvas_assets");
