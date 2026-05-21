import assert from "node:assert/strict";
import { mkdir, readFile, writeFile } from "node:fs/promises";
import { fileURLToPath, pathToFileURL } from "node:url";
import path from "node:path";
import ts from "../../frontend/telegram_mini_app/node_modules/typescript/lib/typescript.js";

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "../..");
const sourcePath = path.join(root, "frontend/telegram_mini_app/src/protocol/ProtocolTypes.ts");
const outDir = path.join(root, "build/frontend-tests");
const outPath = path.join(outDir, "ProtocolTypes.mjs");

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

const protocol = await import(pathToFileURL(outPath).href);

const validSnapshot = protocol.parseIncomingMessage(
  JSON.stringify({
    version: 1,
    type: "snapshot",
    payload: {
      matchId: "1",
      tick: 3,
      serverTick: 3,
      finished: false,
      map: { width: 21, height: 13 },
      players: [
        {
          playerId: "1",
          heroId: "1",
          team: "blue",
          x: 10.5,
          y: 11.5,
          hp: 100,
          attackCooldown: 0,
          dashCooldown: 0,
          inOwnBase: true
        }
      ],
      objective: {
        state: "at_spawn",
        x: 10,
        y: 6,
        carrierPlayerId: "0",
        pickupLockTicks: 0,
        respawnTicks: 0
      },
      scores: [{ team: "blue", score: 0 }],
      hazards: [{ kind: "mine", x: 7, y: 4, cooldown: 0, triggered: false }]
    }
  })
);
assert.equal(validSnapshot.type, "snapshot");

const authorityClaim = protocol.createInputCommand("1", 1, "move", { x: 1, y: 0 });
assert.deepEqual(authorityClaim.payload, {
  matchId: "1",
  command: {
    kind: "move",
    direction: { x: 1, y: 0 }
  }
});
assert.equal("hp" in authorityClaim.payload.command, false);

const malformed = protocol.parseIncomingMessage("{not-json");
assert.equal(malformed.type, "client_parse_error");
assert.equal(malformed.payload.reason, "malformed_json");

const invalidSnapshot = protocol.parseIncomingMessage(
  JSON.stringify({
    version: 1,
    type: "snapshot",
    payload: { matchId: "1", players: "not-an-array" }
  })
);
assert.equal(invalidSnapshot.type, "client_parse_error");
assert.equal(invalidSnapshot.payload.reason, "invalid_envelope");

console.log("[PASS] telegram_protocol_validation");
