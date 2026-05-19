export class ArenaCanvas {
  private readonly context: CanvasRenderingContext2D;

  public constructor(private readonly canvas: HTMLCanvasElement) {
    const context = canvas.getContext("2d");
    if (!context) {
      throw new Error("2D canvas context is unavailable");
    }
    this.context = context;
  }

  public renderPlaceholder(): void {
    const { width, height } = this.canvas;
    this.context.clearRect(0, 0, width, height);
    this.context.strokeRect(20, 20, width - 40, height - 40);
    this.context.fillText("IF Arena Mini App placeholder", 40, 50);
    this.context.fillText("Objective Run: 21x13 grid-authored arena, smooth movement.", 40, 75);
    this.context.fillText("Player view: own base at bottom, enemy base at top.", 40, 100);
    this.context.fillText("Server-authoritative snapshots will be rendered here.", 40, 125);
  }
}
