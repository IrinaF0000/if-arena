import assert from "node:assert/strict";
import { mkdir, readFile, writeFile } from "node:fs/promises";
import { fileURLToPath, pathToFileURL } from "node:url";
import path from "node:path";
import ts from "../../frontend/telegram_mini_app/node_modules/typescript/lib/typescript.js";

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "../..");
const sourceRoot = path.join(root, "frontend/telegram_mini_app/src");
const outDir = path.join(root, "build/frontend-tests");

async function transpile(sourcePath, outPath, replacements = []) {
  let source = await readFile(sourcePath, "utf8");
  for (const [from, to] of replacements) {
    source = source.replaceAll(from, to);
  }
  const transpiled = ts.transpileModule(source, {
    compilerOptions: {
      module: ts.ModuleKind.ES2022,
      target: ts.ScriptTarget.ES2022,
      strict: true
    }
  });
  await mkdir(path.dirname(outPath), { recursive: true });
  await writeFile(outPath, transpiled.outputText, "utf8");
}

await transpile(
  path.join(sourceRoot, "protocol/ProtocolTypes.ts"),
  path.join(outDir, "protocol/ProtocolTypes.mjs")
);
await transpile(
  path.join(sourceRoot, "network/WebSocketClient.ts"),
  path.join(outDir, "network/WebSocketClient.mjs"),
  [["../protocol/ProtocolTypes", "../protocol/ProtocolTypes.mjs"]]
);

class FakeWebSocket {
  static CONNECTING = 0;
  static OPEN = 1;
  static CLOSED = 3;
  static instances = [];

  readyState = FakeWebSocket.CONNECTING;
  sent = [];
  listeners = new Map();

  constructor(url) {
    this.url = url;
    FakeWebSocket.instances.push(this);
  }

  addEventListener(type, listener) {
    const listeners = this.listeners.get(type) ?? [];
    listeners.push(listener);
    this.listeners.set(type, listeners);
  }

  send(payload) {
    this.sent.push(JSON.parse(payload));
  }

  open() {
    this.readyState = FakeWebSocket.OPEN;
    this.dispatch("open", {});
  }

  message(value) {
    this.dispatch("message", { data: JSON.stringify(value) });
  }

  close(code = 1000, reason = "test-close") {
    this.readyState = FakeWebSocket.CLOSED;
    this.dispatch("close", { code, reason });
  }

  dispatch(type, event) {
    for (const listener of this.listeners.get(type) ?? []) {
      listener(event);
    }
  }
}

globalThis.WebSocket = FakeWebSocket;

const { WebSocketClient } = await import(pathToFileURL(path.join(outDir, "network/WebSocketClient.mjs")).href);

const states = [];
const messages = [];
const diagnostics = [];
const client = new WebSocketClient({
  url: "ws://127.0.0.1:8081/ws",
  displayName: "browser-one",
  onStateChanged: (state) => states.push(state),
  onMessage: (message) => messages.push(message),
  onDiagnostic: (message) => diagnostics.push(message)
});

client.connect();
client.sendAuthRequest("");
const socket = FakeWebSocket.instances[0];
socket.open();

assert.equal(socket.sent.length, 1);
assert.equal(socket.sent[0].version, 1);
assert.equal(socket.sent[0].type, "auth_request");

client.createMatch();
assert.equal(socket.sent[1].version, 1);
assert.equal(socket.sent[1].type, "create_match");

socket.message({ version: 1, type: "auth_result", payload: { accepted: true, sessionId: "2" } });
assert.equal(states.at(-1), "authenticated");

const sentBeforeJoin = socket.sent.length;
socket.message({ version: 1, type: "match_joined", payload: { matchId: "1", matchCode: "M1", team: "red" } });
assert.equal(states.at(-1), "in_match");
assert.equal(socket.sent.length, sentBeforeJoin, "match_joined must not trigger automatic input/no-op commands");

client.sendCommand("move", { x: 0, y: -1 });
assert.equal(socket.sent.at(-1).version, 1);
assert.equal(socket.sent.at(-1).type, "input_command");
assert.deepEqual(socket.sent.at(-1).payload.command, { kind: "move", direction: { x: 0, y: -1 } });

socket.message({
  version: 1,
  type: "snapshot",
  payload: {
    matchId: "1",
    tick: 2,
    serverTick: 2,
    finished: false,
    map: { width: 21, height: 13 },
    players: [
      {
        playerId: "2",
        heroId: "2",
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
      state: "at_spawn",
      x: 10,
      y: 6,
      carrierPlayerId: "0",
      pickupLockTicks: 0,
      respawnTicks: 0
    },
    scores: [{ team: "red", score: 0 }],
    hazards: []
  }
});

client.sendCommand("move", { x: 0, y: -1 });
assert.deepEqual(socket.sent.at(-1).payload.command, { kind: "move", direction: { x: 0, y: -1 } });

for (const [kind, direction] of [
  ["stop", undefined],
  ["attack", { x: 1, y: 0 }],
  ["dash", { x: -1, y: 0 }]
]) {
  client.sendCommand(kind, direction);
  assert.equal(socket.sent.at(-1).version, 1);
  assert.equal(socket.sent.at(-1).type, "input_command");
  assert.equal(socket.sent.at(-1).payload.command.kind, kind);
}

socket.message({ version: 1, type: "ping", payload: {} });
assert.equal(socket.sent.at(-1).version, 1);
assert.equal(socket.sent.at(-1).type, "pong");

assert.equal(socket.sent.some((message) => message.type === "input_command" && message.payload.command.kind === "none"), false);

socket.close(4000, "idle timeout");
assert.equal(states.at(-1), "closed");
assert.equal(diagnostics.at(-1), "disconnected, rejoin not supported yet (idle timeout)");
const sentAfterClose = socket.sent.length;
client.sendCommand("move", { x: 1, y: 0 });
assert.equal(socket.sent.length, sentAfterClose, "closed socket must not send stale match commands");

client.connect();
const reconnect = FakeWebSocket.instances[1];
reconnect.open();
client.sendAuthRequest("");
assert.equal(reconnect.sent.at(-1).type, "auth_request");
reconnect.message({ version: 1, type: "auth_result", payload: { accepted: true, sessionId: "3" } });
const sentBeforeFreshJoin = reconnect.sent.length;
client.sendCommand("move", { x: 1, y: 0 });
assert.equal(reconnect.sent.length, sentBeforeFreshJoin, "reconnect must not resume the old match before a fresh join");

console.log("[PASS] telegram_websocket_client_behavior");
