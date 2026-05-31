import { expect, test, type Page } from "@playwright/test";
import { spawn, type ChildProcessWithoutNullStreams } from "node:child_process";
import { randomBytes } from "node:crypto";
import { mkdir, writeFile } from "node:fs/promises";
import net from "node:net";
import os from "node:os";
import path from "node:path";
import { fileURLToPath } from "node:url";

type CapturedEnvelope = {
  type?: string;
  payload?: {
    code?: string;
    message?: string;
  };
};

type SocketCapture = {
  received: CapturedEnvelope[];
  sent: CapturedEnvelope[];
  urls: string[];
  closes: number;
};

const repoRoot = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "../../../..");
const wsPort = Number(process.env.IF_ARENA_E2E_WS_PORT ?? "8081");
const serverExecutable = path.join(repoRoot, "build", process.platform === "win32" ? "battle_server_app.exe" : "battle_server_app");

let server: ChildProcessWithoutNullStreams | undefined;
let serverStdout = "";
let serverStderr = "";

test.describe.configure({ mode: "serial" });

test.beforeAll(async () => {
  const configPath = path.join(os.tmpdir(), `if-arena-e2e-ws-${wsPort}.json`);
  await writeConfig(configPath, wsPort);
  server = spawn(serverExecutable, ["--config", configPath, "--max-clients", "16"], {
    cwd: repoRoot,
    stdio: ["ignore", "pipe", "pipe"]
  });
  await waitForServerReady(wsPort, server);
});

test.afterAll(async () => {
  if (server && server.exitCode === null) {
    server.kill();
    await new Promise((resolve) => server?.once("exit", resolve));
  }
});

test("two browser clients play through websocket without passive disconnect", async ({ browser }) => {
  const context = await browser.newContext();
  const pageA = await context.newPage();
  const pageB = await context.newPage();
  const captureA = captureWebSocket(pageA);
  const captureB = captureWebSocket(pageB);
  const errors: string[] = [];
  collectPageErrors(pageA, "A", errors);
  collectPageErrors(pageB, "B", errors);

  await pageA.goto("/");
  await pageB.goto("/");

  await expect(pageA.locator("#create-match")).toBeDisabled();
  await expect(pageA.locator("#join-match")).toBeDisabled();
  await expect(pageA.locator("#move-up")).toBeDisabled();
  await pageA.locator("#connect").click();
  await expectAuthenticated(pageA, captureA, errors, "A");
  await expect(pageA.locator("#create-match")).toBeEnabled();
  await pageA.locator("#create-match").click();
  await expect(pageA.locator("#match-line")).toContainText("Code");
  await expect(pageA.locator("#move-up")).toBeEnabled();
  const matchCode = await pageA.locator("#join-code").inputValue();
  expect(matchCode.length).toBeGreaterThan(0);

  await expect(pageB.locator("#join-match")).toBeDisabled();
  await pageB.locator("#connect").click();
  await expectAuthenticated(pageB, captureB, errors, "B");
  await expect(pageB.locator("#join-match")).toBeEnabled();
  await pageB.locator("#join-code").fill(matchCode);
  await pageB.locator("#join-match").click();
  await expect(pageB.locator("#match-line")).toContainText(`Code ${matchCode}`);

  await expect.poll(() => countSnapshots(captureA.received)).toBeGreaterThan(0);
  await expect.poll(() => countSnapshots(captureB.received)).toBeGreaterThan(0);

  await pageA.locator("#move-up").click();
  await pageA.waitForTimeout(2_600);
  await pageA.locator("#stop").click();

  await expect.poll(() => countSnapshots(captureA.received)).toBeGreaterThan(1);
  await expect.poll(() => countSnapshots(captureB.received)).toBeGreaterThan(1);

  const allReceived = [...captureA.received, ...captureB.received];
  expect(allReceived.some((message) => isErrorCode(message, "invalid_envelope"))).toBe(false);
  expect(allReceived.some((message) => isErrorCode(message, "idle_timeout"))).toBe(false);
  expect(errors).toEqual([]);

  await context.close();
});

function captureWebSocket(page: Page): SocketCapture {
  const capture: SocketCapture = { received: [], sent: [], urls: [], closes: 0 };
  page.on("websocket", (socket) => {
    const url = socket.url();
    const capturesGameProtocol = url.endsWith("/ws");
    capture.urls.push(url);
    socket.on("framereceived", (frame) => {
      if (!capturesGameProtocol) {
        return;
      }
      const parsed = parseEnvelope(frame.payload);
      if (parsed) {
        capture.received.push(parsed);
      }
    });
    socket.on("framesent", (frame) => {
      if (!capturesGameProtocol) {
        return;
      }
      const parsed = parseEnvelope(frame.payload);
      if (parsed) {
        capture.sent.push(parsed);
      }
    });
    socket.on("close", () => {
      capture.closes += 1;
    });
  });
  return capture;
}

