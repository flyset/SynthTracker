# Audio Rendering Design

This document explains the approved callback-driven audio rendering direction
for live output in plain terminology: the component flow, the distinction
between events and pre-rendered audio, the real-time contract, the current
status, the evidence direction, and the open deferrals.

**Status: approved direction, implemented as the private Track 015 live
route.** The decision is recorded in
[ADR-009](adr/ADR-009-callback-driven-audio-rendering.md)
(Accepted, 2026-08-22), with its private responsibility allocation refined by
[ADR-010](adr/ADR-010-native-adapter-ownership-and-private-demand-coordination.md)
(Accepted, 2026-08-23; see "Private demand coordination"). Track 015 S5
delivered the private device-driven route: the real legacy exact-N renderer
(`src/playback/playback_legacy_renderer.c`), the private CoreAudio adapter
lifecycle/callback with control-side quiescence and the lock-free admission
gate, workspace-only conversion with the HAL workspace-copy route, the real
HAL Output Audio Unit facade with a strict 44.1/48 kHz negotiated-rate gate
and no resampler, and the application cutover with the SDL live-audio path,
the `-o` file-output path, and the `-b`, `-8`, `-f`, `-o`, `-w`, and `-v`
options deleted. Track 015 S6.2–S6.4b delivered the private rate-0
startup-selection sentinel with a verified 44.1/48 kHz configured rate and
control-side renderer preparation before bind/start, and the private legacy
producer's blend-then-widen normalization (blended int32 lanes reconstructed
to their low-16 historical PCM-domain value and multiplied by 65536 into the
existing signed-32
Audio Frame Blocks; the generic adapter conversion remains signed-32/2^31);
`voices_01` is the self-authored supplemental audible-smoke fixture, and a
listener confirmed real 48 kHz `voices_01` playback. The earlier device-free
S4/S5.1–S5.5 coordination, bound-instance, quiescence, borrowed-workspace,
and startup-rejection evidence remains as the deterministic test foundation;
  the full CTest suite passes 8/8. Track 015 S6.5 (final validation and
  documentation) is complete: the required documentation was reconciled, the
  Phase 4 roadmap was revised (Phase 4 revision 14; canonical index revision
  21), and the listener-confirmed real 48 kHz `voices_01` smoke was recorded.
  S7 (completion) is checked: Track 015 is completed at
  `.backlog/COMPLETED/2026/TRACK_015_COMPLETED_coreaudio_live_route_and_sdl_retirement.md`,
  while Stage 3 remains in progress pending its wider acceptance/merge. The public Audio Output
Port, the target `Mixer`, the device-rate-change restart policy, workspace
release/close, invalid storage-length proof, live MIDI/input, broader
event/clock policy, and general real-module loader expansion remain deferred;
this design promises neither current behavior nor a format-wide compatibility
promise.

## Plain-language overview

In the historical legacy path, the engine rendered audio on its own schedule and
handed it to an SDL-era ring buffer that the device drained whenever it wanted.
That push model is the retired context this direction replaces; the current
private CLI live route is device-driven: the macOS HAL Output Audio Unit render
callback asks for audio, and the private legacy exact-N renderer produces
exactly the requested block, as described under "Current status". In this
direction the audio device is the clock. The device asks for
a block of audio, and a private renderer produces exactly that block — the
requested number of frames — using the current playback state and a stream of
time-ordered events. If the device asks for a variable count, the renderer
answers with exactly that count. On the implemented workspace/HAL route a
zero-frame request is accepted at the adapter's admission gate without
invoking the renderer or the exact-N coordinator; the broader approved
direction covers zero-frame requests as answered on-demand blocks. Nothing is
rendered ahead of
time and pushed; every block is rendered on demand.

CoreAudio owns render timing; the software owns the content. The renderer never
waits on, or interferes with, the device's clock.

## Component flow

```text
current/future playback state ───────────────┐
                                              ▼
time-ordered events (player; later live       │
input) ────────────────────────────────►  private render boundary
                                              │  produces, on each device request,
                                              ▼  exactly the requested frame count
                                 Audio Frame Block (signed-32 {left, right})
                                              │
                                              ▼
                              adapter-private conversion (to interleaved Float32)
                                              │
                                              ▼
                                    CoreAudio device (macOS)
```

- **Playback state and time-ordered events** are the renderer's inputs. They
  describe what should sound and when; they are not audio.
