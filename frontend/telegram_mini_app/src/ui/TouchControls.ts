export type Direction = {
  x: number;
  y: number;
};

export class TouchControls {
  public getCurrentDirection(): Direction {
    return { x: 0, y: 0 };
  }
}
