# ADR-004: Tests Mirror Source Architecture

Status: Accepted

This ADR was retrospectively recorded on 2026-08-18 from the original decision
evidence. The original decision date is 2026-08-16.

## Context

Automated coverage needs predictable, independently reviewable locations that
co-evolve with the `src/` component boundaries while the implementation remains
transitional. Shared fixtures have separate ownership from component tests.

## Decision

Tests co-evolve with `src/` component boundaries: when a source component is
introduced or reorganized, its tests follow. Shared fixtures remain separate.
The present test directories are examples of this organization, not a fixed
future component list.

## Consequences

- Tests follow source component introduction and reorganization.
- Existing playback evidence remains under `tests/playback/`.
- Shared fixtures remain separate from component-specific tests.

## Explicit non-decisions

This ADR does not reserve or require named future test directories or a fixed
future component list. It does not claim that target components or GUI are
implemented and does not make a product UI decision. It does not decide C23,
macOS, CoreAudio, audio output, loader/domain ownership, or reentrancy.

## Related ASRs

- ASR-004 — Explicit, independently testable component boundaries.

## Evidence

- [`TRACK_002_COMPLETED_compatibility_safeguards.md`](../../.backlog/COMPLETED/2026/TRACK_002_COMPLETED_compatibility_safeguards.md) — records the
  original component-first test organization.
- `cb41a73a8bcf6fae8b4e9045fa57338c54d28f45` (`cb41a73`) — records the
  original component-first layout.
- [`TRACK_004_COMPLETED_architecture_vision_reconciliation.md`](../../.backlog/COMPLETED/2026/TRACK_004_COMPLETED_architecture_vision_reconciliation.md) — records the
  2026-08-17 refinement of the reserved `tui` location to `gui` without
  replacing the component-first organization.
- `bf038032d3e05368ae45df3e8a837199f6c720e8` (`bf03803`) — records the
  later GUI-reserved-location refinement.
