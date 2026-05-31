import {
  type ClientEnvelope,
  type CommandKind,
  type Direction,
  type IncomingMessage,
  createAuthRequest,
  createInputCommand,
  createJoinRequest,
  createMatchRequest,
  createPong,
  createStartNextMatchRequest,
  parseIncomingMessage
} from "../protocol/ProtocolTypes";

export type ConnectionState = "disconnected" | "connecting" | "connected" | "authenticated" | "in_match" | "closed" | "error";

export type WebSocketClientOptions = {
  url: string;
  displayName: string;
  onStateChanged: (state: ConnectionState) => void;
  onMessage: (message: IncomingMessage) => void;
  onDiagnostic?: (message: string) => void;
};

const maxQueuedSends = 32;

export class WebSocketClient {
  private socket: WebSocket | null = null;
  private readonly queuedSends: ClientEnvelope[] = [];
  private matchId: string | null = null;
  private sessionSeq = 1;
  private state: ConnectionState = "disconnected";

  public constructor(private readonly options: WebSocketClientOptions) {}

  public connect(): void {
    if (this.socket && this.socket.readyState !== WebSocket.CLOSED) {
      return;
    }

    this.setState("connecting");
    this.socket = new WebSocket(this.options.url);

    this.socket.addEventListener("open", () => {
      this.setState("connected");
      this.flushQueue();
    });

    this.socket.addEventListener("message", (event: MessageEvent<unknown>) => {
      const parsed = parseIncomingMessage(event.data);
      this.rememberServerState(parsed);
      this.options.onMessage(parsed);
    });

    this.socket.addEventListener("close", (event: CloseEvent) => {
      const reason = event.reason.length > 0 ? event.reason : `code ${event.code}`;
      const wasInMatch = this.matchId !== null;
      this.matchId = null;
      this.sessionSeq = 1;
      this.queuedSends.length = 0;
      this.options.onDiagnostic?.(
        wasInMatch ? `disconnected, rejoin not supported yet (${reason})` : `websocket closed: ${reason}`
      );
      this.setState("closed");
    });

    this.socket.addEventListener("error", () => {
      this.queuedSends.length = 0;
      this.options.onDiagnostic?.("websocket error");
      this.setState("error");
    });
  }

  public sendAuthRequest(initData: string): void {
    if (!this.socket || this.socket.readyState === WebSocket.CLOSED) {
      this.options.onDiagnostic?.("connect before auth");
      return;
    }
    if (this.state !== "connecting" && this.state !== "connected") {
      this.options.onDiagnostic?.("auth already completed");
      return;
    }
    this.send(createAuthRequest(initData, this.options.displayName), true);
  }

  public createMatch(): void {
    if (this.state !== "authenticated") {
      this.options.onDiagnostic?.("create requires authentication");
      return;
    }
    this.send(createMatchRequest());
  }

  public joinMatch(matchCode: string): void {
    if (this.state !== "authenticated") {
      this.options.onDiagnostic?.("join requires authentication");
      return;
    }
    const trimmed = matchCode.trim();
    if (trimmed.length > 0) {
      this.send(createJoinRequest(trimmed));
    }
  }

  public startNextMatch(): void {
    if (!this.matchId) {
      this.options.onDiagnostic?.("next match requires an active match");
      return;
    }
    this.send(createStartNextMatchRequest(this.matchId));
  }

  public sendCommand(kind: CommandKind, direction?: Direction): void {
    if (this.state !== "in_match" || !this.matchId) {
      this.options.onDiagnostic?.("join a match before sending commands");
      return;
    }
    this.send(createInputCommand(this.matchId, this.sessionSeq, kind, direction));
    this.sessionSeq += 1;
  }

  private send(value: ClientEnvelope, allowQueue = false): void {
    if (!this.socket || this.socket.readyState === WebSocket.CONNECTING) {
      if (allowQueue) {
        this.enqueue(value);
      }
      return;
    }
    if (this.socket.readyState !== WebSocket.OPEN) {
      return;
    }
    this.socket.send(JSON.stringify(value));
  }

  private enqueue(value: ClientEnvelope): void {
    if (this.queuedSends.length >= maxQueuedSends) {
      this.queuedSends.shift();
    }
    this.queuedSends.push(value);
  }

  private flushQueue(): void {
    while (this.queuedSends.length > 0 && this.socket?.readyState === WebSocket.OPEN) {
      const value = this.queuedSends.shift();
      if (value) {
        this.socket.send(JSON.stringify(value));
      }
    }
  }

  private rememberServerState(message: IncomingMessage): void {
    if (message.type === "auth_result" && (message.payload.accepted === true || message.payload.ok === true)) {
      this.setState("authenticated");
      return;
    }
    if (message.type === "match_joined") {
      this.matchId = message.payload.matchId;
      this.sessionSeq = 1;
      this.setState("in_match");
      return;
    }
    if (message.type === "ping") {
      this.send(createPong());
    }
  }

  private setState(state: ConnectionState): void {
    this.state = state;
    this.options.onStateChanged(state);
  }
}