- The **private render boundary** is where a device request is answered: each
  nonzero request is rendered on demand with exactly the requested frame
  count. On the implemented workspace/HAL route a zero-frame request is
  accepted at the adapter without invoking the renderer or the exact-N
  coordinator; the approved direction answers zero-frame requests as
  on-demand blocks.
- The **Audio Frame Block** is the rendered output artifact (ADR-008 / ASR-009):
  raw signed-32 left/right mix values, never serialized PCM and never
  device-native data.
- **Adapter-private conversion** (the Track 014 signed-32 to interleaved
  Float32 mapping) stays inside the adapter, on the device side of the
  boundary. It is not part of the render boundary contract.
- No public Audio Output Port, C API, header, or export exists for this route;
  everything here is private composition.

## Private demand coordination

Native audio adapters own their device callbacks, device lifecycle, and
device-format conversion. Private `audio_output` owns the synchronous
device-demand coordinator: for each adapter request of exactly N frames, it
requests exactly N Audio Frame Block frames from the private renderer and
immediately routes the result to the requesting adapter. The coordinator
function (`audio_output_coordinate_frame_request`) contains no software queue
or timer and is device-independent, and it is not the public
Audio Output Port — that port remains deferred. The private `audio_output`
translation unit still includes `audio_output_dispatch_submit`, which calls
the CoreAudio adapter dispatch on Apple platforms; removing that dispatch is
an outstanding private refactor/deferral. On the workspace/HAL route a
zero-frame request is accepted at the adapter's admission gate before the
coordinator is invoked. This responsibility allocation
is decided by
[ADR-010](adr/ADR-010-native-adapter-ownership-and-private-demand-coordination.md)
(Accepted, 2026-08-23), which refines ADR-009's; ownership is now decided, and
Track 015 S5.1 has implemented and component-tested the device-free exact-N
coordinator policy — it requests exactly N frames from the renderer, rejects a
returned block whose frame count differs from N or a nonzero block with NULL
frame storage without delivery, padding, or truncation, and otherwise delivers
the same borrowed block once. Track 015 S5.2 has since implemented and
component-tested the device-free private adapter-owned bound instance: the
adapter-owned request entry now exists and, once the instance is active, routes
exact-N requests through the existing coordinator to a non-converting sink with
payload identity, with failure short-circuits before activation. Track 015 S5.3
has since implemented and component-tested the device-free fake
deferred-stop-to-quiescence: a stop requested during an active N=3 fake request
  lets that exact-N delivery complete exactly once with payload identity, then
  the instance transitions active→stop-requested→quiescent, the fake source
  quiesces, and later fake source requests are refused at the source before any
  callback/render/sink work; and Track 015 S5.4 has since implemented and
  component-tested the device-free borrowed preallocated conversion workspace: a
  0-frame request is accepted with no observer delivery, 1- and 3-frame requests
  convert signed-32 left/right to interleaved Float32 in place into the supplied
  capacity-3 borrowed Float32 workspace with payload pointer identity and zero
  allocation, an oversize 4-frame request is rejected before observer/write, and
  the old stateless submit path remains separate and allocating. Track 015 S5.5
  has since implemented and component-tested the device-free workspace-mode
  startup rejection: NULL samples, zero capacity, and capacity greater than
  `SIZE_MAX / 2` are rejected before any facade lifecycle,
  bind/request/render/observer, or allocation work, the instance stays INACTIVE
  with no in-request state, and the direct S5.2 route remains valid. This adds no
  real device/framework stop/close, locks/threads, workspace release/close,
  legacy renderer/module teardown, application/SDL/CLI,
  public API/port, queue/timer, rate/restart, or real live route work. The
  device-driven wiring of that coordinator and the callback/render integration
  have since been delivered by Track 015 S5, and S6.2–S6.4b added the
  negotiated-rate preparation handoff and the private legacy normalization
  (see "Current status"); invalid
  storage-length proof, workspace release/close, and the
  rest of this design remain deferred.

## Approved module-rendering direction

The private render boundary must answer a device request with the exact
requested frame count even while legacy module playback advances on its own
tick cadence. During active module playback, the renderer follows this approved
flow for every request:

