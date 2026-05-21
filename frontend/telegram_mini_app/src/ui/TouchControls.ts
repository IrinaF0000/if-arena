import type { CommandKind, Direction } from "../protocol/ProtocolTypes";

export type ControlSink = {
  sendCommand: (kind: CommandKind, direction?: Direction) => void;
};

const keyDirections = new Map<string, Direction>([
  ["KeyW", { x: 0, y: -1 }],
  ["ArrowUp", { x: 0, y: -1 }],
  ["KeyS", { x: 0, y: 1 }],
  ["ArrowDown", { x: 0, y: 1 }],
  ["KeyA", { x: -1, y: 0 }],
  ["ArrowLeft", { x: -1, y: 0 }],
  ["KeyD", { x: 1, y: 0 }],
  ["ArrowRight", { x: 1, y: 0 }]
]);

export class TouchControls {
  private readonly pressed = new Set<string>();

  public constructor(private readonly sink: ControlSink) {}

  public bindKeyboard(target: Window): void {
    target.addEventListener("keydown", (event) => {
      if (event.repeat) {
        return;
      }
      if (keyDirections.has(event.code)) {
        this.pressed.add(event.code);
        this.sink.sendCommand("move", this.currentDirection());
        event.preventDefault();
      }
      if (event.code === "Space") {
        this.sink.sendCommand("attack", this.currentDirection());
        event.preventDefault();
      }
      if (event.code === "ShiftLeft" || event.code === "ShiftRight") {
        this.sink.sendCommand("dash", this.currentDirection());
        event.preventDefault();
      }
      if (event.code === "KeyE") {
        this.sink.sendCommand("interact");
        event.preventDefault();
      }
    });

    target.addEventListener("keyup", (event) => {
      if (keyDirections.has(event.code)) {
        this.pressed.delete(event.code);
        const direction = this.currentDirection();
        if (direction.x === 0 && direction.y === 0) {
          this.sink.sendCommand("stop");
        } else {
          this.sink.sendCommand("move", direction);
        }
        event.preventDefault();
      }
    });
  }

  public bindButton(button: HTMLButtonElement | null, kind: CommandKind, direction?: Direction): void {
    button?.addEventListener("click", () => {
      this.sink.sendCommand(kind, direction ?? this.currentDirection());
    });
  }

  private currentDirection(): Direction {
    let x = 0;
    let y = 0;
    for (const code of this.pressed) {
      const direction = keyDirections.get(code);
      if (direction) {
        x += direction.x;
        y += direction.y;
      }
    }
    return {
      x: clampUnit(x),
      y: clampUnit(y)
    };
  }
}

function clampUnit(value: number): -1 | 0 | 1 {
  if (value < 0) {
    return -1;
  }
  if (value > 0) {
    return 1;
  }
  return 0;
}
