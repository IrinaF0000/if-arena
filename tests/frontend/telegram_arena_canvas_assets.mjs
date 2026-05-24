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

const { ArenaCanvas, playerSpritePath } = await import(pathToFileURL(outPath).href);

assert.equal(playerSpritePath, "/players/swordsman.svg");

const canvas = new FakeCanvas();
const arena = new ArenaCanvas(canvas);
assert.equal(arena.playerSprite.src, "/players/swordsman.svg");

arena.setAimDirection({ x: 1, y: 0 });
arena.setSnapshot(
  {
    matchId: "1",
    tick: 4,
    serverTick: 4,
    finished: false,
    map: { width: 21, height: 13 },
    players: [
      {
        playerId: "local",
        heroId: "swordsman",
        team: "blue",
        x: 10,
        y: 10,
        hp: 100,
        attackCooldown: 0,
        dashCooldown: 0,
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
      { team: "blue", score: 0 },
      { team: "red", score: 0 }
    ],
    hazards: []
  },
  "local"
);

const drawImages = canvas.context.calls.filter((call) => call.name === "drawImage");
assert.equal(drawImages.length, 2, "both players should render through the SVG image");
assert.equal(drawImages[0].args[0].src, "/players/swordsman.svg");

const rotations = canvas.context.calls.filter((call) => call.name === "rotate").map((call) => call.args[0]);
assert.ok(rotations.some((angle) => Math.abs(angle - Math.PI / 2) < 0.0001), "local sprite rotates by aim direction");

const carrierArcs = canvas.context.calls.filter((call) => call.name === "arc" && call.args[2] > 14);
assert.ok(carrierArcs.length > 0, "carrier overlay remains visible");

console.log("[PASS] telegram_arena_canvas_assets");
