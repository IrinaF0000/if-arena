import { defineConfig, devices } from "@playwright/test";

const e2ePort = Number(process.env.IF_ARENA_E2E_WEB_PORT ?? "5174");
const wsUrl = process.env.VITE_WS_URL ?? "ws://127.0.0.1:8081/ws";

export default defineConfig({
  testDir: "./tests/e2e",
  timeout: 60_000,
  expect: {
    timeout: 7_500
  },
  fullyParallel: false,
  reporter: process.env.CI ? [["list"], ["html", { open: "never" }]] : "list",
  use: {
    baseURL: `http://127.0.0.1:${e2ePort}`,
    trace: "retain-on-failure"
  },
  webServer: {
    command: `npm run dev -- --host 127.0.0.1 --port ${e2ePort}`,
    url: `http://127.0.0.1:${e2ePort}`,
    reuseExistingServer: false,
    env: {
      VITE_WS_URL: wsUrl
    },
    timeout: 30_000
  },
  projects: [
    {
      name: "chromium",
      use: { ...devices["Desktop Chrome"] }
    }
  ]
});
