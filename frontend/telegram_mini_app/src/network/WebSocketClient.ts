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

  public constructor(private readonly options: WebSocketClientOptions) {}

  public connect(): void {
    if (this.socket && this.socket.readyState !== WebSocket.CLOSED) {
      return;
    }

    this.options.onStateChanged("connecting");
    this.socket = new WebSocket(this.options.url);

    this.socket.addEventListener("open", () => {
      this.options.onStateChanged("connected");
      this.flushQueue();
    });

    this.socket.addEventListener("message", (event: MessageEvent<unknown>) => {
      const parsed = parseIncomingMessage(event.data);
      this.rememberServerState(parsed);
      this.options.onMessage(parsed);
    });

    this.socket.addEventListener("close", (event: CloseEvent) => {
      const reason = event.reason.length > 0 ? event.reason : `code ${event.code}`;
      this.options.onDiagnostic?.(`websocket closed: ${reason}`);
      this.options.onStateChanged("closed");
    });

    this.socket.addEventListener("error", () => {
      this.options.onDiagnostic?.("websocket error");
      this.options.onStateChanged("error");
    });
  }

  public sendAuthRequest(initData: string): void {
    this.send(createAuthRequest(initData, this.options.displayName));
  }

  public createMatch(): void {
    this.send(createMatchRequest());
  }

  public joinMatch(matchCode: string): void {
    const trimmed = matchCode.trim();
    if (trimmed.length > 0) {
      this.send(createJoinRequest(trimmed));
    }
  }

  public sendCommand(kind: CommandKind, direction?: Direction): void {
    if (!this.matchId) {
      return;
    }
    this.send(createInputCommand(this.matchId, this.sessionSeq, kind, direction));
    this.sessionSeq += 1;
  }

  private send(value: ClientEnvelope): void {
    if (!this.socket || this.socket.readyState === WebSocket.CONNECTING) {
      this.enqueue(value);
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
      this.options.onStateChanged("authenticated");
      return;
    }
    if (message.type === "match_joined") {
      this.matchId = message.payload.matchId;
      this.sessionSeq = 1;
      this.options.onStateChanged("in_match");
      return;
    }
    if (message.type === "ping") {
      this.send(createPong());
    }
  }
}
