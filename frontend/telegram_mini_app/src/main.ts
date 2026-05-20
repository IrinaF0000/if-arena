import { createTelegramBridge } from "./telegram/TelegramBridge";
import { ArenaCanvas } from "./game/ArenaCanvas";
import { WebSocketClient } from "./network/WebSocketClient";
import { TouchControls } from "./ui/TouchControls";

const root = document.querySelector<HTMLDivElement>("#app");
if (!root) {
  throw new Error("Missing #app root");
}

root.innerHTML = `
  <main class="app-shell">
    <header>
      <h1>IF Arena</h1>
      <p id="connection-state">Disconnected</p>
    </header>
    <canvas id="arena" width="960" height="540"></canvas>
    <section class="controls">
      <button id="connect">Connect</button>
      <button id="attack">Attack</button>
      <button id="dash">Dash</button>
      <button id="interact">Interact</button>
    </section>
  </main>
`;

const bridge = createTelegramBridge();
bridge.ready();

const canvas = document.querySelector<HTMLCanvasElement>("#arena");
if (!canvas) {
  throw new Error("Missing arena canvas");
}

const arena = new ArenaCanvas(canvas);
arena.renderPlaceholder();

const stateLabel = document.querySelector<HTMLParagraphElement>("#connection-state");
const touchControls = new TouchControls();
const client = new WebSocketClient({
  url: "ws://127.0.0.1:8081/ws",
  onStateChanged: (state) => {
    if (stateLabel) {
      stateLabel.textContent = state;
    }
  },
  onMessage: (message) => {
    if (stateLabel && message.type === "client_parse_error") {
      stateLabel.textContent = `Protocol error: ${message.payload.reason}`;
    }
  }
});

document.querySelector<HTMLButtonElement>("#connect")?.addEventListener("click", () => {
  client.connect();
  const initData = bridge.getRawInitData();
  client.sendAuthRequest(initData);
});

document.querySelector<HTMLButtonElement>("#attack")?.addEventListener("click", () => {
  touchControls.recordAction("attack");
});

document.querySelector<HTMLButtonElement>("#dash")?.addEventListener("click", () => {
  touchControls.recordAction("dash");
});

document.querySelector<HTMLButtonElement>("#interact")?.addEventListener("click", () => {
  touchControls.recordAction("interact");
});
