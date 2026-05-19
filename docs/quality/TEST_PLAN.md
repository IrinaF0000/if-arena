# Test Plan

## Test categories

### 1. Core simulation tests

- deterministic reference scenario;
- movement validation;
- attack validation;
- cooldown updates;
- objective pickup/drop/capture/score;
- carrier slowdown;
- pickup lock;
- 21x13 map constraints;
- 180-degree rotational symmetry validation;
- player-oriented coordinate transforms;
- tower target selection;
- mine trigger;
- drone/crow movement;
- match end conditions.

### 2. Protocol tests

- encode/decode valid messages;
- split frame across reads;
- multiple frames in one read;
- partial header;
- oversized frame;
- zero-length frame;
- invalid JSON;
- unknown message type;
- missing required fields;
- wrong field types;
- invalid enum values;
- non-finite numeric values.

### 3. Server integration tests

- two clients handshake and join match;
- two clients ready and start match;
- movement command produces snapshot update;
- attack command produces damage event;
- objective pickup produces event;
- carrier hit produces objective drop event;
- pickup lock rejects immediate re-pickup;
- objective capture produces match event;
- invalid ownership command is rejected;
- disconnect during match is handled;
- slow client does not grow memory unbounded;
- spam client is rate-limited.

### 4. Qt client tests/manual checks

Automated Qt tests are optional for early MVP. Manual checklist required:

- connect/disconnect works;
- lobby displays match;
- arena renders own/enemy/hazards/objective;
- own base is shown at bottom for each player;
- WASD movement sends input;
- mouse/Space attack sends input;
- E interacts with objective;
- UI remains responsive on server disconnect;
- errors are visible and non-crashing.

### 5. Load tests

- 100 simulated clients local smoke load;
- 500 simulated clients local target load;
- 1000 simulated clients stretch load where hardware allows;
- slow-client scenario;
- spam-client scenario;
- malformed-input scenario.

### 6. Security tests

- oversized frames rejected before allocation;
- malformed JSON does not crash server;
- unknown message type handled;
- command for another player's unit rejected;
- objective claim from wrong state rejected;
- client-reported objective/carrier state rejected;
- local view transform does not affect server authority;
- rate limit enforced;
- handshake timeout enforced;
- idle timeout enforced.

## Acceptance for MVP

MVP test acceptance requires:

- all core/protocol/server unit and integration tests pass;
- at least one repeatable local load-test report exists;
- hostile input tests pass;
- manual Qt gameplay checklist completed;
- known limitations documented.
