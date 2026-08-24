# ADR-009: Callback-Driven Audio Rendering

Status: Accepted

Decision date: 2026-08-22.

## Context

Track 014 delivered a private, deliberately device-free CoreAudio adapter: it
converts signed-32 Audio Frame Blocks to an adapter-private interleaved
Float32 representation and retains no device lifecycle, render callback,
buffering, device clock, scheduling, or audible output. That device-free
adapter advances toward and supports the ADR-005 device-independent `Audio
Output` target direction, but it does not implement the target Audio Output
Port. Live device lifecycle
and the real-time route remain future private Track 015 work: Track 015 is now
ACTIVE (DRAFT at this decision's date) and its early implementation chunks are
complete — S4 delivered the injected private-façade lifecycle seam, S5.1
implemented and component-tested the device-free exact-N coordinator, S5.2
implemented and component-tested the device-free private adapter-owned bound
instance with its adapter-owned request entry, S5.3 implemented and
  component-tested the device-free fake deferred-stop-to-quiescence, and S5.4
  implemented and component-tested the device-free borrowed preallocated
  conversion workspace, and S5.5 implemented and component-tested the
  device-free workspace-mode startup rejection of invalid preparation — while
  the live device route and callback/render integration remained deferred as of
  that note (see "Implementation evidence and status" below).

The legacy engine renders audio on its own push cadence into a ring buffer that
an SDL-era callback drains. That push model does not map cleanly to a
device-driven route: platform audio hardware does not wait for software to push
audio; it requests audio on its own clock. Apple's Core Audio documents a pull
model in which the host or the hardware drives timing and asks the software to
produce a requested number of frames; PortAudio and JUCE document the same
callback-driven model for portable and cross-platform audio. A decision is
therefore needed about who owns render timing on the future live route and what
the render boundary must guarantee.

## Decision

Approve the callback-driven audio rendering model for the future private
Track 015 CoreAudio live route:

- **CoreAudio owns render timing.** The platform device drives when audio is
  produced; the software does not push audio into the device on its own cadence.
- A **private render boundary** produces every requested Audio Frame Block,
  including variable-size and zero-frame requests. Each request is satisfied on
  demand with exactly the requested frame count.
- The renderer is fed by **current and future playback state and time-ordered
  events** (for example, player events and, in future, live input). These are
  event sources, not pre-rendered-audio producers: what crosses the boundary is
  a freshly rendered block, never audio rendered ahead of time and pushed.
- The **callback path uses lifecycle-preallocated storage** and performs no
  allocation, locking, file I/O, UI work, or other unbounded work.
- **During active module playback, each request is filled by on-demand TFMX
  tick advancement.** The renderer retains the remaining-frame count for the
  current TFMX tick; when it is exhausted, it advances TFMX exactly once (one
  tick), derives the next tick's frame count, and continues mixing until it
  fills the exact device-requested frame count. A single request may span
  multiple TFMX ticks, and a single tick may span multiple requests. This is
  not a pre-rendered audio queue: ticks advance only to produce the frames the
  current request needs.
- **Adapter-private conversion remains separate**: destination-format conversion
  (such as the Track 014 Float32 mapping) stays inside the adapter and is not
  part of the render boundary contract.
- **No public Audio Output Port, C API, header, or export is introduced.** The
  boundary and the route are private; this decision creates no public contract
  or library target.
- **Start/stop ownership and callback quiescence.** The application/control
  thread prepares the module and the callback context before audio starts; the
  callback never prepares playback state itself. While the route is active, the
  render callback has exclusive ownership of the legacy playback, timing,
  voice, and mix state: no other thread reads or mutates that state. Control
  requests a stop at a render-block boundary (between blocks, never mid-block),
  and the callback becomes quiescent before the control thread stops or
  disposes audio resources or tears down module state. The callback path
  contains no locks; exclusivity is enforced by this lifecycle protocol, not by
  callback-path synchronization.
- **The active output device's negotiated sample rate configures the renderer
  for the run.** During pre-start preparation, the private route obtains the
  active output device's negotiated sample rate and configures the
  callback-driven TFMX renderer at that rate for the active run. It does not
  force a device to 44.1 kHz, and Track 015 introduces no resampler. Audio
  Frame Blocks remain rate-free signed-32 stereo mixed values: the rate is
  private lifecycle/configuration context for the run, not Frame Block
  metadata. A device sample-rate change requires quiescence and a later
  reconfiguration/restart policy; detailed device-change handling remains
  deferred.
- **The adapter's output configuration is strictly stereo.** Track 015 supports
  only an output configuration presented to the private adapter as exactly two
  channels. The adapter's private client format is interleaved Float32: each
  signed-32 Audio Frame Block's left/right values map directly to those two
  channels using the existing adapter-private conversion. A selected
  device/configuration that cannot provide this strict stereo setup is
  rejected. No mono fold-down, surround/upmix mapping, channel remapping,
  device-native representation, or Audio Frame Block metadata is introduced.
  CoreAudio API/unit selection and the detailed device negotiation, error, and
  restart mechanics remain deferred.

This decision formalizes the callback-driven render model already recorded as
the resolved Q1 in the Track 015 DRAFT, records the Q2 start/stop ownership
policy approved on 2026-08-22, and resolves Q3 — the sample-rate aspect with
the active-rate policy above, and the channel and adapter-private device-format
aspect with the strict stereo policy above. Q5's strict CLI live-profile
treatment and Q6's SDL-audio retirement were deferred at decision time; Track
015 resolved both on 2026-08-23 as approved-but-not-implemented Track
decisions (see "Later Track 015 Q5/Q6 decisions"). It is target-directional: it
does not implement the live route, does not describe the current transitional
CLI, and does not promise current behavior or compatibility.

## Refinement by ADR-010

ADR-010 (Accepted, 2026-08-23) refines this decision's private responsibility
allocation without altering any of its device-clock, on-demand, real-time,
active-rate, or strict-stereo policies:

- **Native audio adapters own native device concerns.** The CoreAudio adapter
  owns its device callbacks, device lifecycle, and device-format conversion.
- **Private `audio_output` owns the synchronous device-demand coordinator.**
  For each adapter request of exactly N frames, `audio_output` requests exactly
  N Audio Frame Block frames from the private renderer and immediately routes
  the result to the requesting adapter; it contains no software queue, timer,
  or CoreAudio code. This private coordinator is distinct from the deferred
  public Audio Output Port.
- Ownership is now decided at the responsibility level; the coordinator's
  exact-N policy has since been implemented and component-tested at the
  device-free level by Track 015 S5.1, S5.2 has since implemented and
  component-tested the device-free private adapter-owned bound instance whose
  adapter-owned request entry routes active requests exact-N through the
  existing coordinator to a non-converting sink with payload identity, S5.3
  has since implemented and component-tested the device-free fake
  deferred-stop-to-quiescence, and S5.4 has since implemented and
  component-tested the device-free borrowed preallocated conversion workspace,
  and S5.5 has since implemented and component-tested the device-free
  workspace-mode startup rejection of invalid preparation, while
  the device-driven wiring has since been delivered (see "Implementation
  evidence and status" below) and the
  remaining mechanics stay deferred as recorded in this decision's Deferrals
  below.

## Later Track 015 Q5/Q6 decisions

ADR-009 itself deferred the strict CLI live-profile treatment and the exact
SDL-audio removal inventory (see Deferrals). On 2026-08-23, Track 015 resolved
Q5 and Q6 as approved-but-not-implemented Track decisions:

- **Q5 — strict CLI profile retirement.** Track 015 directs removal of the
  strict-profile gate and the `-b`, `-8`, `-f`, `-o`, `-w`, and `-v`
  command-line options. Each removed option is an ordinary unknown option;
  the dependent file-output, stereo-blend, low-pass-filter, and
  oversampling behavior retires. The negotiated-rate, strict-stereo
  CoreAudio device boundary remains the live-device policy.
- **Q6 — SDL-audio retirement.** Track 015 directs retirement of every current
  SDL live-audio use and dependency, including the obsolete callback,
  lifecycle, ring-buffer, conversion, throttle/drain, and synchronization
  paths; SDL CMake discovery/linkage; SDL test stubs and branding; the `-o`
  file-output route; and accidental `SDL.h` transitive includes (replaced by
  direct required headers). No current SDL dependency has an approved
  non-audio justification; a future GUI may decide SDL independently.

These are Track-specific decisions recorded in the Track 015 decision log
(the Q5/Q6 resolutions were made while the Track was DRAFT). They are approved
for future implementation, not implemented behavior: this ADR did not make them
itself, current CLI and SDL behavior remains unchanged, and the retirement work
proceeds under the now-ACTIVE Track's execution gates. Both have since been
executed by Track 015 S5 chunk 3; see "Implementation evidence and status"
below.

## Deferrals

This decision does not fix the CoreAudio API or audio unit choice, the exact
device open/start/stop/close lifecycle, detailed device error and change
handling (including the concrete reconfiguration/restart policy for a device
sample-rate change — the quiescence-before-reconfiguration boundary itself is
decided, see Decision and Consequences), or the concrete lifecycle state-machine
details (the ownership policy itself is decided — see Decision and
Consequences); the concrete device negotiation that selects and validates the
strict stereo configuration (the channel and adapter-format policy itself is
decided — see Decision and Consequences); event handoff, clock, and overflow
semantics; the concrete private
function/state integration of the legacy renderer adaptation to the
callback-driven boundary (the adaptation direction itself is decided — see
Decision and Consequences); or actual live MIDI or input implementation. The
strict CLI live-profile treatment (including the `-f` rate-suggestion
semantics) and the exact SDL-audio removal inventory were likewise not decided
by this ADR; Track 015 resolved both on 2026-08-23 as
approved-but-not-implemented Track decisions (see "Later Track 015 Q5/Q6
decisions"). The concrete C API and layout, threading and
scheduling, and validation representation also remain deferred to later work.
ADR-010 (Accepted, 2026-08-23) assigns the private `audio_output` synchronous
device-demand coordinator role and native adapter ownership of callbacks,
lifecycle, and conversion (see "Refinement by ADR-010"); Track 015 S5.1 has
implemented and component-tested the coordinator's device-free exact-N policy,
S5.2 has implemented and component-tested the device-free private
adapter-owned bound instance with its adapter-owned request entry routing
active requests exact-N through the existing coordinator to a non-converting
sink with payload identity, S5.3 has implemented and component-tested the
device-free fake deferred-stop-to-quiescence, and S5.4 has implemented and
component-tested the device-free borrowed preallocated conversion workspace,
and S5.5 has implemented and component-tested the device-free workspace-mode
startup rejection of invalid preparation,
while its device-driven wiring has since been delivered (see "Implementation
evidence and status" below) and the remaining mechanics stay deferred as
stated above.

## Consequences

- Render timing is device-owned: the engine produces exactly what the device
  requests, including variable-size and zero-frame requests, instead of pushing
  audio from a software cadence.
- Deterministic automated tests can drive the private render boundary with
  synthetic frame-count requests and lifecycle contracts without a physical
  device, providing the real-time contract evidence required by Track 015.
- The real-time contract (lifecycle-preallocated storage; no allocation,
  locking, file I/O, UI work, or other unbounded work on the callback path) is a
  boundary requirement, not an implementation preference.
- Events and rendered audio are distinct: playback state and time-ordered
  events are renderer inputs; Audio Frame Blocks are the rendered outputs; no
  pre-rendered audio is pushed toward the device.
- Adapter-private conversion stays separate from the render boundary, so the
  boundary remains device-independent in representation.
- The future route's output configuration is strictly stereo: only an exactly
  two-channel output presented to the adapter is supported, mapped from each
  Audio Frame Block's left/right values through the existing adapter-private
  interleaved Float32 conversion. A selected device/configuration that cannot
  provide this strict stereo setup is rejected, and no mono fold-down,
  surround/upmix mapping, channel remapping, device-native representation, or
  Audio Frame Block metadata is introduced.
- Start/stop ownership is a lifecycle contract: the control thread prepares the
  module and the callback context before audio starts, requests a stop at a
  render-block boundary, and waits for the callback to become quiescent before
  stopping or disposing audio resources or tearing down module state; while
  active, the callback exclusively owns the legacy playback, timing, voice, and
  mix state, and the callback path contains no locks.
- No public Audio Output Port, C API, header, or export results from this
  decision; the CoreAudio route remains private composition under the
  ADR-005/ASR-006 audio-output direction.
- The private responsibility allocation is refined by ADR-010 (Accepted,
  2026-08-23): native adapters own device callbacks, lifecycle, and conversion,
  and the private `audio_output` component owns the synchronous device-demand
  coordinator — requesting exactly N Audio Frame Block frames per adapter
  request of N frames and routing the result immediately to the requesting
  adapter, with no queue, timer, or CoreAudio code in `audio_output`. That
  private coordinator is distinct from the deferred public Audio Output Port;
  ownership is decided, Track 015 S5.1 has implemented and component-tested
  the coordinator's device-free exact-N policy, S5.2 has implemented and
  component-tested the device-free private adapter-owned bound instance with
  its adapter-owned request entry routing active requests exact-N through the
  existing coordinator to a non-converting sink with payload identity, S5.3
  has implemented and component-tested   the device-free fake
  deferred-stop-to-quiescence, and S5.4 has implemented and component-tested
  the device-free borrowed preallocated conversion workspace, and S5.5 has
  implemented and component-tested the device-free workspace-mode startup
  rejection of invalid preparation, while its
  device-driven wiring has since been delivered (see "Implementation evidence
  and status" below).
- The legacy module-rendering adaptation is decided at the behavioral level:
  the renderer retains each TFMX tick's remaining-frame count, advances TFMX
  exactly once when that count is exhausted, derives the next tick's frame
  count, and keeps mixing until the exact device-requested count is filled; a
  request may span multiple ticks and no pre-rendered queue is introduced. Only
  the concrete private function/state integration of that adaptation remains
  open.
- Preserving the existing TFMX tick timing model (the per-tick frame-count
  derivation and the tick-advance cadence) is bounded Phase 4 compatibility
  evidence for the intended impact on legacy timing and audio semantics — a
  temporary development scaffold, not a SynthTracker v1 compatibility promise.
- Tick/sample calculations use the active rate: the retained tick timing
  model's per-tick frame-count derivation is computed from the negotiated
  device rate for the run, not from a fixed 44.1 kHz. Bounded Phase 4
  compatibility evidence therefore records the intended impact of the
  active-rate tick/sample calculation on legacy timing and audio semantics,
  while Audio Frame Blocks remain rate-free signed-32 stereo mixed values and
  carry no rate metadata.
- During Phase 4, this decision does not alter current SDL-era TFMX behavior
  and creates no SynthTracker v1 compatibility promise. At the time of these
  device-free evidence notes, the private
  CoreAudio adapter was device-free and the production default façade was
  unavailable; Track 015 is ACTIVE, S5.1 has implemented and component-tested
  a device-free private synchronous exact-N coordinator, S5.2 has
  implemented and component-tested the device-free private adapter-owned bound
  instance — strict format and complete route, open→configure→bind
  request→start with failure short-circuits before activation, active only
  after a successful start, and active fake 0/1/3/2 requests routed exact-N
  through the existing coordinator to a non-converting sink with payload
  identity, while failed or inactive requests reach neither renderer nor sink —
  S5.3 has implemented and component-tested the device-free fake
  deferred-stop-to-quiescence: a stop requested during an active N=3 fake
  request lets that exact-N delivery complete exactly once with payload
  identity, then the instance transitions active→stop-requested→quiescent, the
  fake source quiesces, and later fake source requests are refused at the
  source before any callback/render/sink work — and S5.4 has implemented and
  component-tested the device-free borrowed preallocated conversion workspace:
  a 0-frame request is accepted with no observer delivery, 1- and 3-frame
  requests convert signed-32 left/right to interleaved Float32 in place into
  the supplied capacity-3 borrowed Float32 workspace with payload pointer
  identity and zero allocation, an oversize 4-frame request is rejected before
  observer/write, and the old stateless submit path remains separate and
  allocating. S5.5 has implemented and component-tested the device-free
  workspace-mode startup rejection: NULL samples, zero capacity, and capacity
  greater than `SIZE_MAX / 2` are rejected before any facade lifecycle,
  bind/request/render/observer, or allocation work, the instance stays
  INACTIVE with no in-request state, and the direct S5.2 route remains valid.
  No real device/framework stop/close,
  threads/locks, workspace release/close, legacy renderer/module teardown,
  application/SDL/CLI, public API/port, queue/timer, rate/restart, or real live
  route work was added in those device-free chunks; the real HAL Output Audio
  Unit route, callback/render integration, and application/SDL/CLI retirement
  have since been delivered, and the Q5/Q6 approvals have since been executed
  (see "Implementation evidence and status" below). Invalid storage-length
  proof, workspace release/close, and the device-rate-change restart policy
  remain deferred.

## Implementation evidence and status

This decision remains **Accepted** and authoritative for the delivered private
Track 015 route. Implementation evidence, recorded 2026-08-24:

- **S5 delivered the real device-driven route.** The real legacy exact-N
  renderer (`src/playback/playback_legacy_renderer.c`) fulfills exact device
  requests with partial-tick retention and exactly-once tick advance; the
  private CoreAudio adapter runs the lock-free OPEN/IN_FLIGHT admission gate
  with control-side quiescence and workspace-only conversion (borrowed
  preallocated Float32 workspace, then a copy into the separate native buffer
  on the HAL workspace-copy route); the production default facade is a real
  HAL Output Audio Unit on macOS with a strict 44.1/48 kHz negotiated-rate
  gate and no resampler (`UNAVAILABLE` elsewhere); and the application cuts
  over to this route with the SDL live-audio path, the `-o` file-output path,
  and the `-b`, `-8`, `-f`, `-o`, `-w`, and `-v` options deleted (the
  `-o`-dependent file-output stereo blending, low-pass filtering, and
  oversampling retire with the removed options, while the mixer's legacy fixed
  L/R blend remains in the private live route). On this
  workspace/HAL route a zero-frame request is accepted at the adapter's
  admission gate without invoking the renderer or the exact-N coordinator:
  the implemented route answers zero-frame requests without render work, which
  satisfies the decision's on-demand boundary contract for them (the broader
  "every request rendered" phrasing remains the approved direction).
- **S6.2–S6.4b delivered the negotiated-rate preparation handoff.** The
  application requests the private rate-0 startup-selection sentinel; after
  device open the facade accepts only an actual 44.1/48 kHz nominal rate and
  reports the verified configured rate, and the control side prepares the
  renderer at that rate before bind/start (no request precedes preparation;
  configure, unsupported-format, or preparation failure rolls back to
  INACTIVE). The private legacy producer first computes blended int32 lanes
  from the mixed voices using the legacy fixed L/R blend, then explicitly
  reconstructs each blended lane's low-16 historical PCM-domain value and
  multiplies it by 65536 into the existing signed-32 Audio Frame Blocks; the
  generic adapter
  conversion remains signed-32/2^31. `voices_01` is the self-authored
  supplemental audible-smoke fixture, and a listener confirmed real 48 kHz
  `voices_01` playback.
- **Validation evidence.** The full CTest suite passes 8/8;
  `test_audio_output` passes 36/36 and `test_legacy_exact_renderer` passes all
  8 fixture/rate cases (the four approved fixtures at 44.1 and 48 kHz),
  including the exact `voices_01` Float32 oracle (L=1680/32768, R=1392/32768);
  application-level tests prove the workspace-only fake default 48 kHz route
  with requested rate 0 and the exact open→configure→prepare→bind→start→
  stop→quiesce→dispose lifecycle, and the removed-option rejection.
  Supplemental real-device smoke checks ran on macOS (`step8`, and `voices_01`
  at 48 kHz).
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
  and general real-module loader expansion remain deferred; general real-module
  loader compatibility in particular remains deferred after the recorded XOut2
  rejection, with no format-wide compatibility promise. The private
  `audio_output` translation unit still includes
  `audio_output_dispatch_submit`, which calls the CoreAudio adapter dispatch
  on Apple platforms; removing that dispatch — so that the whole `audio_output`
  component is CoreAudio-free as the decision's coordinator role describes —
  remains an outstanding private refactor/deferral.

## Related ASRs

- ASR-006 — Isolated future audio output: the CoreAudio adapter is the intended
  platform-specific adapter behind the device-independent Audio Output Port
  direction.
- ASR-009 — Audio Frame Block Boundary Invariants: the render boundary produces
  Audio Frame Blocks that satisfy the ASR-009 invariants and remain rate-free
  signed-32 stereo mixed values.
- ASR-010 — Callback-driven real-time audio rendering: the callback-driven
  render boundary, the start/stop ownership and quiescence lifecycle, the
  active-rate renderer configuration, and the strict stereo channel/format
  policy this decision satisfies.
- ASR-003 — UI-agnostic playback core.
- ASR-004 — Explicit, independently testable component boundaries.

## Evidence

Official Apple Core Audio pull-model and audio-unit rendering documentation:

- [Core Audio Overview — Introduction](https://developer.apple.com/library/archive/documentation/MusicAudio/Conceptual/CoreAudioOverview/Introduction/Introduction.html) — Apple's architectural overview of Core Audio, including the hardware-driven flow of audio through the system.
- [Audio Unit Hosting Guide for iOS — Audio Unit Hosting Fundamentals](https://developer.apple.com/library/archive/documentation/MusicAudio/Conceptual/AudioUnitHostingGuide_iOS/AudioUnitHostingFundamentals/AudioUnitHostingFundamentals.html) — documents the pull model: a host requests audio from an audio unit by calling its render function, which fills the requested buffer.
- [AUAudioUnit](https://developer.apple.com/documentation/audiotoolbox/auaudiounit) — the audio-unit contract that the system pulls audio from via render blocks.
- [AudioUnitRender](https://developer.apple.com/documentation/audiotoolbox/audiounitrender(_:_:_:_:_:_:)) — the render entrypoint that renders the requested number of frames into the caller's buffer.
- [AudioDeviceCreateIOProcID](https://developer.apple.com/documentation/coreaudio/audiodevicecreateioprocid(_:_:_:_:)) — registers the device I/O proc that Core Audio invokes on the device's audio thread.
- [AudioDeviceIOProc](https://developer.apple.com/documentation/coreaudio/audiodeviceioproc) — the I/O proc contract: a system-driven callback that provides and collects device audio.
- [kAudioDevicePropertyNominalSampleRate](https://developer.apple.com/documentation/coreaudio/kaudiodevicepropertynominalsamplerate) — the Core Audio device property that reports the device's nominal sample rate, the negotiated rate the private route obtains during pre-start preparation.

JUCE callback-driven rendering documentation:

- [AudioIODeviceCallback](https://docs.juce.com/master/classAudioIODeviceCallback.html) — the callback the audio device invokes when it needs the next block of audio data.
- [AudioProcessor](https://docs.juce.com/master/classAudioProcessor.html) — `processBlock`, called when a block of audio needs processing; JUCE's canonical on-demand rendering model.

PortAudio callback guidance:

- [PortAudio v19 API overview](http://files.portaudio.com/docs/v19-doxydocs/api_overview.html) — documents callback streams: PortAudio calls a user callback when new audio data is available or required.
- [Bencina, "Real-Time Audio Programming 101: Time Waits for Nothing"](http://www.rossbencina.com/code/real-time-audio-programming-101-time-waits-for-nothing) — canonical real-time callback safety guidance: no blocking, allocation, locking, or I/O on the audio path.
- [PortAudio GitHub wiki](https://github.com/PortAudio/portaudio/wiki) — maintained guidance and API discussion.

Repository evidence:

- [`ADR-005`](ADR-005-target-daw-component-foundation.md) — the target component
  foundation and the device-independent `Audio Output` direction that a future
  CoreAudio adapter implements; the Track 014 device-free adapter advances
  toward that direction without implementing the target Audio Output Port.
- [`ADR-010`](ADR-010-native-adapter-ownership-and-private-demand-coordination.md)
  — refines this decision's private responsibility allocation, assigning native
  adapter ownership and the private `audio_output` synchronous device-demand
  coordinator role.
- [`ADR-008`](ADR-008-audio-frame-block-mixed-value-boundary.md) — the Audio
  Frame Block mixed-value boundary the render boundary produces.
- [`ASR.md`](../ASR.md) — records ASR-006, ASR-009, and the new ASR-010
  requirement this decision satisfies.
- [`AUDIO_RENDERING_DESIGN.md`](../AUDIO_RENDERING_DESIGN.md) — plain-language
  component flow, real-time contract, current status, and open deferrals.
- [`TRACK_014_COMPLETED_coreaudio_adapter.md`](../../.backlog/COMPLETED/2026/TRACK_014_COMPLETED_coreaudio_adapter.md)
  — completed Track that delivered the private device-free conversion adapter.
- [`TRACK_015_COMPLETED_coreaudio_live_route_and_sdl_retirement.md`](../../.backlog/COMPLETED/2026/TRACK_015_COMPLETED_coreaudio_live_route_and_sdl_retirement.md)
  — Track 015 is COMPLETED: S4 completed the injected private-façade lifecycle
  seam, S5.1 implemented and component-tested the device-free exact-N
  coordinator, S5.2 implemented and component-tested the device-free
  adapter-owned bound instance with its adapter-owned request entry, S5.3
  implemented and component-tested the device-free fake
  deferred-stop-to-quiescence, S5.4 implemented and component-tested the
  device-free borrowed preallocated conversion workspace, and S5.5 implemented
  and component-tested the device-free workspace-mode startup rejection of
  invalid preparation. Its
  resolved Q1 records the callback-driven render model this
  decision formalizes, and its later Q5/Q6 resolutions (2026-08-23) are
  recorded under "Later Track 015 Q5/Q6 decisions". S5 delivered the
  real device-driven route and S6.2–S6.4b the rate-0 startup selection,
  control-side preparation, and legacy normalization; S6.5 validation and
  documentation are complete and S7 completed the Track on 2026-08-24 — see
  "Implementation evidence and status" above.
