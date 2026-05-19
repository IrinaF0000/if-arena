# TCP Transport Agent Rules

TCP transport exposes the Qt/CLI/load-client raw TCP endpoint.

## Required behavior

- TCP is a byte stream. Never assume one read equals one message.
- Implement length-prefixed framing with strict `MAX_FRAME_BYTES`.
- Reject oversized frames before allocating payload buffers.
- Handle partial frames, combined frames, EOF, malformed frames, idle timeout, handshake timeout.
- Use bounded per-session incoming and outgoing queues.
- Do not log raw payloads by default.

## Required tests

Add negative tests for:

- split frame across reads;
- multiple frames in one read;
- oversized frame;
- invalid length prefix;
- invalid JSON;
- unknown message type;
- spam client;
- slow reader.

Read also: `.agents/skills/cpp-secure-networking/SKILL.md`.
