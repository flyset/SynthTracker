# Test Contribution Rules

- Read [`docs/TESTING.md`](../docs/TESTING.md) before adding or moving tests or
  fixtures.
- Co-locate tests with the owned production component as components are
  extracted; name files and fixtures for ownership and observable contracts.
- Use component tests for component contracts, application-level tests for
  workflows/composition, and build/link/executable checks for Main composition.
- Do not use source-text placement checks as behavioral evidence. Structural
  inspection is review support only.
- Keep compatibility fixtures and direct checks bounded and supplemental; every
  behavior change still needs automated coverage.
- Do not treat `src/playback/` or `tests/playback/` as the target product
  architecture. Do not create Application/Main test areas until those
  boundaries are implemented; then place workflow and executable-composition
  evidence at the appropriate boundary.
