import type { Direction, HazardSnapshot, ObstacleSnapshot, PlayerSnapshot, SnapshotPayload, Team } from "../protocol/ProtocolTypes";

export const playerSpritePath = "/players/swordsman.svg";

type ActionFeedbackKind = "attack" | "dash";
type VisualEventKind = "attack_hit" | "hazard_hit" | "objective_picked_up" | "objective_dropped" | "objective_captured";

type VisualEventFeedback = {
  kind: VisualEventKind;
  label: string;
  targetPlayerId?: string;
  from?: { x: number; y: number };
  to?: { x: number; y: number };
};

export class ArenaCanvas {
  private readonly context: CanvasRenderingContext2D;
  private readonly playerSprite: HTMLImageElement;
  private snapshot: SnapshotPayload | null = null;
  private localPlayerId: string | null = null;
  private status = "offline";
  private aimDirection: Direction = { x: 0, y: -1 };
  private actionFeedback: { kind: ActionFeedbackKind; direction: Direction } | null = null;
  private eventFeedbacks: VisualEventFeedback[] = [];

  public constructor(private readonly canvas: HTMLCanvasElement) {
    const context = canvas.getContext("2d");
    if (!context) {
      throw new Error("2D canvas context is unavailable");
    }
    this.context = context;
    this.playerSprite = this.createPlayerSprite();
  }

  public setStatus(status: string): void {
    this.status = status;
    this.render();
  }

  public setSnapshot(snapshot: SnapshotPayload, localPlayerId: string | null): void {
    this.snapshot = snapshot;
    this.localPlayerId = localPlayerId;
    this.render();
  }

  public setAimDirection(direction: Direction | undefined): void {
    if (direction) {
      this.aimDirection = direction;
      this.render();
    }
  }

  public showActionFeedback(kind: ActionFeedbackKind, direction: Direction | undefined): void {
    this.actionFeedback = { kind, direction: direction ?? this.aimDirection };
    this.render();
  }

  public showEventFeedback(events: readonly unknown[] | undefined): void {
    const feedbacks = (events ?? []).map((event) => this.visualFeedbackFromEvent(event)).filter((event) => event !== null);
    if (feedbacks.length === 0) {
      return;
    }
    this.eventFeedbacks = feedbacks;
    this.render();
  }

  public render(): void {
    const { width, height } = this.canvas;
    this.context.clearRect(0, 0, width, height);
    this.context.fillStyle = "#111416";
    this.context.fillRect(0, 0, width, height);

    const snapshot = this.snapshot;
    const mapWidth = snapshot?.map.width ?? 21;
    const mapHeight = snapshot?.map.height ?? 13;
    const margin = 26;
    const cell = Math.min((width - margin * 2) / mapWidth, (height - margin * 2) / mapHeight);
    const boardWidth = cell * mapWidth;
    const boardHeight = cell * mapHeight;
    const originX = (width - boardWidth) / 2;
    const originY = (height - boardHeight) / 2;

    this.drawBoard(originX, originY, boardWidth, boardHeight, cell, mapWidth, mapHeight);
    this.drawBase(originX, originY, cell, mapWidth, mapHeight, "blue");
    this.drawBase(originX, originY, cell, mapWidth, mapHeight, "red");

    if (snapshot) {
      for (const obstacle of snapshot.obstacles) {
        this.drawObstacle(obstacle, snapshot, originX, originY, cell);
      }
      for (const hazard of snapshot.hazards) {
        this.drawHazard(hazard, originX, originY, cell);
      }
      this.drawObjective(snapshot, originX, originY, cell);
      this.drawLocalActionPreview(snapshot, originX, originY, cell);
      for (const player of snapshot.players) {
        this.drawPlayer(player, snapshot, originX, originY, cell);
      }
      this.drawEventFeedback(snapshot, originX, originY, cell);
      this.drawHud(snapshot);
      this.drawHazardLegend(snapshot);
      if (snapshot.finished) {
        this.drawMatchOverOverlay(snapshot);
      }
    } else {
      this.drawOfflineHud();
    }
  }

