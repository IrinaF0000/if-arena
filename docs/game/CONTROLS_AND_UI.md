# Controls and UI

## Goal

The game should feel fast and understandable while remaining simple enough for an MVP. Each player sees their own base at the bottom of the screen and the enemy side at the top. Controls are interpreted relative to this local view.

## Qt desktop controls

| Input | Action |
|---|---|
| W/A/S/D or arrow keys | Move relative to local player view |
| Mouse movement | Aim only; must not move, dash, attack, teleport, or spam gameplay commands |
| Left mouse button or Space | Attack toward aim direction |
| Right mouse button or Shift | Dash/dodge toward aim direction, falling back to movement direction |
| Esc | Cancel/menu/disconnect surface |

Objective pickup and capture are automatic server-side rules. Do not require an `interact` key for the MVP objective flow.

## Input quality rules

- Keyboard movement is the only player movement input.
- Mouse movement may update aim, cursor highlight, or local preview only.
- Qt keeps mouse aim local for the MVP; if a client later sends aim updates to the server, they must be throttled and meaningful.
- Idle clients must not repeatedly send or log no-op/`none` input commands.
- After joining a match, gameplay keys must work without focus staying trapped in the match-code field.
- Rejected backend commands must be shown in the event log or error surface without changing the rendered authoritative snapshot.

## Telegram Mini App controls

Mobile:

| Input | Action |
|---|---|
| Virtual joystick / D-pad | Move relative to local player view |
| Aim drag / target direction, if present | Aim only |
| Attack button | Attack |
| Dash button | Dash/dodge |

Desktop Telegram fallback:

| Input | Action |
|---|---|
| W/A/S/D or arrow keys | Move relative to local player view |
| Mouse movement | Aim only |
| Click/Space | Attack |
| Right click/Shift | Dash/dodge |

## UI requirements

Both clients must show:

- connection state;
- own/enemy identity;
- own HP if HP affects combat;
- cooldowns or unavailable feedback for attack/dash;
- current score;
- objective state: center, carried, dropped, captured;
- carrier indicator;
- match timer or tick/age in debug builds;
- reconnect/disconnect state;
- win/loss result;
- compact controls hint.

## Readability

Use more than color:

- own unit blue/cyan in player view;
- enemy unit red in player view;
- neutral hazards use distinct shapes/icons;
- selected/controlled hero has outline;
- objective glow/label;
- carrier marker/halo;
- attack range or direction preview for the local player;
- brief event messages for pickup, drop, hit, dash, and capture.

## Forbidden

- Do not hide critical state only in logs.
- Do not require reading long rules before first match.
- Do not let UI imply an action is possible if backend rejected it without feedback.
- Do not let local orientation leak into server authority.
- Do not let mouse movement alone move or trigger the player.
- Do not show unexplained player sticks/bars in normal play.
