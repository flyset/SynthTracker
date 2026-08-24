# ADR-010: Native Adapter Ownership and Private Demand Coordination

Status: Accepted

Decision date: 2026-08-23.

## Context

ADR-009 formalized the callback-driven render model and its private
responsibility allocation: CoreAudio owns render timing; a private render
boundary produces every requested Audio Frame Block on demand; adapter-private
conversion stays inside the adapter; and no public Audio Output Port, C API,
header, or export is introduced. That decision deliberately left open where the
coordination between a native device callback and the private renderer lives.

Track 015 — DRAFT when this decision was recorded, ACTIVE since 2026-08-23 —
scopes the private CoreAudio live route: the route must become live and audible
while the target Audio Output Port and any final Mixer extraction remain out of
scope. A decision is therefore needed about who owns
the native device callback, device lifecycle, and device-format conversion, and
which private component synchronously coordinates a device demand with the
renderer that fulfills it.

## Decision

Approve the private responsibility allocation for the future private Track 015
CoreAudio live route:

- **Native audio adapters own native device concerns.** The CoreAudio adapter
  owns its device callbacks, device lifecycle (open/start/stop/close), and
  device-format conversion from Audio Frame Blocks to the adapter-private
  interleaved Float32 representation.
- **Private `audio_output` owns the synchronous device-demand coordinator.**
  For each adapter request of exactly N frames, `audio_output` requests exactly
  N Audio Frame Block frames from the private renderer and immediately routes
  the result to the requesting adapter. The request is satisfied synchronously;
  no block is buffered, delayed, or pushed on a software cadence.
- **`audio_output` contains no software queue, timer, or CoreAudio code.** Its
  coordinator role is device-independent and synchronous; platform concerns stay
  in the adapters.
- **No public API, Audio Output Port, header, or export is introduced**, and no
  final Mixer extraction is introduced by this decision. Everything here is
  private composition under the ADR-005/ASR-006 audio-output direction.
- This decision **refines ADR-009's private responsibility allocation**; it
  preserves all of ADR-009's device-clock, on-demand, real-time, active-rate,
  and strict-stereo policies without altering them.

## Deferrals