  private drawBoard(originX: number, originY: number, boardWidth: number, boardHeight: number, cell: number, mapWidth: number, mapHeight: number): void {
    this.context.fillStyle = "#1a1f20";
    this.context.fillRect(originX, originY, boardWidth, boardHeight);
    this.context.strokeStyle = "rgba(238,214,160,0.07)";
    this.context.lineWidth = 1;
    for (let x = 0; x <= mapWidth; x += 1) {
      this.line(originX + x * cell, originY, originX + x * cell, originY + boardHeight);
    }
    for (let y = 0; y <= mapHeight; y += 1) {
      this.line(originX, originY + y * cell, originX + boardWidth, originY + y * cell);
    }
    this.context.strokeStyle = "#bda769";
    this.context.lineWidth = 2.5;
    this.context.strokeRect(originX, originY, boardWidth, boardHeight);
  }

  private drawBase(originX: number, originY: number, cell: number, mapWidth: number, mapHeight: number, team: Team): void {
    const x = Math.floor(mapWidth / 2);
    const y = team === "blue" ? mapHeight - 2 : 1;
    const point = this.worldToCanvas(x, y, originX, originY, cell, mapWidth, mapHeight);
    this.context.fillStyle = team === "blue" ? "rgba(42, 190, 205, 0.22)" : "rgba(226, 82, 88, 0.2)";
    this.context.strokeStyle = team === "blue" ? "#2abecd" : "#e25258";
    this.roundRect(point.x - cell * 2.5, point.y - cell, cell * 5, cell * 2, 6);
    this.context.fill();
    this.context.stroke();
  }

  private drawObjective(snapshot: SnapshotPayload, originX: number, originY: number, cell: number): void {
    const { objective } = snapshot;
    const point = this.worldToCanvas(objective.x, objective.y, originX, originY, cell, snapshot.map.width, snapshot.map.height);
    this.context.fillStyle = objective.state === "carried" ? "#ffe071" : "#ffc857";
    this.context.strokeStyle = "#fff6c7";
    this.context.lineWidth = 2.5;
    this.context.beginPath();
    this.context.arc(point.x, point.y, cell * 0.58, 0, Math.PI * 2);
    this.context.stroke();
    this.context.beginPath();
    this.context.moveTo(point.x, point.y - cell * 0.4);
    this.context.lineTo(point.x + cell * 0.4, point.y);
    this.context.lineTo(point.x, point.y + cell * 0.4);
    this.context.lineTo(point.x - cell * 0.4, point.y);
    this.context.closePath();
    this.context.fill();
    this.context.stroke();
    if (objective.state !== "at_spawn") {
      this.context.fillStyle = "#fff7c2";
      this.context.font = "13px system-ui, sans-serif";
      this.context.textAlign = "center";
      this.context.fillText(objective.state.replace("_", " "), point.x, point.y - cell * 0.62);
      this.context.textAlign = "start";
    }
  }

  private drawObstacle(obstacle: ObstacleSnapshot, snapshot: SnapshotPayload, originX: number, originY: number, cell: number): void {
    const point = this.worldToCanvas(obstacle.x, obstacle.y, originX, originY, cell, snapshot.map.width, snapshot.map.height);
    const size = cell * 0.82;
    this.context.fillStyle = "#303633";
    this.context.strokeStyle = "#b49a62";
    this.context.lineWidth = 1.5;
    this.context.fillRect(point.x - size / 2, point.y - size / 2, size, size);
    this.context.strokeRect(point.x - size / 2, point.y - size / 2, size, size);
  }

