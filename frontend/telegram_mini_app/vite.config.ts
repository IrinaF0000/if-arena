import { defineConfig } from "vite";
import path from "node:path";
import { fileURLToPath } from "node:url";

const repoRoot = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "../..");

export default defineConfig({
  publicDir: path.join(repoRoot, "assets"),
  server: {
    host: "127.0.0.1",
    port: 5173
  },
  build: {
    target: "es2022"
  }
});
