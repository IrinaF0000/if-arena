# Controls and UI

## Goal

The game should feel fast and understandable while remaining simple enough for an MVP.

Each player should see their own base at the bottom of the screen and the enemy side at the top. Controls are interpreted relative to this local view.

## Qt desktop controls

| Input | Action |
|---|---|
| W/A/S/D | Move relative to local player view |
| Mouse | Aim |
| Left click or Space | Attack |
| Shift | Dash/dodge |
| E | Interact/pick up/capture |
| Esc | Cancel/menu |

Qt also exposes visible connect/disconnect, create, and join controls. Rejected backend commands must be shown in the event log or error surface without changing the rendered authoritative snapshot.

## Telegram Mini App controls

Mobile:

| Input | Action |
|---|---|
| Virtual joystick / D-pad | Move relative to local player view |
| Attack button | Attack |
| Dash button | Dash/dodge |
| Interact button | Pick up/capture/use objective |

Desktop Telegram fallback:

| Input | Action |
|---|---|
| W/A/S/D | Move relative to local player view |
| Mouse | Aim |
| Click/Space | Attack |
| Shift | Dash/dodge |
| E | Interact |

## UI requirements

Both clients must show:

- connection state;
- own/enemy identity;
- own HP;
- cooldowns;
- current score;
- objective state: center, carried, dropped, captured;
- carrier indicator;
- match timer;
- reconnect/disconnect state;
- win/loss result.

## Readability

Use more than color:

- own unit blue/cyan in player view;
- enemy unit red in player view;
- neutral hazards use distinct shapes/icons;
- selected/controlled hero has outline;
- HP bar;
- objective glow/label;
- brief event messages for pickup, drop, and capture.

## Forbidden

- Do not hide critical state only in logs.
- Do not require reading long rules before first match.
- Do not let UI imply an action is possible if backend rejected it without feedback.
- Do not let local orientation leak into server authority.