```text
device requests N frames (one callback)

  while frames produced < N:
    if remaining frames in the current TFMX tick > 0:
        mix from current playback state, up to that remaining count
    else:
        advance TFMX exactly once (one tick)
        capture post-interpreter `eClocks` with that tick's snapshots
        derive the next tick's frame count (existing tick timing model,
        at the active device rate; see Sample-rate policy)
        continue mixing from the current playback state

deliver exactly N frames to the device
```

For N = 0 the loop body does not run and the request is answered with a
zero-frame block; on the implemented workspace/HAL route such a request is
accepted at the adapter's admission gate without invoking the renderer or the
exact-N coordinator (see "Private demand coordination").

- The renderer **retains the remaining-frame count for the current TFMX tick**
  between device requests; a request that ends in the middle of a tick leaves
  the remainder for the next request.
- When that remaining count is exhausted, the renderer **advances TFMX exactly
  once** — one tick of the legacy player. The private bridge captures
  post-interpreter `eClocks` with that tick's snapshots after `tfmxIrqIn()`,
  and the private context passes it to the existing mixer timing argument for
  that same rendered tick. The mixer derives the frame count from that tick
  clock and output rate (the active device rate for the run), retaining its
  exact-N remainder accumulation.
- The renderer **continues mixing until it fills the exact device-requested
  frame count**. A single request may span multiple TFMX ticks, and a single
  TFMX tick may span multiple requests.
- This is **not a pre-rendered audio queue**: no block is produced ahead of the
  request and pushed; the renderer advances TFMX only as far as needed to fill
  the current request, and every block is rendered fresh on demand.
- Preserving the existing tick timing model is **bounded Phase 4 compatibility
  evidence** for the intended impact on legacy timing and audio semantics; it is
  not a SynthTracker v1 compatibility promise.

Implemented Track 018 correction: for a qualifying local speed control (high
mask passes; low9 is 16..511), the interpreter sets `eClocks = 0x1B51F8 /
low9`; the prior Boolean-divisor defect is corrected. The private handoff,
mixer exact-N/remainder arithmetic, and callback-path restrictions are not a
public contract or a broad compatibility claim.

## Sample-rate policy

The approved sample-rate policy records how the private route is configured
for a run and what the rate does and does not mean:

- **Pre-start preparation obtains the negotiated device rate.** Before audio
  starts, the private route reads the active output device's negotiated sample
  rate and configures the callback-driven TFMX renderer at that rate for the
  active run.
- **No forcing to 44.1 kHz, no resampler.** The route does not force a device
  to 44.1 kHz, and Track 015 introduces no resampler: the renderer runs at the
  rate the device negotiates.
- **Audio Frame Blocks stay rate-free.** Blocks remain rate-free signed-32
  stereo mixed values. The rate is private lifecycle/configuration context for
  the run, not Frame Block metadata.
- **A device rate change requires quiescence.** A sample-rate change while the
  route is active requires the callback to become quiescent and a later
  reconfiguration/restart policy; detailed device-change handling remains
  deferred.

The bounded compatibility implication of the active-rate policy: the retained
tick timing model's tick/sample calculations use the active rate. The
per-tick frame-count derivation and the tick-advance cadence run at the
negotiated device rate for the run, not at a fixed 44.1 kHz, and bounded
compatibility evidence for the tick timing model is recorded accordingly.

Implemented: the private route requests the rate-0 startup-selection sentinel
(`sample_rate_hz == 0`); after device open the facade accepts only an actual
44.1/48 kHz nominal rate and reports the verified configured rate, and the
control side prepares the legacy exact-N renderer at that rate before
bind/start (S6.2–S6.4b). Explicit 44.1/48 requests retain equality checking;
any other nonzero request is preflight-invalid.

## Channel and format policy

The approved channel and format policy records what output configuration the
future route presents to the adapter and how Audio Frame Blocks map to it:

- **Strict stereo only.** Track 015 supports only an output configuration
  presented to the private adapter as exactly two channels. The adapter's
  private client format is interleaved Float32: each signed-32 Audio Frame
  Block's left/right values map directly to those two channels using the
  existing adapter-private conversion (the Track 014 signed-32 to interleaved
  Float32 mapping; see [`GLOSSARY.md`](GLOSSARY.md)).
- **Non-stereo-capable configurations are rejected.** A selected
  device/configuration that cannot provide this strict stereo setup is
  rejected rather than adapted.
