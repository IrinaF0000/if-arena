export const protocolVersion = 1;

export type ServerMessageType = "auth_result" | "match_joined" | "input_ack" | "snapshot" | "event_batch" | "error";

export type ProtocolEnvelope = {
  version: typeof protocolVersion;
  type: ServerMessageType;
  requestId?: string;
  payload: Record<string, unknown>;
};

export type ClientParseError = {
  version: typeof protocolVersion;
  type: "client_parse_error";
  payload: {
    reason: "malformed_json" | "invalid_envelope";
  };
};

export type IncomingMessage = ProtocolEnvelope | ClientParseError;

const serverMessageTypes: readonly ServerMessageType[] = [
  "auth_result",
  "match_joined",
  "input_ack",
  "snapshot",
  "event_batch",
  "error"
];

export function parseIncomingMessage(raw: string): IncomingMessage {
  try {
    const value = JSON.parse(raw) as unknown;
    if (isProtocolEnvelope(value)) {
      return value;
    }
    return parseError("invalid_envelope");
  } catch {
    return parseError("malformed_json");
  }
}

function parseError(reason: ClientParseError["payload"]["reason"]): ClientParseError {
  return {
    version: protocolVersion,
    type: "client_parse_error",
    payload: { reason }
  };
}

function isProtocolEnvelope(value: unknown): value is ProtocolEnvelope {
  if (!isRecord(value)) {
    return false;
  }

  const { version, type, requestId, payload } = value;
  if (version !== protocolVersion || typeof type !== "string" || !isServerMessageType(type) || !isRecord(payload)) {
    return false;
  }

  return requestId === undefined || typeof requestId === "string";
}

function isServerMessageType(value: string): value is ServerMessageType {
  return serverMessageTypes.includes(value as ServerMessageType);
}

function isRecord(value: unknown): value is Record<string, unknown> {
  return typeof value === "object" && value !== null && !Array.isArray(value);
}
