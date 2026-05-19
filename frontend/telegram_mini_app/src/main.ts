import { createTelegramBridge } from "./telegram/TelegramBridge";
import { ArenaCanvas } from "./game/ArenaCanvas";
import { WebSocketClient } from "./network/WebSocketClient";

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
const client = new WebSocketClient({
  url: "ws://127.0.0.1:8081/ws",
  onStateChanged: (state) => {
    if (stateLabel) {
      stateLabel.textContent = state;
    }
  },
  onMessage: (message) => {
    console.log("message", message);
  }
});

document.querySelector<HTMLButtonElement>("#connect")?.addEventListener("click", () => {
  client.connect();
  const initData = bridge.getRawInitData();
  client.sendAuthRequest(initData);
});
