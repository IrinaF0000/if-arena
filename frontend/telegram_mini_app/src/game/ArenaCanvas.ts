import type { Direction, HazardSnapshot, ObstacleSnapshot, PlayerSnapshot, SnapshotPayload, Team } from "../protocol/ProtocolTypes";

export const playerSpritePath = "/players/swordsman.svg";

export class ArenaCanvas {
  private readonly context: CanvasRenderingContext2D;
  private readonly playerSprite: HTMLImageElement;
  private snapshot: SnapshotPayload | null = null;
  private localPlayerId: string | null = null;
  private status = "offline";
  private aimDirection: Direction = { x: 0, y: -1 };

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

  public render(): void {
    const { width, height } = this.canvas;
    this.context.clearRect(0, 0, width, height);
    this.context.fillStyle = "#101820";
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
      for (const player of snapshot.players) {
        this.drawPlayer(player, snapshot, originX, originY, cell);
      }
      this.drawHud(snapshot);
    } else {
      this.drawOfflineHud();
    }
  }

  private drawBoard(originX: number, originY: number, boardWidth: number, boardHeight: number, cell: number, mapWidth: number, mapHeight: number): void {
    this.context.fillStyle = "#18232b";
    this.context.fillRect(originX, originY, boardWidth, boardHeight);
    this.context.strokeStyle = "rgba(255,255,255,0.08)";
    this.context.lineWidth = 1;
    for (let x = 0; x <= mapWidth; x += 1) {
      this.line(originX + x * cell, originY, originX + x * cell, originY + boardHeight);
    }
    for (let y = 0; y <= mapHeight; y += 1) {
      this.line(originX, originY + y * cell, originX + boardWidth, originY + y * cell);
    }
    this.context.strokeStyle = "#dde7ee";
    this.context.lineWidth = 2;
    this.context.strokeRect(originX, originY, boardWidth, boardHeight);
  }

  private drawBase(originX: number, originY: number, cell: number, mapWidth: number, mapHeight: number, team: Team): void {
    const x = Math.floor(mapWidth / 2);
    const y = team === "blue" ? mapHeight - 2 : 1;
    const point = this.worldToCanvas(x, y, originX, originY, cell, mapWidth, mapHeight);
    this.context.fillStyle = team === "blue" ? "rgba(68, 194, 255, 0.22)" : "rgba(255, 112, 96, 0.22)";
    this.context.strokeStyle = team === "blue" ? "#44c2ff" : "#ff7060";
    this.roundRect(point.x - cell * 2.5, point.y - cell, cell * 5, cell * 2, 6);
    this.context.fill();
    this.context.stroke();
  }

  private drawObjective(snapshot: SnapshotPayload, originX: number, originY: number, cell: number): void {
    const { objective } = snapshot;
    const point = this.worldToCanvas(objective.x, objective.y, originX, originY, cell, snapshot.map.width, snapshot.map.height);
    this.context.fillStyle = objective.state === "carried" ? "#f6d365" : "#f7b731";
    this.context.strokeStyle = "#fff3b0";
    this.context.lineWidth = 2;
    this.context.beginPath();
    this.context.arc(point.x, point.y, cell * 0.52, 0, Math.PI * 2);
    this.context.stroke();
    this.context.beginPath();
    this.context.moveTo(point.x, point.y - cell * 0.35);
    this.context.lineTo(point.x + cell * 0.35, point.y);
    this.context.lineTo(point.x, point.y + cell * 0.35);
    this.context.lineTo(point.x - cell * 0.35, point.y);
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
    this.context.fillStyle = "#314451";
    this.context.strokeStyle = "#89a7b8";
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
    this.context.fillStyle = hazard.kind === "tower" ? "#b38cff" : "#ff9f43";
    this.context.globalAlpha = hazard.triggered ? 0.35 : 0.9;
    this.context.beginPath();
    this.context.arc(point.x, point.y, cell * 0.24, 0, Math.PI * 2);
    this.context.fill();
    this.context.globalAlpha = 1;
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
    const teamColor = player.team === "blue" ? "#44c2ff" : "#ff7060";
    this.context.fillStyle = teamColor;
    this.context.strokeStyle = teamColor;
    this.context.lineWidth = player.playerId === this.localPlayerId ? 4 : 3;
    this.context.beginPath();
    this.context.arc(point.x, point.y, cell * 0.42, 0, Math.PI * 2);
    this.context.stroke();
    this.drawPlayerSprite(point.x, point.y, cell * 0.78, player.playerId === this.localPlayerId);

    const hpWidth = cell * 0.7;
    const hpRatio = Math.max(0, Math.min(1, player.hp / 100));
    this.context.fillStyle = "rgba(0,0,0,0.45)";
    this.context.fillRect(point.x - hpWidth / 2, point.y - cell * 0.58, hpWidth, 4);
    this.context.fillStyle = hpRatio > 0.35 ? "#4cd964" : "#ff5e57";
    this.context.fillRect(point.x - hpWidth / 2, point.y - cell * 0.58, hpWidth * hpRatio, 4);
    if (carriesObjective) {
      this.context.fillStyle = "#ffd640";
      this.context.beginPath();
      this.context.moveTo(point.x, point.y - cell * 0.74);
      this.context.lineTo(point.x + cell * 0.16, point.y - cell * 0.58);
      this.context.lineTo(point.x, point.y - cell * 0.42);
      this.context.lineTo(point.x - cell * 0.16, point.y - cell * 0.58);
      this.context.closePath();
      this.context.fill();
    }
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
    this.context.fillStyle = "rgba(0,0,0,0.45)";
    this.context.fillRect(14, 12, 270, 58);
    this.context.fillStyle = "#f8fbff";
    this.context.font = "16px system-ui, sans-serif";
    this.context.fillText(`Blue ${blue} - ${red} Red`, 28, 36);
    this.context.fillText(`Tick ${snapshot.serverTick} | ${snapshot.objective.state} | ${this.status}`, 28, 58);
  }

  private drawOfflineHud(): void {
    this.context.fillStyle = "#f8fbff";
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
