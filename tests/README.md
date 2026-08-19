# Tests

Test locations mirror current ownership and will evolve as production
components are extracted. See [`../docs/TESTING.md`](../docs/TESTING.md) for the
canonical strategy.

## Current areas

- [`playback/`](playback/) contains CMocka tests for the private, SDL-free
  playback seam: loading, validation, start/tick behavior, snapshots,
  rendering, completion, and bounded fixture compatibility evidence.
- [`fixtures/`](fixtures/) contains self-authored valid and malformed TFMX
  fixture pairs plus layout notes used by playback evidence. These are bounded
  compatibility fixtures, not a format-wide compatibility claim.
- [`application/`](application/) contains the current source-level Application
  boundary tests. `test_application.c` directly exercises the non-root
  `application_run` path for missing arguments and invalid options, verifying
  status `2` and `Usage:` output on standard error.
The private `src/playback/` and `tests/playback/` areas are temporary
compatibility evidence, not a declaration of target product architecture.

## Direction as components are extracted

New tests should be named and located by the owned component and its observable
contract. Component tests cover component behavior; application-level tests
cover workflows and composition. The source-level Application seam now exists,
but its current tests are limited to bounded non-root usage/error status and
output behavior; the future target architecture remains unimplemented beyond
this structural seam. Main is covered by compile/link/executable integration
checks. Main should remain minimal; do not add source-text existence or
placement tests for it.
