import { type IncomingMessage, parseIncomingMessage } from "../protocol/ProtocolTypes";

export type ConnectionState = "disconnected" | "connecting" | "connected" | "closed" | "error";

export type WebSocketClientOptions = {
  url: string;
  onStateChanged: (state: ConnectionState) => void;
  onMessage: (message: IncomingMessage) => void;
};

export class WebSocketClient {
  private socket: WebSocket | null = null;

  public constructor(private readonly options: WebSocketClientOptions) {}

  public connect(): void {
    if (this.socket && this.socket.readyState !== WebSocket.CLOSED) {
      return;
    }

    this.options.onStateChanged("connecting");
    this.socket = new WebSocket(this.options.url);

    this.socket.addEventListener("open", () => {
      this.options.onStateChanged("connected");
    });

    this.socket.addEventListener("message", (event: MessageEvent<string>) => {
      this.options.onMessage(parseIncomingMessage(event.data));
    });

    this.socket.addEventListener("close", () => {
      this.options.onStateChanged("closed");
    });

    this.socket.addEventListener("error", () => {
      this.options.onStateChanged("error");
    });
  }

  public sendAuthRequest(initData: string): void {
    this.send({
      version: 1,
      type: "auth_request",
      requestId: crypto.randomUUID(),
      payload: {
        mode: initData.length > 0 ? "telegram" : "demo",
        initData,
        displayName: initData.length > 0 ? undefined : "Local Demo Player"
      }
    });
  }

  private send(value: unknown): void {
    if (!this.socket || this.socket.readyState !== WebSocket.OPEN) {
      return;
    }
    this.socket.send(JSON.stringify(value));
  }
}
