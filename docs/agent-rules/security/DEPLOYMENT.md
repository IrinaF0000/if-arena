# Deployment Security

## Public service requirements

- Bind intentionally and document public ports.
- Use TLS/WSS for WebSocket traffic in public deployment.
- Keep raw TCP game port explicit and bounded.
- Use firewall/security group rules.
- Do not expose admin/debug endpoints publicly.
- Use environment variables or secret stores for secrets.
- Do not commit real secrets.
- Use safe default limits in public config.

## CI/CD

- PR CI must not deploy.
- PR CI must not access production secrets.
- Main CI packaging must not publish unless an explicit release/deploy task is approved.
