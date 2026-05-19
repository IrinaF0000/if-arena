# ADR 0002: Use Length-Prefixed JSON over TCP for MVP

## Status

Accepted.

## Context

The project needs a custom protocol over TCP. TCP is a byte stream and does not preserve application-level message boundaries. The protocol must therefore define framing.

Options considered:

- newline-delimited JSON
- length-prefixed JSON
- custom binary protocol
- existing protocol such as protobuf

## Decision

Use length-prefixed JSON for MVP.

Frame format:

```text
uint32 length in network byte order
UTF-8 JSON payload
```

## Consequences

### Positive

- demonstrates explicit TCP message framing
- easy to debug with logs
- easy to document
- easy to use from both C++ server and Qt client
- avoids premature binary protocol complexity

### Negative

- larger payloads than binary protocol
- parsing overhead
- schema is less strict unless validation is implemented carefully

## Future options

After MVP, the project can add:

- binary protocol
- protobuf/flatbuffers
- delta snapshots
- compression for large snapshots

The MVP keeps JSON because portfolio reviewers can understand it quickly.
