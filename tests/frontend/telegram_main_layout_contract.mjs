import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import { fileURLToPath } from "node:url";
import path from "node:path";

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), "../..");
const mainSource = await readFile(path.join(root, "frontend/telegram_mini_app/src/main.ts"), "utf8");
const styleSource = await readFile(path.join(root, "frontend/telegram_mini_app/src/style.css"), "utf8");

function assertContains(source, needle, description) {
  assert.ok(source.includes(needle), description);
}

function assertMatches(source, pattern, description) {
  assert.ok(pattern.test(source), description);
}

assertContains(mainSource, '<p id="match-line">No match</p>', "topbar keeps a compact match/status line");
assertContains(mainSource, '<details class="match-panel" open>', "service controls live in the collapsible match panel");
assertContains(mainSource, "<summary>Match</summary>", "match panel uses a concise Match summary label");
assertMatches(
  mainSource,
  /<details class="match-panel" open>[\s\S]*<section class="toolbar">[\s\S]*id="connect"[\s\S]*id="create-match"[\s\S]*id="join-code"[\s\S]*id="join-match"[\s\S]*id="start-next-match"[\s\S]*<\/details>/,
  "connection, join, create, and next-match controls stay grouped in the match panel"
);
assertContains(
  mainSource,
  'matchLine.textContent = `${scoreLine()} | ${message.payload.objective.state} | ${message.payload.scenario.id}`;',
  "live snapshots display score, objective state, and authoritative scenario id"
);
assertContains(
  mainSource,
  'matchLine.textContent = `${winnerLine()} | ${scoreLine()} | ${message.payload.scenario.id}`;',
  "finished snapshots display winner, score, and authoritative scenario id"
);
assertContains(mainSource, 'return `Blue ${lastScores.blue} - ${lastScores.red} Red`;', "score line remains explicit");
assertContains(mainSource, 'return lastScores.blue > lastScores.red ? "Blue wins" : "Red wins";', "winner line remains derived from snapshot scores");
assertContains(mainSource, "startNextMatchButton.addEventListener", "same-screen next-match action remains wired");
assertContains(mainSource, "client.startNextMatch()", "next-match button sends the rematch request through the client");

assertContains(styleSource, ".match-panel {", "match panel has dedicated styling");
assertContains(styleSource, ".match-panel summary {", "match panel summary remains styled as a reachable control");
assertContains(styleSource, ".match-panel .toolbar {", "toolbar spacing remains scoped inside the match panel");
assertContains(styleSource, "@media (max-width: 720px)", "mobile layout breakpoint remains present");
assertMatches(
  styleSource,
  /@media \(max-width: 720px\) \{[\s\S]*\.app-shell \{[\s\S]*grid-template-rows: auto minmax\(300px, 1fr\) auto auto;/,
  "mobile shell keeps the arena as the flexible central row"
);
assertMatches(
  styleSource,
  /@media \(max-width: 720px\) \{[\s\S]*\.toolbar \{[\s\S]*display: grid;[\s\S]*grid-template-columns: repeat\(2, minmax\(110px, 1fr\)\);/,
  "mobile toolbar remains a stable two-column control grid"
);
assertMatches(
  styleSource,
  /@media \(max-width: 720px\) \{[\s\S]*\.controls \{[\s\S]*grid-template-columns: repeat\(4, minmax\(58px, 1fr\)\);/,
  "mobile gameplay controls stay in a stable bottom grid"
);

console.log("[PASS] telegram_main_layout_contract");