- **No channel elaboration.** No mono fold-down, surround/upmix mapping,
  channel remapping, or device-native representation is introduced, and Audio
  Frame Blocks carry no channel or format metadata.
- **Negotiation mechanics remain deferred.** CoreAudio API/unit selection and
  the detailed device negotiation, error, and restart mechanics remain open;
  only the policy is decided here.

This policy is implemented on the private route: the adapter accepts only an
exactly two-channel interleaved Float32 configuration at a verified 44.1 or
48 kHz rate and rejects anything else before activation. The strict-profile
gate was removed by Track 015's executed Q5/Q6 retirement (see "Track 015 Q5/Q6
decisions"); the removed `-b`, `-8`, `-f`, `-o`, `-w`, and `-v` options are
ordinary unknown options with no `-o` file side effect.

## Events versus pre-rendered audio

Events are small, ordered descriptions of musical or control activity — a
note starting or stopping, a parameter change, a live-input message. They are
inputs that the renderer interprets while producing frames.

Pre-rendered audio would be a completed, finished block of audio produced
ahead of time and then delivered. That is not the model here. The renderer
produces every requested nonzero block fresh, on demand, from current state and
events (a zero-frame request on the implemented workspace/HAL route is accepted
at the adapter without render work — see "Private demand coordination"). This
distinction matters because it lets the device request any frame
count at any time — variable or zero — and still be answered correctly.

## Real-time contract

The callback path must be safe to run on a real-time audio thread. The
approved contract:

- **No allocation** on the callback path; all storage is lifecycle-preallocated.
- **No locking** on the callback path; no mutexes or other blocking
  primitives.
- **No file I/O** on the callback path.
- **No UI work** on the callback path.
- **No other unbounded work**: nothing that could take an unpredictable
  amount of time.

These are boundary requirements (ASR-010), not implementation preferences.
Adapter-private conversion also honors the real-time path: it must not
introduce allocation, locking, or unbounded work when the device is live.

## Start/stop ownership and callback quiescence

The real-time path is not only about what the callback must not do; it is also
about who may touch playback state and when. The approved ownership policy:

- **Prepare before start.** The application/control thread prepares the module
  and the callback context before audio starts. The callback never prepares or
  constructs playback state itself.
- **Callback-only mutation while active.** Once the route is active, the render
  callback has exclusive ownership of the legacy playback, timing, voice, and
  mix state. No other thread reads or mutates that state while the route is
  active.
- **Stop at a render-block boundary.** Control requests a stop between render
  blocks, never mid-block; the callback finishes the block it is rendering.
- **Quiescence before teardown.** The callback becomes quiescent before the
  control thread stops or disposes audio resources or tears down module state.
  Teardown proceeds only after quiescence.
- **No locks on the callback path.** Exclusivity comes from this lifecycle
  protocol — prepare before start, callback-only mutation while active,
  quiescence before teardown — not from synchronization inside the callback.
  The callback path contains no locks.

## Current status

Accurate as of 2026-08-24:

- **Track 014 is completed (historical).** It delivered the private, deliberately
  device-free CoreAudio adapter at
  `src/audio_output/adapters/coreaudio_adapter.c/.h`. The adapter converts
  signed-32 Audio Frame Blocks to adapter-private interleaved Float32 and had
  no device open/close, render callback, buffering, device clock, scheduling,
  or audible output; Track 015 built the live route on it.
- **Track 015 is completed.** The device-free foundation chunks are delivered and
  remain the deterministic test foundation: S3/S4 delivered the injected
  private-façade lifecycle seam, S5.1 the device-free exact-N coordinator,
  S5.2 the device-free private adapter-owned bound instance (open→configure→
  bind request→start with failure short-circuits before activation, active
  only after a successful start, active fake 0/1/3/2 requests routed exact-N
  to a non-converting sink with payload identity), S5.3 the device-free fake
  deferred-stop-to-quiescence (a stop during an active N=3 fake request
  completes that delivery exactly once, then active→stop-requested→quiescent),
  S5.4 the device-free borrowed preallocated conversion workspace (0-frame
  accepted without delivery; 1-/3-frame in-place conversion with payload
  identity and zero allocation; oversize rejected), and S5.5 the device-free
  workspace-mode startup rejection (NULL samples, zero capacity, and capacity
  greater than `SIZE_MAX / 2` rejected before any facade lifecycle,
  bind/request/render/observer, or allocation work).
