import { ArenaCanvas } from "./game/ArenaCanvas";
import { WebSocketClient } from "./network/WebSocketClient";
import type { ConnectionState } from "./network/WebSocketClient";
import type { IncomingMessage } from "./protocol/ProtocolTypes";
import { createTelegramBridge } from "./telegram/TelegramBridge";
import { TouchControls } from "./ui/TouchControls";
import "./style.css";

const root = document.querySelector<HTMLDivElement>("#app");
if (!root) {
  throw new Error("Missing #app root");
}

root.innerHTML = `
  <main class="app-shell">
    <header class="topbar">
      <div>
        <h1>IF Arena</h1>
        <p id="match-line">No match</p>
      </div>
      <p id="connection-state" class="status-pill">disconnected</p>
    </header>
    <section class="play-surface">
      <canvas id="arena" width="960" height="540"></canvas>
    </section>
    <section class="toolbar">
      <button id="connect">Connect</button>
      <button id="create-match">Create</button>
      <label class="join-field">
        <span>Code</span>
        <input id="join-code" autocomplete="off" maxlength="32" placeholder="M1" />
      </label>
      <button id="join-match">Join</button>
    </section>
    <section class="controls" aria-label="Player controls">
      <button id="move-up">↑</button>
      <button id="move-left">←</button>
      <button id="move-down">↓</button>
      <button id="move-right">→</button>
      <button id="attack">Attack</button>
      <button id="dash">Dash</button>
      <button id="stop">Stop</button>
    </section>
  </main>
`;

const bridge = createTelegramBridge();
bridge.ready();

const canvas = requireElement<HTMLCanvasElement>("#arena");
const stateLabel = requireElement<HTMLParagraphElement>("#connection-state");
const matchLine = requireElement<HTMLParagraphElement>("#match-line");
const joinCodeInput = requireElement<HTMLInputElement>("#join-code");
const arena = new ArenaCanvas(canvas);
let localPlayerId: string | null = null;

const wsUrl = configuredWsUrl();
const client = new WebSocketClient({
  url: wsUrl,
  displayName: "Telegram Demo Player",
  onStateChanged: (state) => updateConnectionState(state),
  onMessage: (message) => handleMessage(message),
  onDiagnostic: (message) => {
    stateLabel.textContent = message;
    arena.setStatus(message);
  }
});

const controls = new TouchControls({
  sendCommand: (kind, direction) => client.sendCommand(kind, direction)
});
controls.bindKeyboard(window);
controls.bindButton(document.querySelector<HTMLButtonElement>("#move-up"), "move", { x: 0, y: -1 });
controls.bindButton(document.querySelector<HTMLButtonElement>("#move-left"), "move", { x: -1, y: 0 });
controls.bindButton(document.querySelector<HTMLButtonElement>("#move-down"), "move", { x: 0, y: 1 });
controls.bindButton(document.querySelector<HTMLButtonElement>("#move-right"), "move", { x: 1, y: 0 });
controls.bindButton(document.querySelector<HTMLButtonElement>("#attack"), "attack");
controls.bindButton(document.querySelector<HTMLButtonElement>("#dash"), "dash");
controls.bindButton(document.querySelector<HTMLButtonElement>("#stop"), "stop");

document.querySelector<HTMLButtonElement>("#connect")?.addEventListener("click", () => {
  client.connect();
  client.sendAuthRequest(bridge.getRawInitData());
});

document.querySelector<HTMLButtonElement>("#create-match")?.addEventListener("click", () => {
  client.createMatch();
});

document.querySelector<HTMLButtonElement>("#join-match")?.addEventListener("click", () => {
  client.joinMatch(joinCodeInput.value);
});

updateConnectionState("disconnected");
arena.render();

function handleMessage(message: IncomingMessage): void {
  switch (message.type) {
    case "auth_result": {
      const accepted = message.payload.accepted === true || message.payload.ok === true;
      localPlayerId = message.payload.sessionId ?? message.payload.playerId ?? localPlayerId;
      stateLabel.textContent = accepted ? "authenticated" : "auth rejected";
      arena.setStatus(stateLabel.textContent);
      break;
    }
    case "match_joined":
      matchLine.textContent = `Match ${message.payload.matchId} | Code ${message.payload.matchCode}`;
      joinCodeInput.value = message.payload.matchCode;
      break;
    case "snapshot":
      arena.setSnapshot(message.payload, localPlayerId);
      break;
    case "input_ack":
      if (!message.payload.accepted) {
        stateLabel.textContent = `input ${message.payload.reason ?? "rejected"}`;
        arena.setStatus(stateLabel.textContent);
      }
      break;
    case "error":
      stateLabel.textContent = `error: ${message.payload.code}`;
      arena.setStatus(stateLabel.textContent);
      break;
    case "client_parse_error":
      stateLabel.textContent = `protocol: ${message.payload.reason}`;
      arena.setStatus(stateLabel.textContent);
      break;
    case "event_batch":
      updateEventStatus(message.payload.events);
      break;
    case "ping":
      break;
  }
}

function updateConnectionState(state: ConnectionState): void {
  stateLabel.textContent = state;
  arena.setStatus(state);
}

function updateEventStatus(events: unknown[] | undefined): void {
  const label = readableEvent(events);
  if (!label) {
    return;
  }
  stateLabel.textContent = label;
  arena.setStatus(label);
}

function readableEvent(events: unknown[] | undefined): string | null {
  if (!events) {
    return null;
  }
  for (const event of events) {
    if (!isEventRecord(event)) {
      continue;
    }
    switch (event.type) {
      case "objective_picked_up":
        return "objective picked up";
      case "objective_dropped":
        return "objective dropped";
      case "objective_captured":
        return "objective captured";
      case "attack_hit":
        return "attack hit";
      case "player_dashed":
        return "dash";
      case "hazard_hit":
        return "hazard hit";
      default:
        break;
    }
  }
  return null;
}

function isEventRecord(value: unknown): value is { type: string } {
  return typeof value === "object" && value !== null && "type" in value && typeof value.type === "string";
}

function requireElement<T extends Element>(selector: string): T {
  const element = document.querySelector<T>(selector);
  if (!element) {
    throw new Error(`Missing element ${selector}`);
  }
  return element;
}

function configuredWsUrl(): string {
  const meta = import.meta as ImportMeta & { env?: { VITE_WS_URL?: string } };
  return meta.env?.VITE_WS_URL ?? "ws://127.0.0.1:8081/ws";
}
