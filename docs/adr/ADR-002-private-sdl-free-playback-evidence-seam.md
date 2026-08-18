# ADR-002: Private SDL-Free Playback Evidence Seam

Status: Accepted

This ADR was retrospectively recorded on 2026-08-18 from the original decision
evidence. The original decision date is 2026-08-16.

## Context

The SDL-bound CLI did not provide a small deterministic boundary for automated
playback and compatibility evidence.

## Decision

Maintain a private internal SDL-free playback evidence seam with bounded
load, start, tick, snapshot, caller-owned render, and completion operations.

## Consequences

- Focused tests can exercise playback evidence without SDL runtime or device
  execution.
- The evidence remains fixture-oriented and bounded.
- The seam remains single-global and non-reentrant.

## Explicit non-decisions

This seam is not a public API, MCP contract, or completed reusable playback
core. It does not claim broad format compatibility or bit-identical playback,
and does not decide final loader/domain ownership. It does not decide UI, C23,
macOS, CoreAudio, or Audio Output Port architecture.

## Related ASRs

- ASR-002 — Preservation of documented compatibility semantics.
- ASR-004 — Explicit, independently testable component boundaries (private
  test-boundary evidence only; does not fulfill the target requirement).

## Evidence

- [`TRACK_002_COMPLETED_compatibility_safeguards.md`](../../.backlog/COMPLETED/2026/TRACK_002_COMPLETED_compatibility_safeguards.md) — records the
  bounded private SDL-free seam and its limitation.
- `cb41a73a8bcf6fae8b4e9045fa57338c54d28f45` (`cb41a73`) — introduced the
  context/bridge/mixer/loader fixture seam and SDL-free focused target.
