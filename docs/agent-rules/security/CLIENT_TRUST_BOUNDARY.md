# Client Trust Boundary

Clients are presentation and input devices, not authorities.

## Clients may send

- movement intent;
- aim/attack intent;
- interact/capture intent;
- ready/join/lobby intent;
- ping/latency data.

## Clients must not control

- player id ownership;
- unit ownership;
- HP, damage, cooldowns, score;
- objective state;
- match tick state;
- hazard behavior;
- server time.

The backend validates every accepted command against current authoritative state.