- **S5 delivered the real device-driven route.** The production default facade
  is now the real macOS HAL Output Audio Unit route: open, configure (the
  device's nominal rate must be 44.1 or 48 kHz with strict interleaved Float32
  stereo), bind (`SetRenderCallback`), start (`AudioUnitInitialize`,
  `AudioOutputUnitStart`), stop, quiesce, and dispose, with the render callback
  admitting through the lock-free OPEN→IN_FLIGHT exchange and control-side
  quiescence (a zero-frame request on this workspace/HAL route is accepted at
  the admission gate without invoking the renderer or the exact-N
  coordinator); the non-APPLE default remains `UNAVAILABLE`. The real legacy
  exact-N renderer (`src/playback/playback_legacy_renderer.c`) fulfills exact
  device requests with partial-tick retention and exactly-once tick advance;
  the adapter converts through the validated borrowed Float32 workspace and
  copies into the separate native buffer on the HAL workspace-copy route. The
  application cuts over to this route, `src/audio.c`/`src/tfmx.c` are deleted,
  CMake no longer discovers or links SDL, and the `-b`, `-8`, `-f`, `-o`, `-w`,
  and `-v` options are removed (each an ordinary unknown option with no `-o`
  file side effect).
- **S6.2–S6.4b delivered the negotiated-rate preparation handoff.** The
  application requests the private rate-0 startup-selection sentinel; after
  device open the facade accepts only an actual 44.1/48 kHz nominal rate and
  reports the verified configured rate, and the control side prepares the
  renderer at that rate before bind/start — configure → prepare → bind →
  admission → start, with no request before preparation and rollback to
  INACTIVE on configure, unsupported-format, or preparation failure. The
  private legacy producer (`playback_legacy_mixer_render_frames`) first
  computes blended int32 lanes from the mixed voices using the legacy fixed
  L/R blend, then explicitly reconstructs each blended lane's low-16
  historical PCM-domain value and multiplies it by 65536 to produce the
  signed-32 Audio Frame Block values; the
  generic adapter conversion remains signed-32/2^31. `voices_01` is the
  self-authored supplemental audible-smoke fixture; a listener confirmed real
  48 kHz `voices_01` playback.
- **Validation evidence.** The full CTest suite passes 8/8;
  `test_audio_output` passes 36/36 (admission gate, workspace-copy, rollback,
  and lifecycle-quiescence tests) and `test_legacy_exact_renderer` passes all
  8 fixture/rate cases (the four approved fixtures at 44.1 and 48 kHz) with
  exact-N, zero-frame no-advance, partial-tick retention, exactly-once
  advance, non-silent stereo Float32, and zero allocation, including the exact
  `voices_01` Float32 oracle (L=1680/32768, R=1392/32768); application-level
  tests prove the workspace-only fake default 48 kHz route with requested rate
  0 and the exact open→configure→prepare→bind→start→stop→quiesce→dispose
  lifecycle, and the removed-option rejection. Supplemental real-device smoke
  checks ran on macOS (`step8`, and `voices_01` at 48 kHz).
- **Loader deferral.** A recorded user-supplied XOut2 module/sample pair is
  rejected by the private loader (`TFMX_LOAD_INVALID_FORMAT`); general
  real-module loader compatibility is deferred to a separate loader-focused
  Track, and no format-wide compatibility promise is made.
- **S6.5 complete; S7 completed the Track.** Track 015 S6.5 (final
  validation and documentation) was checked complete: the post-S6.4b full
  CTest suite passes 8/8, the listener-confirmed real 48 kHz `voices_01`
  smoke was recorded, the required documentation was reconciled, and the
  Phase 4 roadmap was revised (Phase 4 revision 14; canonical index revision
  21). S7 (completion) is checked: Track 015 is completed on 2026-08-24 at
  `.backlog/COMPLETED/2026/TRACK_015_COMPLETED_coreaudio_live_route_and_sdl_retirement.md`,
  and Stage 3 remains in progress pending its wider acceptance/merge.
- This design does not describe the current CLI as a public boundary, and it
  is not a SynthTracker v1 compatibility promise.

## Evidence direction

Evidence follows [`TESTING.md`](TESTING.md) ownership-based validation, as
delivered by Track 015 S5 and S6.2–S6.4b:

