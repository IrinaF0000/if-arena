# Load Tests

This directory contains repeatable load-test scenarios and expected report formats.

Required scenarios:

- 20-client smoke load;
- 100-client baseline gameplay load;
- slow-reader scenario;
- spam-client scenario;
- malformed-client scenario.

Long-running load tests do not need to run in every CI job, but the smoke load should be easy to run locally.
