import assert from "node:assert/strict";
import { mkdir, readFile, writeFile } from "node:fs/promises";
import { fileURLToPath, pathToFileURL } from "node:url";
import path from "node:path";
import ts from "../../frontend/telegram_mini_app/node_modules/typescript/lib/typescript.js";

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "../..");
const sourcePath = path.join(root, "frontend/telegram_mini_app/src/ui/TouchControls.ts");
const outDir = path.join(root, "build/frontend-tests/ui");
const outPath = path.join(outDir, "TouchControls.mjs");

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

const { TouchControls } = await import(pathToFileURL(outPath).href);

class FakeTarget {
  listeners = new Map();

  addEventListener(type, listener) {
    const listeners = this.listeners.get(type) ?? [];
    listeners.push(listener);
    this.listeners.set(type, listeners);
  }

  dispatch(type, event) {
    for (const listener of this.listeners.get(type) ?? []) {
      listener(event);
    }
  }
}

class FakeButton {
  listener = null;

  addEventListener(type, listener) {
    assert.equal(type, "click");
    this.listener = listener;
  }

  click() {
    assert.ok(this.listener);
    this.listener();
  }
}

function keyEvent(code) {
  return {
    code,
    repeat: false,
    defaultPrevented: false,
    preventDefault() {
      this.defaultPrevented = true;
    }
  };
}

const sent = [];
const controls = new TouchControls({
  sendCommand: (kind, direction) => sent.push({ kind, direction })
});
const keyboard = new FakeTarget();
controls.bindKeyboard(keyboard);

const space = keyEvent("Space");
keyboard.dispatch("keydown", space);
assert.equal(space.defaultPrevented, true);
assert.deepEqual(sent.at(-1), { kind: "attack", direction: { x: 0, y: -1 } });

const up = keyEvent("ArrowUp");
keyboard.dispatch("keydown", up);
assert.deepEqual(sent.at(-1), { kind: "move", direction: { x: 0, y: -1 } });

keyboard.dispatch("keyup", keyEvent("ArrowUp"));
assert.deepEqual(sent.at(-1), { kind: "stop", direction: undefined });

keyboard.dispatch("keydown", keyEvent("Space"));
assert.deepEqual(sent.at(-1), { kind: "attack", direction: { x: 0, y: -1 } });

const rightButton = new FakeButton();
const attackButton = new FakeButton();
const dashButton = new FakeButton();
const stopButton = new FakeButton();
controls.bindButton(rightButton, "move", { x: 1, y: 0 });
controls.bindButton(attackButton, "attack");
controls.bindButton(dashButton, "dash");
controls.bindButton(stopButton, "stop");

rightButton.click();
assert.deepEqual(sent.at(-1), { kind: "move", direction: { x: 1, y: 0 } });
attackButton.click();
assert.deepEqual(sent.at(-1), { kind: "attack", direction: { x: 1, y: 0 } });
dashButton.click();
assert.deepEqual(sent.at(-1), { kind: "dash", direction: { x: 1, y: 0 } });
stopButton.click();
assert.deepEqual(sent.at(-1), { kind: "stop", direction: undefined });

assert.equal(sent.some((message) => message.kind === "interact"), false);
assert.equal(sent.some((message) => message.kind === "close" || message.kind === "disconnect"), false);

console.log("[PASS] telegram_touch_controls_behavior");