- **Deterministic component tests** drive the private render boundary with
  synthetic frame-count requests — variable counts, zero-frame requests, and
  boundary values — proving each request is answered with exactly the requested
  frames, without a physical device.
- **Deterministic application-level tests** exercise lifecycle and composition
  through the approved private CoreAudio system-call façade and its test-only
  fake (Track 015 Q4): the fake deterministically controls lifecycle outcomes,
  negotiated format, and zero or variable frame requests without physical
  audio hardware, so lifecycle, error, event-ordering, and ownership evidence
  runs in CI.
- **Audit of the real-time path** confirms the callback path performs no
  allocation, locking, file I/O, UI work, or other unbounded work. Structural
  or layout inspection is review support only; behavioral evidence comes from
  the automated tests above.
- **Bounded on-device supplemental checks** ran on a macOS device (a `step8`
  smoke and a `voices_01` 48 kHz smoke with listener confirmation); they
  complement automated coverage, never replace it, and are not a compatibility
  promise.
- **Bounded compatibility evidence for the tick timing model** records that the
  callback route uses post-interpreter `eClocks` for the same rendered tick's
  existing per-tick frame-count derivation and tick-advance cadence (see
  "Approved module-rendering direction") at the active device rate (see
   "Sample-rate policy"). Self-authored tests and fixtures are the primary
   temporary Phase 4 evidence. The user confirmed that the previously missing
   Turrican2-TITLE tempo transition is now audible. This supplemental user
   judgment follows automated evidence, does not replace tests or establish
   exact-audio or broad compatibility claims, and resolves only that observed
   transition.

## Open deferrals

The following remain explicitly open and are not decided by this design, by
ADR-009, or by ADR-010:

- **Runtime device-rate-change restart policy** — the HAL Output Audio Unit
  route, its open/configure/bind/start/stop/quiesce/dispose lifecycle, the
  concrete lifecycle state machine (active→stop-requested→quiescent), and the
  best-effort stop→quiesce→dispose teardown are implemented. What remains open
  is the concrete reconfiguration/restart policy for a device sample-rate
  change while the route is active (the quiescence-before-reconfiguration
  boundary itself is decided — see "Sample-rate policy") and detailed device
  error and change handling beyond the implemented teardown.
- **Device negotiation mechanics for the strict stereo configuration** — the
  channel and format policy is implemented (see "Channel and format policy"):
  the route supports only an exactly two-channel output configuration
  presented to the adapter, the adapter's private client format is interleaved
  Float32 with direct left/right mapping through the existing adapter-private
  conversion, and a selected device/configuration that cannot provide this
  strict stereo setup is rejected before activation. The sample-rate policy is
  also implemented (see "Sample-rate policy"): the route requests the rate-0
  startup-selection sentinel and accepts only a verified 44.1/48 kHz device
  rate. Detailed device error and restart mechanics remain open.
- **Event handoff, clock, and overflow semantics** — how time-ordered events
  reach the renderer across the real-time boundary, clock handling, and what
  happens under overload (overflow policy). Existing module playback is the
  only current renderer input; no event source is implemented yet.
