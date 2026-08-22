# ADR-007: Audio Frame Block Boundary and Fixed First Format

Status: Superseded

Decision date: 2026-08-20.

Superseded by: [ADR-008 — Audio Frame Block Mixed-Value Boundary](ADR-008-audio-frame-block-mixed-value-boundary.md), accepted 2026-08-22. Retained as the historical record of the first fixed format; the mixed-value boundary is now authoritative.

## Context

ADR-005 and ASR-006 establish the direction from `Mixer` to a
device-independent `Audio Output`, but defer the concrete artifacts and
contracts at that boundary. The same rendered audio must also be usable by
`File I/O` for rendered-audio export without making either consumer define the
rendering boundary.

## Decision

Name **Audio Frame Block** as the boundary artifact from `Mixer` to `Audio
Output` and from rendered audio to `File I/O` for rendered-audio export. The
first contract fixes its format as 44.1 kHz, stereo, interleaved PCM, signed
16-bit, little-endian.

An Audio Frame Block is a finite, ordered block of rendered audio. A frame is
all channel samples at one sample instant. The block payload is not SDL-native,
CoreAudio-native, or device-native; platform adapters consume the boundary
artifact rather than defining it.

This decision establishes the artifact and its initial representation without
establishing an API or implementation. Format negotiation and conversion
beyond this first contract are deferred. Ownership and lifetime,
timing/order/queueing/backpressure, API and concrete types, adapter behavior,
and implementation are also explicitly deferred to later work.

## Consequences

- `Mixer` has a named rendered-audio artifact to produce, and `Audio Output` and
  rendered-audio export have a shared boundary vocabulary.
- The first contract is deterministic for component tests and export/output
  boundary design: the initial representation is fixed rather than negotiated.
- The payload remains independent of SDL, CoreAudio, and other device-native
  representations.
- This decision does not define ownership, lifetime, timing, ordering,
  queueing, backpressure, API/types, adapter behavior, or implementation.
- Format negotiation or conversion beyond the fixed first format remains open.
- During Phase 4, this target artifact does not alter current SDL-era TFMX
  behavior or create a SynthTracker v1 compatibility promise.

## Related ASRs

- ASR-003 — UI-agnostic playback core.
- ASR-004 — Explicit, independently testable component boundaries.
- ASR-006 — Isolated future audio output.
- ASR-009 — Audio Frame Block Boundary Invariants.

## Evidence

- [`ADR-005`](ADR-005-target-daw-component-foundation.md) — establishes the
  target component foundation and device-independent `Audio Output` direction.
- [`ASR.md`](../ASR.md) — records the related architectural requirements,
  including ASR-006 and ASR-009.
- [`ARTIFACTS.md`](../ARTIFACTS.md) — catalogs the Audio Frame Block artifact and
  its deferred contracts.
- [`ARCHITECTURE.md`](../ARCHITECTURE.md) — records the target flow from
  `Mixer` to `Audio Output` or `File I/O`.
- [`TRACK_007_COMPLETED_target_architectural_foundation.md`](../../.backlog/COMPLETED/2026/TRACK_007_COMPLETED_target_architectural_foundation.md)
  — completed Track evidence for the approved target foundation.
