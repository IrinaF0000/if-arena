export type Direction = {
  x: number;
  y: number;
};

export type TouchAction = "attack" | "dash" | "interact";

export class TouchControls {
  private lastAction: TouchAction | null = null;

  public getCurrentDirection(): Direction {
    return { x: 0, y: 0 };
  }

  public recordAction(action: TouchAction): void {
    this.lastAction = action;
  }

  public consumeLastAction(): TouchAction | null {
    const action = this.lastAction;
    this.lastAction = null;
    return action;
  }
}