- **Concrete legacy adaptation integration** — implemented (see "Approved
  module-rendering direction"): the real legacy exact-N renderer
  (`src/playback/playback_legacy_renderer.c`) retains the current TFMX tick's
  remaining-frame count, advances TFMX exactly once when it is exhausted,
  derives the next tick's frame count, and keeps mixing until the exact
  device-requested count is filled, and the private legacy mixer
  (`playback_legacy_mixer_render_frames`) first computes blended int32 lanes
  from the mixed voices using the legacy fixed L/R blend, then explicitly
  reconstructs each blended lane's low-16 historical PCM-domain value and
  multiplies it by 65536 into the existing signed-32 Frame Blocks. The legacy
  `src/audio.c`
  push machinery is retired with the SDL-era path. What remains open is the
  runtime device-rate-change reconfiguration path.
- **Actual live MIDI/input implementation** — live performance input is an
  event source in the model but is not implemented.
- **Workspace release/close and invalid storage-length proof** — the borrowed
  Float32 workspace is supplied by the caller for the run; a release/close
  contract and a proof for invalid storage-length preparation remain deferred.
- **General real-module loader expansion** — the recorded XOut2 rejection is a
  private loader-layout limitation deferred to a separate loader-focused Track;
  no format-wide loader compatibility promise is made.

The strict CLI profile and SDL-audio removal questions are not open deferrals
of this design: Track 015 resolved Q5 and Q6 on 2026-08-23 and executed them
in S5 chunk 3 (see "Track 015 Q5/Q6 decisions"). The removed options are
ordinary unknown options, and the SDL live-audio and `-o` file-output paths
are deleted.

## Track 015 Q5/Q6 decisions (executed)

The strict-CLI-profile and SDL-audio-retirement questions were deferred by
ADR-009 and by this design's original approval. On 2026-08-23, Track 015
resolved them as Track decisions:

- **Q5 — strict CLI profile retirement.** Track 015 directs removal of the
  strict-profile gate and the `-b`, `-8`, `-f`, `-o`, `-w`, and `-v`
  command-line options. Each removed option is an ordinary unknown option,
  and the `-o`-dependent file-output, stereo-blend, low-pass-filter, and
  oversampling behavior retires (the mixer's legacy fixed L/R blend itself
  remains in the private live route). The negotiated-rate, strict-stereo
  CoreAudio
  device boundary remains the live-device policy.
- **Q6 — SDL-audio retirement.** Track 015 directs retirement of every current
  SDL live-audio use and dependency, including the obsolete callback,
  lifecycle, ring-buffer, conversion, throttle/drain, and synchronization
  paths; SDL CMake discovery/linkage; SDL test stubs and branding; the `-o`
  file-output route; and accidental `SDL.h` transitive includes (replaced by
  direct required headers). No current SDL dependency has an approved
  non-audio justification; a future GUI may decide SDL independently.

These are Track-specific decisions recorded in the Track 015 decision log
(the Q5/Q6 resolutions were made while the Track was DRAFT) and have since
been executed by Track 015 S5 chunk 3: the strict-profile gate and the
`-o`-dependent file-output, stereo-blend, low-pass-filter, and oversampling
behavior are removed (the mixer's legacy fixed L/R blend remains in the
private live route), the SDL live-audio path and dependencies are retired,
and each removed option is an ordinary unknown option with no `-o` file side
effect. SDL remains a future GUI decision only.

## Related documents

- [`ADR-009`](adr/ADR-009-callback-driven-audio-rendering.md) — the accepted
  decision this design explains.
- [`ADR-010`](adr/ADR-010-native-adapter-ownership-and-private-demand-coordination.md)
  — refines ADR-009's private responsibility allocation: native adapters own
  device callbacks, lifecycle, and conversion, while private `audio_output`
  owns the synchronous device-demand coordinator.
- [`ADR-008`](adr/ADR-008-audio-frame-block-mixed-value-boundary.md) and
  [`ADR-005`](adr/ADR-005-target-daw-component-foundation.md) — the Audio Frame
  Block boundary and target component foundation.
- [`ASR.md`](ASR.md) — ASR-010 (callback-driven real-time rendering),
  ASR-009 (Audio Frame Block invariants), and ASR-006 (isolated future audio
  output).
- [`ARTIFACTS.md`](ARTIFACTS.md) — the Audio Frame Block artifact and its
  deferred contracts.
- [`GLOSSARY.md`](GLOSSARY.md) — canonical terminology, including
  callback-driven audio rendering and the Audio Render Boundary.
- [`ARCHITECTURE.md`](ARCHITECTURE.md) — current system and target boundary.
- [`TESTING.md`](TESTING.md) — evidence levels and validation strategy.
- [`TRACK_015_COMPLETED_coreaudio_live_route_and_sdl_retirement.md`](../.backlog/COMPLETED/2026/TRACK_015_COMPLETED_coreaudio_live_route_and_sdl_retirement.md)
  — Track 015 is COMPLETED: S5 delivered the real device-driven route and the
  application cutover with SDL/`-o`/CLI retirement, and S6.2–S6.4b delivered
  the rate-0 startup-selection sentinel with verified 44.1/48 kHz
  configuration and control-side preparation, the private legacy
  blend-then-widen normalization (blended int32 lanes reconstructed to their
  low-16 historical PCM-domain value and multiplied by 65536), and the
  `voices_01` audible-smoke
  fixture. S6.5 (final validation/documentation) is complete; S7 (completion)
  is checked. This design supports the Track, including the Q4
  test-double and the Q5/Q6 retirement decisions approved on 2026-08-23.
