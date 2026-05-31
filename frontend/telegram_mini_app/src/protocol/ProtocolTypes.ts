export const protocolVersion = 1;

export type Team = "blue" | "red";
export type ObjectiveState = "at_spawn" | "dropped" | "carried" | "respawning" | "captured";
export type CommandKind = "move" | "aim" | "attack" | "dash" | "interact" | "stop";

export type Direction = {
  x: -1 | 0 | 1;
  y: -1 | 0 | 1;
};

export type PlayerSnapshot = {
  playerId: string;
  heroId: string;
  team: Team;
  x: number;
  y: number;
  hp: number;
  attackCooldown: number;
  dashCooldown: number;
  inOwnBase: boolean;
};

export type ObjectiveSnapshot = {
  state: ObjectiveState;
  x: number;
  y: number;
  carrierPlayerId: string;
  pickupLockTicks: number;
  respawnTicks: number;
};

export type ScoreSnapshot = {
  team: Team;
  score: number;
};

export type HazardSnapshot = {
  id: string;
  kind: "mine" | "tower" | "crow";
  visualId: string;
  x: number;
  y: number;
  radius: number;
  range: number;
  damage: number;
  effect: "damage" | "damage_drop_objective";
  trigger: "proximity" | "range";
  icon: string;
  blocksMovement: boolean;
  causesDrop: boolean;
  rangeRadius: number;
  team: "neutral";
  cooldownTicks: number;
  cooldown: number;
  triggered: boolean;
};

export type ScenarioMetadata = {
  id: string;
  mode: string;
  version: number;
  source: string;
};

export type ObstacleSnapshot = {
  id: string;
  kind: "blocking_obstacle";
  visualId: string;
  x: number;
  y: number;
  blocksMovement: boolean;
  damage: number;
  causesDrop: boolean;
  rangeRadius: number;
  cooldownTicks: number;
  cooldown: number;
  team: "neutral";
};

export type SnapshotPayload = {
  matchId: string;
  tick: number;
  serverTick: number;
  finished: boolean;
  scenario: ScenarioMetadata;
  map: {
    width: number;
    height: number;
  };
  obstacles: ObstacleSnapshot[];
  players: PlayerSnapshot[];
  objective: ObjectiveSnapshot;
  scores: ScoreSnapshot[];
  hazards: HazardSnapshot[];
};

export type AuthResultMessage = {
  version: typeof protocolVersion;
  type: "auth_result";
  requestId?: string;
  payload: {
    accepted?: boolean;
    ok?: boolean;
    sessionId?: string;
    playerId?: string;
    displayName?: string;
  };
};

export type MatchJoinedMessage = {
  version: typeof protocolVersion;
  type: "match_joined";
  requestId?: string;
  payload: {
    matchId: string;
    matchCode: string;
    scenario: ScenarioMetadata;
    team?: Team;
  };
};

export type InputAckMessage = {
  version: typeof protocolVersion;
  type: "input_ack";
  requestId?: string;
  payload: {
    accepted: boolean;
    reason?: string;
    serverTick?: number;
  };
};

export type SnapshotMessage = {
  version: typeof protocolVersion;
  type: "snapshot";
  requestId?: string;
  payload: SnapshotPayload;
};

export type EventBatchMessage = {
  version: typeof protocolVersion;
  type: "event_batch";
  requestId?: string;
  payload: {
    matchId: string;
    tick?: number;
    serverTick?: number;
    events?: unknown[];
  };
};

export type ErrorMessage = {
  version: typeof protocolVersion;
  type: "error";
  requestId?: string;
  payload: {
    code: string;
    message: string;
  };
};

export type PingMessage = {
  version: typeof protocolVersion;
  type: "ping";
  requestId?: string;
  payload: Record<string, never>;
};

export type PongMessage = {
  version: typeof protocolVersion;
  type: "pong";
  requestId?: string;
  payload: Record<string, never>;
};

export type ServerMessage =
  | AuthResultMessage
  | MatchJoinedMessage
  | InputAckMessage
  | SnapshotMessage
  | EventBatchMessage
  | ErrorMessage
  | PingMessage
  | PongMessage;

export type ClientParseError = {
  version: typeof protocolVersion;
  type: "client_parse_error";
  payload: {
    reason: "non_text_message" | "malformed_json" | "invalid_envelope";
  };
};

export type IncomingMessage = ServerMessage | ClientParseError;

export type ClientEnvelope = {
  version: typeof protocolVersion;
  type: "auth_request" | "create_match" | "join_match" | "start_next_match" | "input_command" | "pong";
  requestId?: string;
  sessionSeq?: number;
  payload: Record<string, unknown>;
};

