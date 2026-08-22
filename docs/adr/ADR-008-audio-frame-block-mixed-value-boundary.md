# ADR-008: Audio Frame Block Mixed-Value Boundary

Status: Accepted

Decision date: 2026-08-22.

## Context

ADR-007 and ASR-009 fixed an initial signed 16-bit little-endian PCM byte
payload for the Audio Frame Block before the user clarified the intended
boundary: the block carries raw left/right mix-accumulator values rather than
serialized PCM. The fixed PCM byte format therefore reflects a boundary that
was clarified later and does not match the intended representation.

Track 012 implemented private validation of that fixed PCM format and a
synchronous null adapter; that work stands as historical private evidence of
the ADR-007-era assumption, not as the canonical representation. Track 013
(ACTIVE) planned to route legacy final PCM under the same assumption; its
provisional S2 decision already records the conflict with the clarified
boundary.

## Decision

Name the **Audio Frame Block** as the boundary artifact emitted by the
`Mixer`, carrying the left/right signed-32 mix-accumulator values as the
initial canonical representation.

- The block contains raw mixed-value data, not serialized PCM and not
  device-native data. Platform adapters consume the boundary artifact rather
  than defining it.
- A future `Mixer` emits the same Audio Frame Blocks and may perform additional
  internal mixing, effects, or processing before emitting them; the exact
  internal processing point inside a future `Mixer` remains open and is not
  fixed by this decision.
- The temporary legacy bridge (the current CLI live route) cuts immediately
  after the current legacy renderer combines voices; its legacy stereo blend,
  low-pass filtering, and PCM byte packing are intentionally absent from that
  temporary live route, remain outside this boundary, and must not be moved to
  Audio Output.
- Future representation changes require an explicit, versioned decision; the
  initial canonical representation is not an unversioned implementation detail.
- Audio Output and its CoreAudio adapter perform destination format adaptation:
  conversion to the device's required format is an adapter responsibility.
- File I/O remains the separate direct consumer defined by ADR-005 and
  serializes output for rendered-file export; it does not define the rendering
  boundary.

This decision supersedes ADR-007, which is retained as a historical record.

## Deferrals

This decision does not fix the concrete C API or layout, the precise internal
processing point inside a future `Mixer`, numerical range/overflow/clip
behavior, ownership/lifetime, sample-rate/channel metadata,
timing/order/queueing/backpressure, or adapter capability/format policy. Each
remains deferred to later work.

## Consequences

- The Audio Frame Block carries raw mixed-value data at the boundary, so
  rendering and serialization/adaptation are separated: serialization to PCM
  happens in adapters and in File I/O, not in the block.
- The boundary contract fixes only what crosses the boundary, so a future
  `Mixer` may perform additional internal mixing, effects, or processing before
  emitting Audio Frame Blocks; the exact internal processing point is not a
  boundary concern.
- The temporary legacy bridge (the current CLI live route) cuts immediately
  after the current legacy renderer combines voices. Its legacy stereo blend,
  low-pass filtering, and PCM byte packing are intentionally absent from that
  temporary live route, stay in the legacy renderer path, and must not be
  relocated into Audio Output.
- Track 012 is historical private fixed-PCM evidence: its private validation
  and null adapter remain valid as evidence of the ADR-007-era design, but do
  not bind the canonical representation.
- Track 013's PCM route must be re-scoped before implementation. Its planned
  legacy final-PCM packaging and routing do not match the mixed-value boundary
  and cannot proceed under the superseded ADR-007 assumption.
- Audio Output and CoreAudio adapters own destination format adaptation;
  File I/O serializes output for rendered-file export as the separate direct
  consumer defined by ADR-005.
- Future representation changes require an explicit, versioned decision, so
  the boundary representation cannot drift silently.
- During Phase 4, this decision does not alter current SDL-era TFMX behavior
  or create a SynthTracker v1 compatibility promise.

## ASR-009 revision

As part of this decision, ASR-009's requirement was revised to replace the
fixed four-byte/payload invariants (exact `frame_count × 4` bytes, distinct
missing-payload and incorrect-length failures) with generic mixed-value block
invariants: an ordered sequence of zero or more frames, each frame carrying
signed-32 left and right mix values emitted by the `Mixer` without fixing the
precise internal creation point, with no device-native or serialized PCM
representation. The revision is recorded in `ASR.md` under the ASR register
process; the concrete layout, validation rules, and the Mixer's internal
processing point remain deferred as stated above.

## Related ASRs

- ASR-003 — UI-agnostic playback core.
- ASR-004 — Explicit, independently testable component boundaries.
- ASR-006 — Isolated future audio output.
- ASR-007 — Explicit filesystem and shell boundaries.
- ASR-009 — Audio Frame Block Boundary Invariants (revised by this decision).

## Evidence

- [`ADR-007`](ADR-007-audio-frame-block-boundary-and-fixed-first-format.md) —
  the superseded record; it fixed the initial s16le PCM byte payload that
  preceded the clarified boundary.
- [`ADR-005`](ADR-005-target-daw-component-foundation.md) — establishes the
  target component foundation, the device-independent `Audio Output` direction,
  and `File I/O` as the rendered-audio export consumer.
- [`ASR.md`](../ASR.md) — records the related architectural requirements,
  including ASR-006, ASR-007, and the current ASR-009 invariants.
- [`ARTIFACTS.md`](../ARTIFACTS.md) — catalogs the Audio Frame Block artifact
  and its deferred contracts.
- [`ARCHITECTURE.md`](../ARCHITECTURE.md) — records the target flow from
  `Mixer` to `Audio Output` or `File I/O`.
- [`TRACK_012_COMPLETED_audio_output_null_adapter.md`](../../.backlog/COMPLETED/2026/TRACK_012_COMPLETED_audio_output_null_adapter.md)
  — completed Track implementing private fixed-PCM validation; historical
  evidence of the ADR-007-era design.
- [`TRACK_013_ACTIVE_legacy_pcm_audio_output_routing.md`](../../.backlog/ACTIVE/2026/TRACK_013_ACTIVE_legacy_pcm_audio_output_routing.md)
  — ACTIVE Track whose PCM route must be re-scoped before implementation.