  private drawHazard(hazard: HazardSnapshot, originX: number, originY: number, cell: number): void {
    const snapshot = this.snapshot;
    if (!snapshot) {
      return;
    }
    const point = this.worldToCanvas(hazard.x, hazard.y, originX, originY, cell, snapshot.map.width, snapshot.map.height);
    const icon = this.hazardIconKind(hazard);
    const effectColor = hazard.effect === "damage_drop_objective" ? "#ffcf5c" : "#ff8d7a";
    const rangeCells = hazard.trigger === "range" ? hazard.range : hazard.radius;
    this.context.save();
    this.context.globalAlpha = hazard.triggered ? 0.22 : 0.16;
    this.context.fillStyle = effectColor;
    this.context.strokeStyle = effectColor;
    this.context.lineWidth = 1.5;
    this.context.beginPath();
    this.context.arc(point.x, point.y, Math.max(cell * rangeCells, cell * 0.28), 0, Math.PI * 2);
    this.context.fill();
    this.context.stroke();
    this.context.restore();

    this.context.fillStyle = icon === "tower" ? "#b38cff" : icon === "crow" ? "#dce8ef" : "#ff9f43";
    this.context.globalAlpha = hazard.triggered ? 0.35 : 0.9;
    this.context.beginPath();
    if (icon === "crow") {
      this.context.arc(point.x, point.y, cell * 0.3, 0, Math.PI * 2);
      this.context.fill();
      this.context.fillStyle = "#1e2a31";
      this.context.beginPath();
      this.context.moveTo(point.x - cell * 0.32, point.y);
      this.context.lineTo(point.x, point.y - cell * 0.16);
      this.context.lineTo(point.x + cell * 0.32, point.y);
      this.context.lineTo(point.x, point.y + cell * 0.1);
      this.context.closePath();
      this.context.fill();
    } else if (icon === "tower") {
      const size = cell * 0.46;
      this.context.fillRect(point.x - size / 2, point.y - size / 2, size, size);
      this.context.strokeStyle = "#f0e9ff";
      this.context.lineWidth = 1.5;
      this.context.strokeRect(point.x - size / 2, point.y - size / 2, size, size);
    } else {
      this.context.moveTo(point.x, point.y - cell * 0.28);
      this.context.lineTo(point.x + cell * 0.28, point.y + cell * 0.22);
      this.context.lineTo(point.x - cell * 0.28, point.y + cell * 0.22);
      this.context.closePath();
      this.context.fill();
    }
    this.context.globalAlpha = 1;

    if (hazard.cooldown > 0) {
      this.context.fillStyle = "#f8fbff";
      this.context.font = "11px system-ui, sans-serif";
      this.context.textAlign = "center";
      this.context.fillText(String(hazard.cooldown), point.x, point.y + cell * 0.55);
      this.context.textAlign = "start";
    }
  }

  private hazardIconKind(hazard: HazardSnapshot): "mine" | "tower" | "crow" {
    if (hazard.icon.includes("tower")) {
      return "tower";
    }
    if (hazard.icon.includes("crow") || hazard.icon.includes("drone")) {
      return "crow";
    }
    return "mine";
  }

  private drawLocalActionPreview(snapshot: SnapshotPayload, originX: number, originY: number, cell: number): void {
    const player = this.localPlayer(snapshot);
    if (!player) {
      return;
    }
    const point = this.worldToCanvas(player.x, player.y, originX, originY, cell, snapshot.map.width, snapshot.map.height);
    const range = cell * 2.3;
    const end = this.directionEnd(point, this.aimDirection, range);
    this.context.strokeStyle = "rgba(255,255,255,0.22)";
    this.context.lineWidth = 1.5;
    this.context.beginPath();
    this.context.arc(point.x, point.y, range, 0, Math.PI * 2);
    this.context.stroke();
    this.context.strokeStyle = "rgba(255,245,170,0.78)";
    this.context.lineWidth = 2.5;
    this.context.beginPath();
    this.context.moveTo(point.x, point.y);
    this.context.lineTo(end.x, end.y);
    this.context.stroke();

    if (!this.actionFeedback) {
      return;
    }
    const actionEnd = this.directionEnd(point, this.actionFeedback.direction, this.actionFeedback.kind === "dash" ? range * 0.9 : range * 1.08);
    this.context.strokeStyle = this.actionFeedback.kind === "dash" ? "#70deff" : "#ffe45c";
    this.context.lineWidth = this.actionFeedback.kind === "dash" ? 6 : 4;
    this.context.beginPath();
    this.context.moveTo(point.x, point.y);
    this.context.lineTo(actionEnd.x, actionEnd.y);
    this.context.stroke();
  }

