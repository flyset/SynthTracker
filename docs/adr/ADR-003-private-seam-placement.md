# ADR-003: Private Seam Placement

Status: Accepted

This ADR was retrospectively recorded on 2026-08-18 from the original decision
evidence. The original decision date is 2026-08-17.

## Context

The private playback seam needed component-aligned placement without changing
its observed behavior or turning its test boundary into a finalized domain
contract.

## Decision

Place the private playback context, narrow loader, legacy bridge, and SDL-free
legacy mixer under `src/playback/`.

## Consequences

- The seam has an explicit internal source and build location.
- Existing behavior and SDL-free test linkage are preserved.
- Placement communicates internal playback responsibility without finalizing
  domain ownership.

## Explicit non-decisions

This ADR does not decide final loader/domain/playback ownership, or make the
seam public or reentrant. It does not decide GUI, Sequencing, Synthesis,
CoreAudio, or Audio Output Port boundaries.

## Related ASRs

- ASR-004 — Explicit, independently testable component boundaries.

## Evidence

- [`TRACK_003_COMPLETED_playback_seam_architecture.md`](../../.backlog/COMPLETED/2026/TRACK_003_COMPLETED_playback_seam_architecture.md) — records the
  behavior-preserving placement decision and its unresolved ownership
  boundary.
- `c9a6a3d55c6fe69148af51c2d3e85e44f048f7df` (`c9a6a3d`) — records the
  implementation of the private seam placement under `src/playback/`.