async function expectAuthenticated(page: Page, capture: SocketCapture, errors: string[], label: string): Promise<void> {
  try {
    await expect(page.locator("#connection-state")).toHaveText("authenticated");
  } catch (error) {
    const state = await page.locator("#connection-state").textContent();
    throw new Error(
      `${label} did not authenticate; state=${state}; diagnostics=${JSON.stringify({
        capture,
        errors,
        serverStdout,
        serverStderr
      })}; ${error instanceof Error ? error.message : String(error)}`
    );
  }
}

function collectPageErrors(page: Page, label: string, errors: string[]): void {
  page.on("console", (message) => {
    if (message.type() === "error") {
      errors.push(`${label} console: ${message.text()}`);
    }
  });
  page.on("pageerror", (error) => {
    errors.push(`${label} pageerror: ${error.message}`);
  });
}

function parseEnvelope(payload: string | Buffer): CapturedEnvelope | null {
  const text = Buffer.isBuffer(payload) ? payload.toString("utf8") : payload;
  try {
    const value = JSON.parse(text) as unknown;
    if (typeof value === "object" && value !== null) {
      return value as CapturedEnvelope;
    }
  } catch {
    return null;
  }
  return null;
}

function countSnapshots(messages: CapturedEnvelope[]): number {
  return messages.filter((message) => message.type === "snapshot").length;
}

function isErrorCode(message: CapturedEnvelope, code: string): boolean {
  return message.type === "error" && message.payload?.code === code;
}

async function waitForServerReady(port: number, processHandle: ChildProcessWithoutNullStreams): Promise<void> {
  const deadline = Date.now() + 5_000;
  serverStdout = "";
  serverStderr = "";
  processHandle.stdout.on("data", (chunk) => {
    serverStdout += String(chunk);
  });
  processHandle.stderr.on("data", (chunk) => {
    serverStderr += String(chunk);
  });

  while (Date.now() < deadline) {
    if (processHandle.exitCode !== null) {
      throw new Error(`server exited before listening: ${processHandle.exitCode} ${serverStdout} ${serverStderr}`);
    }
    if (await canUpgradeWebSocket(port)) {
      return;
    }
    await new Promise((resolve) => setTimeout(resolve, 50));
  }
  throw new Error(`server did not accept WebSocket upgrade: ${serverStdout} ${serverStderr}`);
}

async function canUpgradeWebSocket(port: number): Promise<boolean> {
  return await new Promise((resolve) => {
    const socket = net.createConnection({ host: "127.0.0.1", port });
    let response = "";
    let settled = false;
    const finish = (ready: boolean) => {
      if (settled) {
        return;
      }
      settled = true;
      socket.destroy();
      resolve(ready);
    };

    socket.setTimeout(750);
    socket.once("connect", () => {
      const key = randomBytes(16).toString("base64");
      socket.write(
        [
          "GET /ws HTTP/1.1",
          `Host: 127.0.0.1:${port}`,
          "Upgrade: websocket",
          "Connection: Upgrade",
          `Sec-WebSocket-Key: ${key}`,
          "Sec-WebSocket-Version: 13",
          "",
          ""
        ].join("\r\n")
      );
    });
    socket.on("data", (chunk) => {
      response += String(chunk);
      if (response.includes("\r\n\r\n")) {
        finish(response.includes("101 Switching Protocols"));
      }
    });
    socket.once("timeout", () => finish(false));
    socket.once("error", () => finish(false));
    socket.once("close", () => finish(response.includes("101 Switching Protocols")));
  });
}

async function writeConfig(configPath: string, port: number): Promise<void> {
  await mkdir(path.dirname(configPath), { recursive: true });
  await writeFile(
    configPath,
    JSON.stringify({
      mode: "local",
      server: { tickRate: 20, snapshotRate: 10, maxConnections: 16, maxMatches: 8 },
      transports: {
        tcp: { enabled: false, host: "127.0.0.1", port: 5555, maxFrameBytes: 65536 },
        websocket: {
          enabled: true,
          host: "127.0.0.1",
          port,
          path: "/ws",
          maxMessageBytes: 65536,
          requireTls: false
        }
      },
      security: {
        demoAuthEnabled: true,
        telegramAuthEnabled: false,
        handshakeTimeoutMs: 1000,
        idleTimeoutMs: 1000,
        maxInputCommandsPerSecond: 30,
        maxPendingWriteBytesPerSession: 1048576,
        maxPendingOutboundMessages: 64,
        maxPendingCommandsPerMatch: 128,
        maxCommandsPerTick: 64,
        maxPendingCommandsPerSession: 64
      },
      metrics: { logEverySeconds: 5 }
    }),
    "utf8"
  );
}