  private drawPlayer(player: PlayerSnapshot, snapshot: SnapshotPayload, originX: number, originY: number, cell: number): void {
    const point = this.worldToCanvas(player.x, player.y, originX, originY, cell, snapshot.map.width, snapshot.map.height);
    const carriesObjective = player.playerId === snapshot.objective.carrierPlayerId;
    if (carriesObjective) {
      this.context.strokeStyle = "#ffd640";
      this.context.lineWidth = 4;
      this.context.beginPath();
      this.context.arc(point.x, point.y, cell * 0.48, 0, Math.PI * 2);
      this.context.stroke();
    }
    const teamColor = player.team === "blue" ? "#2abecd" : "#e25258";
    this.context.fillStyle = teamColor;
    this.context.strokeStyle = teamColor;
    this.context.lineWidth = player.playerId === this.localPlayerId ? 4 : 3;
    this.context.beginPath();
    this.context.arc(point.x, point.y, cell * 0.5, 0, Math.PI * 2);
    this.context.stroke();
    this.drawPlayerSprite(point.x, point.y, cell * 0.92, player.playerId === this.localPlayerId);

    const hpWidth = cell * 0.82;
    const hpRatio = Math.max(0, Math.min(1, player.hp / 100));
    this.context.fillStyle = "rgba(0,0,0,0.45)";
    this.context.fillRect(point.x - hpWidth / 2, point.y - cell * 0.58, hpWidth, 4);
    this.context.fillStyle = hpRatio > 0.35 ? "#4cd964" : "#ff5e57";
    this.context.fillRect(point.x - hpWidth / 2, point.y - cell * 0.58, hpWidth * hpRatio, 4);
    if (player.playerId === this.localPlayerId) {
      this.drawCooldownBars(player, point, cell);
    }
    if (carriesObjective) {
      this.context.fillStyle = "#ffd640";
      this.context.beginPath();
      this.context.moveTo(point.x, point.y - cell * 0.74);
      this.context.lineTo(point.x + cell * 0.18, point.y - cell * 0.58);
      this.context.lineTo(point.x, point.y - cell * 0.4);
      this.context.lineTo(point.x - cell * 0.18, point.y - cell * 0.58);
      this.context.closePath();
      this.context.fill();
    }
  }

  private drawCooldownBars(player: PlayerSnapshot, point: { x: number; y: number }, cell: number): void {
    const width = cell * 0.75;
    const attackReady = 1 - Math.max(0, Math.min(1, player.attackCooldown / 3));
    const dashReady = 1 - Math.max(0, Math.min(1, player.dashCooldown / 5));
    this.context.fillStyle = "rgba(0,0,0,0.42)";
    this.context.fillRect(point.x - width / 2, point.y + cell * 0.54, width, 3);
    this.context.fillRect(point.x - width / 2, point.y + cell * 0.64, width, 3);
    this.context.fillStyle = "#ffe45c";
    this.context.fillRect(point.x - width / 2, point.y + cell * 0.54, width * attackReady, 3);
    this.context.fillStyle = "#70deff";
    this.context.fillRect(point.x - width / 2, point.y + cell * 0.64, width * dashReady, 3);
  }

  private drawEventFeedback(snapshot: SnapshotPayload, originX: number, originY: number, cell: number): void {
    for (const event of this.eventFeedbacks) {
      if (event.kind === "attack_hit" || event.kind === "hazard_hit") {
        const target = event.targetPlayerId ? snapshot.players.find((player) => player.playerId === event.targetPlayerId) : undefined;
        if (target) {
          const point = this.worldToCanvas(target.x, target.y, originX, originY, cell, snapshot.map.width, snapshot.map.height);
          this.context.strokeStyle = event.kind === "hazard_hit" ? "#ff9f43" : "#ff5e57";
          this.context.lineWidth = 5;
          this.context.beginPath();
          this.context.arc(point.x, point.y, cell * 0.62, 0, Math.PI * 2);
          this.context.stroke();
        }
      }
      const anchor = event.to ?? { x: snapshot.objective.x, y: snapshot.objective.y };
      const labelPoint = this.worldToCanvas(anchor.x, anchor.y, originX, originY, cell, snapshot.map.width, snapshot.map.height);
      this.context.fillStyle = "rgba(0,0,0,0.62)";
      this.context.fillRect(labelPoint.x - cell * 1.35, labelPoint.y - cell * 1.15, cell * 2.7, 20);
      this.context.fillStyle = "#f8fbff";
      this.context.font = "13px system-ui, sans-serif";
      this.context.textAlign = "center";
      this.context.fillText(event.label, labelPoint.x, labelPoint.y - cell * 0.72);
      this.context.textAlign = "start";
    }
  }

