# Qt Quality Rules

## Required

- Keep UI and network/client state separated.
- Use signals/slots to cross UI/network boundaries.
- Do not block the UI thread.
- All server messages must be validated before changing UI state.
- Handle disconnects, reconnect attempts, and user-facing errors gracefully.
- Keep rendering independent from protocol parsing.

## UX requirements

- Keyboard and mouse controls for desktop.
- Clear team colors plus non-color indicators.
- Visible connection state, latency indicator, HP, cooldowns, objective state.
- No authoritative game rules in the client.