This decision does not fix the CoreAudio API or audio unit choice; the concrete
device lifecycle, negotiation, error, change, and restart mechanics (including
the concrete reconfiguration/restart policy for a device sample-rate change —
the quiescence-before-reconfiguration boundary itself is decided by ADR-009,
see Decision and Consequences); event handoff, clock, and overflow semantics;
the concrete private function/state integration of the legacy renderer
adaptation to the callback-driven boundary (the adaptation direction itself is
decided by ADR-009, see Decision and Consequences); or actual live MIDI or
input implementation. The public Audio Output Port and Mixer extraction also
remain deferred. Ownership of the private coordinator is decided here; Track
015 S5.1 has since implemented and component-tested the coordinator's
device-free exact-N policy, S5.2 has since implemented and
component-tested the device-free private adapter-owned bound instance with its
adapter-owned request entry routing active requests exact-N through the
existing coordinator to a non-converting sink with payload identity, S5.3
has since implemented and component-tested the device-free fake
deferred-stop-to-quiescence, and S5.4 has since implemented and
component-tested the device-free borrowed preallocated conversion workspace,
and S5.5 has since implemented and component-tested the device-free
workspace-mode startup rejection of invalid preparation, while its
device-driven wiring has since been delivered (see "Implementation evidence
and status" below) and the remaining
mechanics stay deferred to later work.

## Consequences

- Ownership is decided at the responsibility level: native adapters own native
  device callbacks, lifecycle, and device-format conversion, and the private
  `audio_output` component owns the synchronous device-demand coordinator.
  Track 015 S5.1 has implemented and component-tested the coordinator's
  device-free exact-N policy, S5.2 has implemented and component-tested the
  device-free private adapter-owned bound instance with its adapter-owned
  request entry routing active requests exact-N through the existing
  coordinator to a non-converting sink with payload identity, S5.3 has
  implemented and component-tested the device-free fake
  deferred-stop-to-quiescence, and S5.4 has implemented and component-tested
  the device-free borrowed preallocated conversion workspace, and S5.5 has
  implemented and component-tested the device-free workspace-mode startup
  rejection of invalid preparation; the
  device-driven wiring has since been delivered (see "Implementation evidence
  and status" below) and the remaining mechanics
  remain open.
- Each adapter request of exactly N frames is answered synchronously with
  exactly N Audio Frame Block frames routed immediately to the requesting
  adapter; the route introduces no software queue, timer, or CoreAudio code in
  `audio_output`.
- This decision preserves ADR-009's device-clock, on-demand, real-time,
  active-rate, and strict-stereo policies unchanged; it only assigns the private
  responsibilities between the adapter and the coordinator.
- No public Audio Output Port, C API, header, or export results, and no final
  Mixer extraction is introduced; the CoreAudio route remains private
  composition under the ADR-005/ASR-006 audio-output direction.
- During Phase 4, this decision does not alter current SDL-era TFMX behavior and
  creates no SynthTracker v1 compatibility promise. At the time of these
  device-free evidence notes, the private
  CoreAudio adapter was device-free and the production default façade was
  unavailable; Track 015 is ACTIVE, S5.1 has implemented and component-tested
  a device-free private synchronous exact-N coordinator (it requests exactly N
  frames from the renderer, rejects count mismatch or nonzero NULL payload
  without delivery, padding, or truncation, and otherwise delivers the same
  borrowed block once), S5.2 has implemented and component-tested the
  device-free private adapter-owned bound instance — strict format and complete
  route, open→configure→bind request→start with failure short-circuits before
  activation, active only after a successful start, and active fake 0/1/3/2
  requests routed exact-N through the existing coordinator to a non-converting
  sink with payload identity, while failed or inactive requests reach neither
  renderer nor sink — S5.3 has implemented and component-tested the
  device-free fake deferred-stop-to-quiescence: a stop requested during an
  active N=3 fake request lets that exact-N delivery complete exactly once with
  payload identity, then the instance transitions active→stop-requested→
  quiescent, the fake source quiesces, and later fake source requests are
  refused at the source before any callback/render/sink work — and S5.4 has
  implemented and component-tested the device-free borrowed preallocated
  conversion workspace: a 0-frame request is accepted with no observer
  delivery, 1- and 3-frame requests convert signed-32 left/right to interleaved
  Float32 in place into the supplied capacity-3 borrowed Float32 workspace with
  payload pointer identity and zero allocation, an oversize 4-frame request is
  rejected before observer/write, and the old stateless submit path remains
  separate and allocating. S5.5 has implemented and component-tested the
  device-free workspace-mode startup rejection: NULL samples, zero capacity,
  and capacity greater than `SIZE_MAX / 2` are rejected before any facade
  lifecycle, bind/request/render/observer, or allocation work, the instance
  stays INACTIVE with no in-request state, and the direct S5.2 route remains
  valid. The S4 stateless
  start is unchanged; no real device/framework stop/close, threads/locks,
  workspace release/close, legacy renderer/module teardown, application/SDL/CLI,
  public API/port, queue/timer, rate/restart, or real live route work was
  added in those device-free chunks; the real HAL Output Audio Unit route,
  callback/render integration, and application/SDL/CLI retirement have since
  been delivered (see "Implementation evidence and status" below). Invalid
  storage-length proof, workspace release/close, the device-rate-change
  restart policy, and
  the public Audio Output Port remain deferred.

## Implementation evidence and status

This decision remains **Accepted** and authoritative for the delivered private
Track 015 responsibility split. Implementation evidence, recorded 2026-08-24:

- **S5 delivered the device-driven route under this allocation.** Native
  adapter ownership (device callbacks, lifecycle, and device-format
  conversion) and the private `audio_output` synchronous exact-N coordinator
  are implemented as approved: the real legacy exact-N renderer, the private
  CoreAudio adapter lifecycle/callback with control-side quiescence and the
  lock-free admission gate, workspace-only conversion with the HAL
  workspace-copy route, the real HAL Output Audio Unit facade (strict
  44.1/48 kHz negotiated-rate gate, no resampler, `UNAVAILABLE` elsewhere),
  and the application cutover with SDL/`-o`/CLI retirement. On this
  workspace/HAL route a zero-frame request is accepted at the adapter's
  admission gate without invoking the renderer or the exact-N coordinator;
  the coordinator is invoked for each nonzero exact-N request as decided.
- **S6.2–S6.4b delivered the negotiated-rate preparation handoff.** The
  private rate-0 startup-selection sentinel with a facade-verified 44.1/48 kHz
  configured rate and control-side renderer preparation before bind/start, and
  the private legacy producer's blend-then-widen normalization (blended int32
  lanes reconstructed to their low-16 historical PCM-domain value and
  multiplied by 65536 into the
  existing signed-32 Audio Frame Blocks; the generic adapter conversion
  remains signed-32/2^31). `voices_01` is the self-authored supplemental
  audible-smoke fixture; a listener confirmed real 48 kHz `voices_01`
  playback.
- **Validation evidence.** Full CTest 8/8; `test_audio_output` 36/36;
  `test_legacy_exact_renderer` 8 fixture/rate cases at 44.1/48 kHz with the
  exact `voices_01` Float32 oracle (L=1680/32768, R=1392/32768);
  application-level proof of the 48 kHz rate-0 workspace route and
  removed-option rejection; supplemental real-device smoke checks on macOS
  (including `voices_01` at 48 kHz).
- **S6.5 is complete; S7 completed the Track.** Track 015 S6.5 (final
  validation and documentation) was checked complete: the full CTest suite
  passes 8/8, the listener-confirmed real 48 kHz `voices_01` smoke was
  recorded, the required documentation was reconciled, and the Phase 4 roadmap
  was revised (Phase 4 revision 14; canonical index revision 21). S7
  (completion) is checked: Track 015 is completed on 2026-08-24 at
  `.backlog/COMPLETED/2026/TRACK_015_COMPLETED_coreaudio_live_route_and_sdl_retirement.md`,
  and Stage 3 remains in progress pending its wider acceptance/merge.
- **Preserved deferrals.** The device-rate-change restart policy, workspace
  release/close, invalid storage-length proof, live MIDI/input, the public
  Audio Output Port, the target `Mixer`, broader event/clock/overflow policy,
  and general real-module loader expansion remain deferred (no format-wide
  loader compatibility promise after the recorded XOut2 rejection). The
  coordinator function (`audio_output_coordinate_frame_request`) is
  device-independent as decided; the private `audio_output` translation unit
  still includes `audio_output_dispatch_submit`, which calls the CoreAudio
  adapter dispatch on Apple platforms — removing that dispatch so that the
  whole component is CoreAudio-free remains an outstanding private
  refactor/deferral.

## Related ASRs

- ASR-010 — Callback-driven real-time audio rendering: the adapter ownership
  and coordinator responsibilities this decision assigns satisfy ASR-010's
  responsibility allocation.
- ASR-006 — Isolated future audio output: the CoreAudio adapter is the intended
  platform-specific adapter behind the device-independent Audio Output Port
  direction; the private `audio_output` coordinator is not that port.
- ASR-009 — Audio Frame Block Boundary Invariants: the coordinator requests
  Audio Frame Block frames that satisfy the ASR-009 invariants.

## Evidence

- [`ADR-009`](ADR-009-callback-driven-audio-rendering.md) — the callback-driven
  decision this record refines; the private responsibility allocation and all
  preserved device-clock/on-demand/real-time/rate/stereo policies originate
  there.
- [`ASR.md`](../ASR.md) — records ASR-010, ASR-009, and ASR-006.
- [`AUDIO_RENDERING_DESIGN.md`](../AUDIO_RENDERING_DESIGN.md) — plain-language
  component flow, real-time contract, current status, and open deferrals.
- [`TRACK_015_COMPLETED_coreaudio_live_route_and_sdl_retirement.md`](../../.backlog/COMPLETED/2026/TRACK_015_COMPLETED_coreaudio_live_route_and_sdl_retirement.md)
  — Track 015 is COMPLETED: S4 completed the injected private-façade lifecycle
  seam, S5.1 implemented and component-tested the device-free exact-N
  coordinator, S5.2 implemented and component-tested the device-free
  adapter-owned bound instance with its adapter-owned request entry, S5.3
  implemented and component-tested the device-free fake
  deferred-stop-to-quiescence, S5.4 implemented and component-tested the
  device-free borrowed preallocated conversion workspace, and S5.5 implemented
  and component-tested the device-free workspace-mode startup rejection of
  invalid preparation. This
  record formalizes the Track's approved ADR-010 private
  demand coordination decision. S5 delivered the device-driven route and
  S6.2–S6.4b the rate-0 startup selection, control-side preparation, and
  legacy normalization; S6.5 validation and documentation are complete and S7
  completed the Track on 2026-08-24 — see "Implementation evidence and
  status" above.
- [`TRACK_014_COMPLETED_coreaudio_adapter.md`](../../.backlog/COMPLETED/2026/TRACK_014_COMPLETED_coreaudio_adapter.md)
  — completed Track that delivered the private device-free conversion adapter
  the future live route builds on.