  private drawMatchOverOverlay(snapshot: SnapshotPayload): void {
    const blue = snapshot.scores.find((score) => score.team === "blue")?.score ?? 0;
    const red = snapshot.scores.find((score) => score.team === "red")?.score ?? 0;
    const winner = blue === red ? "Draw" : blue > red ? "Blue wins" : "Red wins";
    this.context.fillStyle = "rgba(0,0,0,0.58)";
    this.context.fillRect(0, 0, this.canvas.width, this.canvas.height);
    this.context.fillStyle = "#f8fbff";
    this.context.font = "28px system-ui, sans-serif";
    this.context.textAlign = "center";
    this.context.fillText("Match over", this.canvas.width / 2, this.canvas.height / 2 - 18);
    this.context.font = "18px system-ui, sans-serif";
    this.context.fillText(`${winner} ${blue}-${red}`, this.canvas.width / 2, this.canvas.height / 2 + 14);
    this.context.textAlign = "start";
  }

  private createPlayerSprite(): HTMLImageElement {
    const image = new Image();
    image.decoding = "async";
    image.src = playerSpritePath;
    image.addEventListener("load", () => this.render());
    return image;
  }

  private drawPlayerSprite(x: number, y: number, size: number, isLocal: boolean): void {
    const ready = this.playerSprite.complete && this.playerSprite.naturalWidth > 0;
    if (!ready) {
      this.context.beginPath();
      this.context.arc(x, y, size * 0.38, 0, Math.PI * 2);
      this.context.fill();
      this.context.stroke();
      return;
    }

    this.context.save();
    this.context.translate(x, y);
    this.context.rotate(isLocal ? this.directionAngle(this.aimDirection) : 0);
    this.context.drawImage(this.playerSprite, -size / 2, -size / 2, size, size);
    this.context.restore();
  }

  private directionAngle(direction: Direction): number {
    const dx = direction.x;
    const dy = direction.y;
    if (dx === 0 && dy === 0) {
      return 0;
    }
    return Math.atan2(dx, -dy);
  }

  private drawHud(snapshot: SnapshotPayload): void {
    const blue = snapshot.scores.find((score) => score.team === "blue")?.score ?? 0;
    const red = snapshot.scores.find((score) => score.team === "red")?.score ?? 0;
    this.context.fillStyle = "rgba(0,0,0,0.5)";
    this.context.fillRect(14, 12, 270, 58);
    this.context.fillStyle = "#f8fbff";
    this.context.font = "16px system-ui, sans-serif";
    this.context.fillText(`Blue ${blue} - ${red} Red`, 28, 36);
    this.context.fillText(`Tick ${snapshot.serverTick} | ${snapshot.objective.state} | ${this.status}`, 28, 58);
  }

  private drawHazardLegend(snapshot: SnapshotPayload): void {
    const unique = new Map<string, HazardSnapshot>();
    for (const hazard of snapshot.hazards) {
      if (!unique.has(hazard.icon)) {
        unique.set(hazard.icon, hazard);
      }
    }
    if (unique.size === 0) {
      return;
    }
    const lines = [...unique.values()].map((hazard) => {
      const label = this.hazardIconKind(hazard);
      const effect = hazard.effect === "damage_drop_objective" ? "damage + drop" : "damage";
      const reach = hazard.trigger === "range" ? hazard.range : hazard.radius;
      return `${label}: ${effect}, r${reach}`;
    });
    const width = 220;
    const lineHeight = 16;
    const height = 18 + lineHeight * lines.length;
    this.context.fillStyle = "rgba(0,0,0,0.42)";
    this.context.fillRect(this.canvas.width - width - 14, 12, width, height);
    this.context.fillStyle = "#f7f3e8";
    this.context.font = "12px system-ui, sans-serif";
    lines.forEach((line, index) => {
      this.context.fillText(line, this.canvas.width - width, 32 + index * lineHeight);
    });
  }