export function parseIncomingMessage(raw: unknown): IncomingMessage {
  if (typeof raw !== "string") {
    return parseError("non_text_message");
  }

  try {
    const value = JSON.parse(raw) as unknown;
    if (isServerMessage(value)) {
      return value;
    }
    return parseError("invalid_envelope");
  } catch {
    return parseError("malformed_json");
  }
}

export function createAuthRequest(initData: string, displayName: string): ClientEnvelope {
  const hasTelegramInitData = initData.length > 0;
  return {
    version: protocolVersion,
    type: "auth_request",
    requestId: requestId(),
    payload: hasTelegramInitData
      ? { mode: "telegram", initData }
      : { mode: "demo", displayName }
  };
}

export function createMatchRequest(): ClientEnvelope {
  return {
    version: protocolVersion,
    type: "create_match",
    requestId: requestId(),
    payload: {
      mode: "objective_run",
      scenario: "arena_small_objective_run"
    }
  };
}

export function createJoinRequest(matchCode: string): ClientEnvelope {
  return {
    version: protocolVersion,
    type: "join_match",
    requestId: requestId(),
    payload: { matchCode }
  };
}

export function createStartNextMatchRequest(matchId: string): ClientEnvelope {
  return {
    version: protocolVersion,
    type: "start_next_match",
    requestId: requestId(),
    payload: { matchId }
  };
}

export function createInputCommand(matchId: string, sessionSeq: number, kind: CommandKind, direction?: Direction): ClientEnvelope {
  const command: Record<string, unknown> = { kind };
  if (direction) {
    command.direction = direction;
  }

  return {
    version: protocolVersion,
    type: "input_command",
    requestId: requestId(),
    sessionSeq,
    payload: {
      matchId,
      command
    }
  };
}

export function createPong(): ClientEnvelope {
  return {
    version: protocolVersion,
    type: "pong",
    requestId: requestId(),
    payload: {}
  };
}

function parseError(reason: ClientParseError["payload"]["reason"]): ClientParseError {
  return {
    version: protocolVersion,
    type: "client_parse_error",
    payload: { reason }
  };
}

function isServerMessage(value: unknown): value is ServerMessage {
  if (!isEnvelopeBase(value)) {
    return false;
  }

  switch (value.type) {
    case "auth_result":
      return isAuthResult(value);
    case "match_joined":
      return isMatchJoined(value);
    case "input_ack":
      return isInputAck(value);
    case "snapshot":
      return isSnapshot(value);
    case "event_batch":
      return isEventBatch(value);
    case "error":
      return isError(value);
    case "ping":
      return isPing(value);
    case "pong":
      return isPong(value);
    default:
      return false;
  }
}

function isEnvelopeBase(value: unknown): value is { version: 1; type: string; requestId?: string; payload: unknown } {
  if (!isRecord(value)) {
    return false;
  }
  return (
    value.version === protocolVersion &&
    typeof value.type === "string" &&
    (value.requestId === undefined || typeof value.requestId === "string") &&
    isRecord(value.payload)
  );
}

function isAuthResult(value: { payload: unknown }): value is AuthResultMessage {
  const payload = value.payload;
  return (
    isRecord(payload) &&
    optionalBoolean(payload.accepted) &&
    optionalBoolean(payload.ok) &&
    optionalString(payload.sessionId) &&
    optionalString(payload.playerId) &&
    optionalString(payload.displayName)
  );
}

function isMatchJoined(value: { payload: unknown }): value is MatchJoinedMessage {
  const payload = value.payload;
  return (
    isRecord(payload) &&
    typeof payload.matchId === "string" &&
    typeof payload.matchCode === "string" &&
    isScenarioMetadata(payload.scenario)
  );
}

function isInputAck(value: { payload: unknown }): value is InputAckMessage {
  const payload = value.payload;
  return isRecord(payload) && typeof payload.accepted === "boolean" && optionalString(payload.reason) && optionalNumber(payload.serverTick);
}

function isSnapshot(value: { payload: unknown }): value is SnapshotMessage {
  const payload = value.payload;
  return (
    isRecord(payload) &&
    typeof payload.matchId === "string" &&
    isNumber(payload.tick) &&
    isNumber(payload.serverTick) &&
    typeof payload.finished === "boolean" &&
    isScenarioMetadata(payload.scenario) &&
    isRecord(payload.map) &&
    isNumber(payload.map.width) &&
    isNumber(payload.map.height) &&
    Array.isArray(payload.obstacles) &&
    payload.obstacles.every(isObstacleSnapshot) &&
    Array.isArray(payload.players) &&
    payload.players.every(isPlayerSnapshot) &&
    isObjectiveSnapshot(payload.objective) &&
    Array.isArray(payload.scores) &&
    payload.scores.every(isScoreSnapshot) &&
    Array.isArray(payload.hazards) &&
    payload.hazards.every(isHazardSnapshot)
  );
}

