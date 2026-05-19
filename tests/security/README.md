# Security Tests

Security tests use fake or hostile clients against protocol and server boundaries.

Required cases:

- oversized frame;
- zero-length frame;
- invalid JSON;
- wrong field type;
- missing required field;
- repeated protocol violations;
- spoofed player or unit ownership;
- command spam;
- slow reader;
- disconnect during partial frame.

Expected result: server rejects, rate-limits, or disconnects safely without crash or unbounded memory growth.