  private drawOfflineHud(): void {
    this.context.fillStyle = "#f7f3e8";
    this.context.font = "16px system-ui, sans-serif";
    this.context.fillText(`Connection: ${this.status}`, 28, 36);
  }

  private worldToCanvas(
    x: number,
    y: number,
    originX: number,
    originY: number,
    cell: number,
    mapWidth: number,
    mapHeight: number
  ): { x: number; y: number } {
    const point = this.viewerTeam() === "red" ? { x: mapWidth - 1 - x, y: mapHeight - 1 - y } : { x, y };
    return {
      x: originX + (point.x + 0.5) * cell,
      y: originY + (point.y + 0.5) * cell
    };
  }

  private viewerTeam(): Team {
    if (!this.snapshot || !this.localPlayerId) {
      return "blue";
    }
    return this.snapshot.players.find((player) => player.playerId === this.localPlayerId)?.team ?? "blue";
  }

  private localPlayer(snapshot: SnapshotPayload): PlayerSnapshot | undefined {
    if (!this.localPlayerId) {
      return undefined;
    }
    return snapshot.players.find((player) => player.playerId === this.localPlayerId);
  }

  private directionEnd(origin: { x: number; y: number }, direction: Direction, length: number): { x: number; y: number } {
    const dx = direction.x;
    let dy = direction.y;
    if (dx === 0 && dy === 0) {
      dy = -1;
    }
    const magnitude = Math.hypot(dx, dy);
    return {
      x: origin.x + (dx / magnitude) * length,
      y: origin.y + (dy / magnitude) * length
    };
  }

  private visualFeedbackFromEvent(event: unknown): VisualEventFeedback | null {
    if (!isRecord(event) || typeof event.type !== "string") {
      return null;
    }
    const targetPlayerId = typeof event.targetPlayerId === "string" ? event.targetPlayerId : undefined;
    const playerId = typeof event.playerId === "string" ? event.playerId : undefined;
    const to = isPoint(event.to) ? event.to : undefined;
    const actor = playerId && this.localPlayerId && playerId === this.localPlayerId ? "You" : "Enemy";
    const feedback = (kind: VisualEventKind, label: string): VisualEventFeedback => ({
      kind,
      label,
      ...(targetPlayerId ? { targetPlayerId } : {}),
      ...(to ? { to } : {})
    });
    switch (event.type) {
      case "attack_hit":
        return feedback("attack_hit", "hit");
      case "hazard_hit":
        return feedback("hazard_hit", "hazard hit");
      case "objective_picked_up":
        return feedback("objective_picked_up", `${actor} picked up`);
      case "objective_dropped":
        return feedback("objective_dropped", `${actor} dropped`);
      case "objective_captured":
        return feedback("objective_captured", `${actor} captured`);
      default:
        return null;
    }
  }

  private line(x1: number, y1: number, x2: number, y2: number): void {
    this.context.beginPath();
    this.context.moveTo(x1, y1);
    this.context.lineTo(x2, y2);
    this.context.stroke();
  }

  private roundRect(x: number, y: number, width: number, height: number, radius: number): void {
    this.context.beginPath();
    this.context.roundRect(x, y, width, height, radius);
  }
}

function isRecord(value: unknown): value is Record<string, unknown> {
  return typeof value === "object" && value !== null && !Array.isArray(value);
}

function isPoint(value: unknown): value is { x: number; y: number } {
  return isRecord(value) && typeof value.x === "number" && Number.isFinite(value.x) && typeof value.y === "number" && Number.isFinite(value.y);
}