function isEventBatch(value: { payload: unknown }): value is EventBatchMessage {
  const payload = value.payload;
  return (
    isRecord(payload) &&
    typeof payload.matchId === "string" &&
    optionalNumber(payload.tick) &&
    optionalNumber(payload.serverTick) &&
    (payload.events === undefined || Array.isArray(payload.events))
  );
}

function isError(value: { payload: unknown }): value is ErrorMessage {
  const payload = value.payload;
  return isRecord(payload) && typeof payload.code === "string" && typeof payload.message === "string";
}

function isPing(value: { payload: unknown }): value is PingMessage {
  return isRecord(value.payload);
}

function isPong(value: { payload: unknown }): value is PongMessage {
  return isRecord(value.payload);
}

function isPlayerSnapshot(value: unknown): value is PlayerSnapshot {
  return (
    isRecord(value) &&
    typeof value.playerId === "string" &&
    typeof value.heroId === "string" &&
    isTeam(value.team) &&
    isNumber(value.x) &&
    isNumber(value.y) &&
    isNumber(value.hp) &&
    isNumber(value.attackCooldown) &&
    isNumber(value.dashCooldown) &&
    typeof value.inOwnBase === "boolean"
  );
}

function isObjectiveSnapshot(value: unknown): value is ObjectiveSnapshot {
  return (
    isRecord(value) &&
    isObjectiveState(value.state) &&
    isNumber(value.x) &&
    isNumber(value.y) &&
    typeof value.carrierPlayerId === "string" &&
    isNumber(value.pickupLockTicks) &&
    isNumber(value.respawnTicks)
  );
}

function isScoreSnapshot(value: unknown): value is ScoreSnapshot {
  return isRecord(value) && isTeam(value.team) && isNumber(value.score);
}

function isHazardSnapshot(value: unknown): value is HazardSnapshot {
  return (
    isRecord(value) &&
    typeof value.id === "string" &&
    (value.kind === "mine" || value.kind === "tower" || value.kind === "crow") &&
    typeof value.visualId === "string" &&
    value.visualId.length > 0 &&
    isNumber(value.x) &&
    isNumber(value.y) &&
    isNumber(value.radius) &&
    isNumber(value.range) &&
    isNumber(value.damage) &&
    (value.effect === "damage" || value.effect === "damage_drop_objective") &&
    (value.trigger === "proximity" || value.trigger === "range") &&
    typeof value.icon === "string" &&
    typeof value.blocksMovement === "boolean" &&
    typeof value.causesDrop === "boolean" &&
    isNumber(value.rangeRadius) &&
    value.blocksMovement === false &&
    value.causesDrop === (value.effect === "damage_drop_objective") &&
    value.rangeRadius === (value.trigger === "range" ? value.range : value.radius) &&
    value.team === "neutral" &&
    isNumber(value.cooldownTicks) &&
    isNumber(value.cooldown) &&
    typeof value.triggered === "boolean"
  );
}

function isScenarioMetadata(value: unknown): value is ScenarioMetadata {
  return (
    isRecord(value) &&
    typeof value.id === "string" &&
    typeof value.mode === "string" &&
    isNumber(value.version) &&
    typeof value.source === "string"
  );
}

function isObstacleSnapshot(value: unknown): value is ObstacleSnapshot {
  return (
    isRecord(value) &&
    typeof value.id === "string" &&
    value.kind === "blocking_obstacle" &&
    typeof value.visualId === "string" &&
    value.visualId.length > 0 &&
    isNumber(value.x) &&
    isNumber(value.y) &&
    value.blocksMovement === true &&
    isNumber(value.damage) &&
    value.damage === 0 &&
    value.causesDrop === false &&
    isNumber(value.rangeRadius) &&
    value.rangeRadius === 0 &&
    isNumber(value.cooldownTicks) &&
    isNumber(value.cooldown) &&
    value.team === "neutral"
  );
}

function isTeam(value: unknown): value is Team {
  return value === "blue" || value === "red";
}

function isObjectiveState(value: unknown): value is ObjectiveState {
  return value === "at_spawn" || value === "dropped" || value === "carried" || value === "respawning" || value === "captured";
}

function isRecord(value: unknown): value is Record<string, unknown> {
  return typeof value === "object" && value !== null && !Array.isArray(value);
}

function isNumber(value: unknown): value is number {
  return typeof value === "number" && Number.isFinite(value);
}

function optionalString(value: unknown): boolean {
  return value === undefined || typeof value === "string";
}

function optionalNumber(value: unknown): boolean {
  return value === undefined || isNumber(value);
}

function optionalBoolean(value: unknown): boolean {
  return value === undefined || typeof value === "boolean";
}

function requestId(): string {
  if (typeof crypto.randomUUID === "function") {
    return crypto.randomUUID();
  }
  return `r-${Date.now()}-${Math.floor(Math.random() * 1_000_000)}`;
}
