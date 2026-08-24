# TRACK 015 [COMPLETED]: coreaudio_live_route_and_sdl_retirement

Track
- ID: TRACK_015
- Repository: SynthTracker
- Branch: stage/04-03-audio-output-extraction
- Current path: `.backlog/COMPLETED/2026/TRACK_015_COMPLETED_coreaudio_live_route_and_sdl_retirement.md`

Problems (PORE)
- P1: As a SynthTracker listener, I cannot hear live playback because the current
  private CoreAudio adapter is deliberately device-free and the temporary live
  bridge ends at a synchronous sink.
- P2: As a SynthTracker maintainer, I cannot reason about the live Audio Output
  boundary because the retired SDL-era live device, callback, ring-buffer, and
  synchronization implementation remains alongside the temporary route.

Objective
- Deliver a privately composed, real-time CoreAudio live-output route on macOS
  and remove the legacy SDL live-audio and `-o` file-output routes.

Non-negotiables
- Track 015 was ACTIVE and its Move-to-ACTIVE plan step (S2) was checked before
  implementation, including TDD-driving tests; completed work followed the
  ACTIVE Track/TDD gates.
- Follow TDD for every implementation chunk: failing focused automated test,
  smallest passing change, refactor, then relevant validation.
- Keep Audio Frame Blocks as ordered, zero-or-more signed-32 `{left, right}`
  mixed values. They are never serialized PCM or device-native data.
- Keep all CoreAudio conversion and device representation adapter-private. Add
  no public C API, header, export, library, or public target.
- The CoreAudio render callback drives a private render boundary for each
  device-requested frame count, including variable and zero-length requests.
  It uses lifecycle-preallocated storage; it performs no allocation, locking,
  file I/O, or UI work. Existing module playback is the only current source;
  later sources may contribute time-ordered control events, not pre-rendered
  audio.
- The application prepares module and callback context before audio starts.
  While active, only the callback mutates legacy playback, timing, voice, and
  mix state. Control requests a stop at a render-block boundary; the callback
  becomes quiescent before control stops/disposes audio resources or tears down
  module state.
- Pre-start preparation configures the renderer for the active output device's
  negotiated sample rate. Track 015 neither forces 44.1 kHz nor adds a
  resampler; Audio Frame Blocks remain rate-free mixed values.
- The private adapter supports exactly two output channels in interleaved
  Float32, mapping each Frame Block's left/right values directly. It rejects a
  selected device configuration that cannot provide this strict stereo setup.
- Use C23 or later for project-owned source. Project-owned headers remain
  private and co-located; `include/` remains retired.
- Remove every current SDL audio API use and any CMake or test dependency needed
  solely for legacy SDL audio. This Track does not implement a GUI; a later GUI
  component may introduce SDL for graphical use.
- Remove the `-b`, `-8`, `-f`, `-o`, `-w`, and `-v` command-line options and
  their dependent file-output, stereo-blend, low-pass-filter, and
  oversampling behavior. Each removed option is an ordinary unknown option;
  `-o` has no file-output side effect.
  Assess and retain bounded evidence for the intended impact on legacy TFMX
  playback, trackstep, pattern, macro, timing, interpreter, and audio semantics;
  compatibility is a temporary Phase 4 scaffold, not a SynthTracker v1 promise.
- Resolve and record the mandatory six-dimension version-impact decision before
  implementation: C API/ABI, module compatibility/extension,
  interpreter/timing/audio behavior, persistent DAW format/versioning,
  platform/audio-output adapter, and component/package boundaries.

Acceptance criteria
- [x] A1) [P1] A deterministic automated component contract proves the private
  CoreAudio route can accept Audio Frame Blocks, manage its approved live-device
  lifecycle, and drive its private renderer for requested frame counts without
  exposing device-native data at the Audio Frame Block boundary.
- [x] A2) [P1] An automated application-level workflow proves the default live
  playback route reaches private CoreAudio; automated CLI evidence proves that
  each removed option (`-b`, `-8`, `-f`, `-o`, `-w`, and `-v`) is rejected as
  an unknown option with usage and a non-zero exit status, and that `-o` cannot
  create a file or invoke file output.
- [x] A3) [P2] The legacy SDL live-audio callback, device lifecycle,
  conversion/ring-buffer, throttle/drain, synchronization, and `-o` file-output
  routes are removed without moving rendered-file export into CoreAudio; the
  executable and affected tests contain no SDL audio API use or dependency
  justified solely by the retired audio path.
- [x] A4) [P1, P2] Automated lifecycle, error, event ordering, ownership, and
  real-time render-safety evidence covers the approved design without requiring
  a physical audio device in CI.
- [x] A5) [P1, P2] Relevant focused tests and project validation pass, and
  bounded compatibility evidence records the intended legacy impact.
- [x] A6) [P2] Architecture, ASR, ADR, glossary, README, and testing
  documentation are updated when the approved implementation changes their
  governed boundary or behavior.

Why now / impact
- Track 014 established a private device-free CoreAudio conversion boundary.
  The Stage 3 roadmap now calls for the real-time route and SDL live-route
  retirement before later Mixer extraction can begin.

Scope
- In scope:
  - Private macOS CoreAudio device lifecycle and real-time render route for
    Audio Frame Blocks at the approved negotiated sample rate and strict stereo
    adapter format.
  - A callback-driven private render boundary that produces each requested,
    possibly variable-sized Audio Frame Block with lifecycle-preallocated
    storage. It integrates existing module playback and must not foreclose later
    time-ordered event sources; it does not implement live input.
  - A private legacy tick adapter that retains the current TFMX tick's remaining
    frames, advances the interpreter once when that tick is exhausted, and mixes
    across ticks as needed to fill each exact device request.
  - Adaptation or replacement of the current legacy live bridge so the callback
    renders requested frames on demand rather than receiving pushed mixed blocks.
  - Retirement of the legacy SDL live-audio code and linkage after equivalent
    approved CoreAudio behavior and test evidence exist.
  - Removal of the `-o` option and its legacy file-output code, tests, and
    documentation as part of retiring the obsolete audio path.
  - Removal of current SDL build and test dependencies that serve only the
    retired audio route, without implementing or deciding the future GUI.
  - Deterministic component, application, integration, and bounded compatibility
    evidence for the approved route.
  - Private legacy-producer amplitude normalization from its historical
    signed-16 PCM domain into the existing signed-32 Audio Frame Block range;
    it does not define a numerical rule for future producers or a public Frame
    Block contract.
- Out of scope:
  - A public device-independent Audio Output Port or public C API.
  - Mixer extraction or a target Mixer internal representation.
  - Non-macOS audio adapters, Linux support, or a persistent DAW format.
  - GUI implementation, editing workflows, or SDL GUI decisions.
  - Live MIDI devices, musical-performance input, or any other live-input
    feature.
  - A replacement rendered-file export; a future File I/O component may assess
    that capability independently.
  - General real-module loader compatibility beyond the four valid fixture pairs
    and malformed-fixture coverage approved for this Track. The recorded XOut2
    layout limitation is deferred to a separate loader-focused Track after Track
    015; this Track makes no assertion that that external module is malformed.

Milestones
- [x] M1) Resolve live-device mechanics, deterministic test approach,
  strict-profile, and SDL-retirement decisions; record the complete
  version-impact decision.
- [x] M2) Establish deterministic CoreAudio lifecycle and render-path contract
  evidence through focused tests.
- [x] M3) Deliver the approved private CoreAudio live route through sequential
  TDD chunks.
- [x] M4) Retire the SDL live-audio route, validate application behavior and
  compatibility evidence, and reconcile the Phase 4 roadmap.

Risks / decisions
- Risk: A live device callback can introduce timing, ownership, and real-time
  safety failures not exercised by Track 014's device-free adapter.
- Risk: A selected device may not provide the approved strict stereo
  configuration and must be rejected without entering the active lifecycle.
- Risk: Rendering TFMX at the active device rate changes per-tick frame counts;
  bounded compatibility evidence must assess its timing and audio impact.
- Risk: The current SDL build/test dependency may combine audio and future GUI
  concerns. Retire only its current audio use and any dependency justified
  solely by it; a later GUI component owns graphical SDL integration.
- Risk: Removing `-o` is an intentional CLI compatibility break and requires
  explicit CLI, documentation, and bounded compatibility evidence.
- Decision: The callback-driven render model, private CoreAudio test double,
  CLI retirement, SDL retirement boundary, bounded compatibility evidence, and
  ADR-010 private demand coordination are approved. Native adapters own their
  device callbacks, lifecycle, and conversion; private `audio_output` owns the
  synchronous N-frame render-request coordinator. Concrete CoreAudio API/unit,
  detailed lifecycle/error/change handling, renderer-state integration, and
  device-rate-change restart policy remain implementation details within the
  approved boundary.
- Version impact: Complete in the Decision log before implementation. C API/ABI,
  module compatibility/extension, and persistent DAW format/versioning remain
  unchanged; interpreter/timing/audio behavior, platform/audio-output adapter,
  and component/package boundaries change as recorded below.

Open questions
- [x] Q1) The CoreAudio callback drives a private render boundary for each
  requested, possibly variable-sized frame block. The renderer uses
  lifecycle-preallocated storage and performs no allocation, locking, file I/O,
  or UI work; existing module playback is the sole current source. Later inputs
  may provide time-ordered control events, not pre-rendered audio. This
  establishes no public Audio Output Port.
- [x] Q2) The application prepares module and callback context before start; the
  callback exclusively mutates legacy playback, timing, voice, and mix state
  while active. Control requests stop at a render-block boundary, and callback
  quiescence precedes audio-resource disposal and module teardown.
- [x] Q3) Configure the renderer at the active output device's negotiated rate,
  without forcing 44.1 kHz or adding a resampler. The private adapter supports
  exactly two interleaved Float32 output channels, mapping Frame Block left/right
  values directly and rejecting configurations that cannot provide strict stereo.
  Frame Blocks have no rate, channel, or device-format metadata. CLI strict
  profile and `-f` consequences remain Q5.
- [x] Q4) Use a private CoreAudio system-call façade and a test-only fake. The
  fake deterministically controls lifecycle outcomes, negotiated format, and
  zero or variable frame requests without physical audio hardware. It adds no
  public or generic device API.
- [x] Q5) Remove the strict-profile gate and the `-b`, `-8`, `-f`, `-o`, `-w`,
  and `-v` options. Remove their dependent file-output, stereo-blend,
  low-pass-filter, and oversampling behavior. Each removed option is an ordinary
  unknown option; the negotiated-rate, strict-stereo CoreAudio device boundary
  remains.
- [x] Q6) Retire every current SDL live-audio use and dependency, including the
  obsolete callback, lifecycle, ring-buffer, conversion, throttle/drain, and
  synchronization paths; SDL CMake discovery/linkage; test stubs; SDL branding;
  and the `-o` file-output route. Replace accidental `SDL.h` transitive includes
  with direct required headers. No current SDL dependency has an approved
  non-audio justification; a future GUI may decide SDL independently.
- [x] Q7) Retain the four valid fixture pairs (`step8`, `loop_f1`,
  `envelope_tempo`, and `voices_01`) and existing malformed-fixture coverage.
  Add deterministic fake-device callback evidence at 44.1 and 48 kHz for zero
  and variable requests, frame retention, and exactly-once tick advance; prove
  non-silent stereo Float32 fixture-to-callback output; record a supplemental
  macOS on-device smoke check; and test every removed option as unknown,
  including `-o`'s lack of file side effects. This is bounded Phase 4 evidence,
  not a format-wide or bit-identical compatibility claim.

Decision log
- Decision (roadmap): The living Phase 4 roadmap, revision 13, places Track 015
  in Stage 3 after Track 014: establish the live real-time CoreAudio route and
  retire the SDL live route before later Mixer extraction.
- Decision (scope): User authorized drafting this Track on 2026-08-22 with
  audible CoreAudio playback and legacy SDL live-audio code/linkage retirement
  in scope.
- Decision (`-o`): User directed removal of the `-o` option and its legacy
  file-output functionality on 2026-08-22; this is an intentional CLI
  compatibility break within this Track's scope.
- Decision (`-o` CLI contract): User approved that `-o` is rejected as an
  unknown option, prints usage, exits non-zero, and has no output-file side
  effect. This is not Q1; Q1 remains the CoreAudio lifecycle decision.
- Decision (rendered-file export): User approved removing the legacy rendered-
  file export without a CoreAudio or other replacement. Any future export is
  deferred to a later File I/O component.
- Decision (SDL boundary): User approved retaining SDL for a future GUI only;
  Track 015 removes every current SDL audio API use and dependencies justified
  solely by the retired audio route. This Track does not implement GUI behavior
  or decide its future SDL integration.
- Decision (Q1): User approved the callback-driven render model on 2026-08-22.
  CoreAudio drives a private render boundary with the requested frame count;
  the renderer uses lifecycle-preallocated storage, fills that request from
  current playback/voice state, and performs no allocation, locking, file I/O,
  or UI work. Existing module playback is the only current source. Later inputs
  may provide time-ordered events rather than pre-rendered audio. Callback
  ownership and lifecycle details remain Q2.
- Decision (legacy tick rendering): User approved the existing-module renderer
  model on 2026-08-22. It retains the remaining-frame count for the current
  TFMX tick; when exhausted, it advances TFMX exactly once, derives the next
  tick's frame count, and keeps mixing until it fills the exact device request.
  A request may span ticks and a tick may span requests. This is not an audio
  queue; preserving its tick timing is bounded Phase 4 compatibility evidence.
- Decision (Q2): User approved the lifecycle ownership policy on 2026-08-22.
  The application prepares module and callback context before start. The active
  callback has exclusive mutable access to legacy playback, timing, voice, and
  mix state. Control requests stop at a render-block boundary; callback
  quiescence precedes device disposal and module teardown. CoreAudio API,
  detailed error/change handling, and concrete lifecycle states remain deferred.
- Decision (Q3 sample rate): User approved the negotiated-device-rate policy on
  2026-08-22. Pre-start preparation configures the private renderer at the
  active output device's rate for that run; it neither forces 44.1 kHz nor adds
  a resampler. Frame Blocks carry no rate metadata. Rate changes require
  quiescence and a later reconfiguration/restart policy.
- Decision (Q3 channel/format): User approved the strict stereo policy on
  2026-08-22. The adapter accepts exactly two output channels in interleaved
  Float32, directly maps Frame Block left/right values, and rejects unsupported
  device configurations. It adds no channel mapping, mono fold-down, surround
  upmix, device-native representation, or Frame Block metadata.
- Decision (live input): User approved deferring live MIDI and other live-input
  implementation. Track 015 integrates existing module playback only, while its
  private render boundary must not foreclose later time-ordered event sources.
- Decision (architecture record): User approved ADR-009 and the Audio Rendering
  Design on 2026-08-22. They formalize Q1's callback-driven direction and its
  evidence, while retaining this Track's unresolved implementation decisions.
- Decision (ADR-010 private demand coordination): User approved ADR-010 on
  2026-08-23. The native CoreAudio adapter owns device callback registration,
  lifecycle, and Float32 conversion. For each device request of N frames, it
  calls private `audio_output` coordination, which synchronously requests
  exactly N Audio Frame Block frames from the private renderer and immediately
  routes the result to that adapter. This adds no queue, timer, CoreAudio code
  in `audio_output`, public API, public Audio Output Port, or Mixer extraction.
- Decision (S5.1 exact-N render policy): User approved the S5.1 policy on
  2026-08-23. The private `audio_output` exact-N coordinator requests exactly N
  frames from its renderer; if the returned block is not exactly N frames, or a
  nonzero block lacks frame storage, it rejects without adapter delivery,
  padding, or truncation. This creates no public port, queue, timer, callback,
  device behavior, or Mixer boundary.
- Decision (Q4): User approved a private CoreAudio system-call façade and
  test-only fake on 2026-08-23. Production uses the private façade; tests use
  the fake to control lifecycle outcomes, negotiated format, and zero or
  variable render requests without hardware. It creates no public or generic
  device API.
- Decision (Q5): User approved removal of the strict-profile gate and the
  `-b`, `-8`, `-f`, `-o`, `-w`, and `-v` options on 2026-08-23. The dependent
  file-output, stereo-blend, low-pass-filter, and oversampling behavior retires.
  Each removed option is an ordinary unknown option. Negotiated-rate, strict
  stereo CoreAudio configuration remains the live-device policy.
- Decision (Q6): User approved full current SDL-audio retirement on 2026-08-23:
  all SDL audio uses, audio-only dependencies, SDL test scaffolding and branding,
  legacy file output, and accidental SDL-header dependencies retire. No current
  SDL dependency has an approved non-audio justification. SDL remains a future
  GUI decision only.
- Decision (Q7): User approved bounded compatibility evidence on 2026-08-23:
  retain the four valid fixture pairs and malformed-fixture coverage; add
  deterministic fake-device callback tests at 44.1 and 48 kHz for zero/variable
  requests, frame retention, and exactly-once tick advance; prove non-silent
  stereo Float32 fixture-to-callback output; record a supplemental macOS
  on-device smoke check; and test removed-option rejection, including no `-o`
  file side effect. It is not a format-wide or bit-identical promise.
- Decision (version impact): Initial decision complete on 2026-08-23; revised
  for the approved startup negotiated-rate handoff and private legacy amplitude
  normalization on 2026-08-24.

  | Dimension | Impact | Reason |
  | --- | --- | --- |
  | C API/ABI | Unchanged | The CoreAudio façade and fake remain private; no public header, export, library, or target is introduced. |
  | Module compatibility/extension | Unchanged | No TFMX bytes, loader format, module extension, or persistence changes; bounded fixture evidence records intended impact. |
  | Interpreter/timing/audio behavior | Changed | A private unspecified startup-rate request selects the verified active 44.1 or 48 kHz device rate before activation and initializes tick/sample scheduling at that rate. The legacy producer corrects its historical signed-16-scale mixed values into signed-32 Frame Blocks with explicit signed-16 wrapping for multimode overflow before range expansion; no resampling or runtime rate-change handling is added, and blend, filter, and oversampling behavior remains retired. |
  | Persistent DAW format/versioning | Unchanged | This Track adds or changes no DAW persistence, schema, or version field. |
  | Platform/audio-output adapter | Changed | The private strict-stereo CoreAudio facade reports its verified 44.1/48 kHz format to the adapter, which performs control-side renderer preparation before bind/start; its generic signed-32-to-Float32 conversion responsibility is unchanged. This replaces the SDL-era audio route and its dependencies. |
  | Component/package boundaries | Changed | ADR-010 continues to assign private synchronous N-frame request coordination to `audio_output`; the adapter route gains a private negotiated-format preparation handoff while native adapters retain device ownership, callbacks, lifecycle, and conversion. The private legacy producer gains its own signed-16-to-signed-32 normalization responsibility; no public package boundary is added. |
- Decision (external-module loader deferral): User approved on 2026-08-24 that
  Track 015 will finish with its existing bounded four-fixture and malformed
  fixture compatibility evidence. The recorded XOut2 `TFMX_LOAD_INVALID_FORMAT`
  result is a private loader-layout limitation to address in a separate
  loader-focused Track after Track 015, not an assertion that the external
  module is malformed. No loader, player, module-byte, extension, public API,
  CoreAudio, or SDL behavior changes in this Track under this decision; the
  existing module compatibility/extension version-impact assessment remains
  unchanged.
- Decision (startup negotiated-rate handoff): User approved on 2026-08-24 the
  private adapter-managed control-side preparation design with an explicit
  success/failure result. After the CoreAudio facade has opened the default
  device and successfully configured a verified strict-stereo Float32 44.1 or
  48 kHz format, the adapter invokes the application-supplied private route
  preparation callback with that format. Preparation initializes the legacy
  exact-N renderer at that rate before bind, callback admission, or start. A
  preparation failure rolls the still-inactive route back through adapter-owned
  disposal to INACTIVE; CoreAudio facade/adapter ownership of device selection,
  negotiation, lifecycle, callback, and conversion remains unchanged, and
  `audio_output` remains the synchronous exact-N coordinator. This introduces
  no public API, loader change, resampler, or runtime device-rate-change policy.
- Decision (startup unspecified-rate request): User approved on 2026-08-24 the
  correction required for a real 48 kHz default device. `sample_rate_hz == 0`
  is an adapter-private startup-selection sentinel only: the application passes
  it with the otherwise strict stereo Float32 format; after device open the
  facade accepts only an actual nominal 44.1 or 48 kHz rate and reports that
  exact configured format to the adapter for preparation. Explicit 44.1/48 kHz
  private requests retain equality checking; any other nonzero request remains
  preflight-invalid. The fake must model the same wildcard/explicit behavior.
  This does not add Frame Block rate metadata, a public API, a resampler, or a
  runtime device-rate-change policy.
- Decision (legacy amplitude normalization): User approved on 2026-08-24 that
  `playback_legacy_mixer_render_frames` is the sole private legacy-producer
  owner of amplitude normalization before Audio Frame Blocks. It first reduces
  each legacy mixed lane deterministically into the historical signed-16 PCM
  domain by two's-complement modulo wrapping, including current multimode
  overflow, then expands that signed-16 value into signed-32 range by
  multiplication by 65536. The generic adapter continues to divide true
  signed-32 Frame Blocks by 2^31. This preserves the normal legacy PCM amplitude
  (`n / 32768`) without changing Frame Block metadata or defining a numerical
  policy for future producers. `voices_01` is the bounded oracle: its first
  historical PCM frame L=1680/R=1392 must become Float32 L=1680/32768 and
  R=1392/32768. Low-pass behavior and PCM serialization remain retired; no
  resampler or runtime rate-change policy is introduced.
- Decision (audible smoke fixture): User approved on 2026-08-24 that the
  existing self-authored `voices_01` pair is Track 015's dedicated supplemental
  audible smoke fixture after S6.4b proves its exact audible-scale oracle. Its
  zero-mean `+64/-64` sample payload, two active voices, and bounded active run
  make it preferable to `step8` for listener confirmation. This designation
  adds no fifth fixture, loader admission rule, or general module-compatibility
  claim; automated evidence remains primary and the on-device smoke remains
  supplemental.
- Decision (S5.2 device-free adapter-owned demand handoff): User approved the
  S5.2 policy on 2026-08-23: C API/ABI unchanged (private only); module
  compatibility/extension unchanged; interpreter/timing/audio unchanged because
  this is test-double handoff only with no legacy renderer or live route;
  persistent DAW format/versioning unchanged; platform/audio-output adapter
  changed privately (bind-before-start and active-only fake request seam, no
  real device); component/package boundary changed privately (adapter-owned
  request entry invokes the existing `audio_output` exact-N coordinator). No
  public port/API, CoreAudio framework/device, conversion, callback/render
  integration, application, or SDL work is included.
- Decision (S5.3 deterministic deferred stop to quiescence): User approved the
  S5.3 policy on 2026-08-23: C API/ABI unchanged (private only); module
  compatibility/extension unchanged; interpreter/timing/audio unchanged because
  this is fake-route/test-only behavior bounded by the current block completion
  policy; persistent DAW format/versioning unchanged; platform/audio-output
  adapter changed privately (active→stop-requested→quiescent with fake source
  quiescence after the current request); component/package boundaries changed
  privately (adapter lifecycle and stop request source around the existing
   coordinator). No locks/threads, real CoreAudio or device stop/close,
   conversion/workspace, legacy renderer/module teardown, application/SDL/CLI,
   public API/port, queue/timer, or rate-change work is included.
- Decision (S5.4 borrowed preallocated conversion workspace): User approved the
  S5.4 policy on 2026-08-23: C API/ABI unchanged (private only); module
  compatibility/extension unchanged; interpreter/timing/audio unchanged because
  this is fake-route/test-only conversion behavior; persistent DAW
  format/versioning unchanged; platform/audio-output adapter changed privately
  (pre-start borrowed Float32 workspace with a capacity bound and oversize
  rejection, no active allocation); component/package boundary changed privately
  (the adapter owns conversion delivery, while `audio_output` remains the
  exact-N coordinator). Capacity is greater than zero and the caller supplies
  valid writable storage of exactly `2 * capacity` through quiescence; there is
  no allocation, free, or reallocation, and S5.4 defers the invalid-preparation,
  release, and close contract. No device/framework, locks/threads,
  legacy/audio, application/SDL, public API/port, queue/timer, or rate-change
  work is included.
- Decision (S5.5 invalid borrowed workspace rejection): User approved the S5.5
  policy on 2026-08-23: C API/ABI unchanged (private only); module
  compatibility/extension unchanged; interpreter/timing/audio unchanged because
  rejection occurs pre-activation with no renderer; persistent DAW
  format/versioning unchanged; platform/audio-output adapter changed privately
  (workspace preparation requires non-NULL samples and capacity greater than
  zero, with `2 * capacity` bounds checked, and invalid preparation is rejected
  pre-activation); component/package boundary unchanged (existing adapter
  workspace delivery mode only). The direct mode remains valid with an unused
  workspace; invalid preparation rejects before open/configure/bind/start, and
  no release/close/device/locks/legacy/application/SDL/public/queue/rate work is
  included.
- Decision (S5.6 rate-configured exact-N tick scheduler): User approved the
  S5.6 policy on 2026-08-23: C API/ABI unchanged (private only); module
  compatibility/extension unchanged; interpreter/timing/audio legacy behavior
  unchanged because this is a deterministic test double with schedule
  arithmetic evidence only; persistent DAW format/versioning unchanged;
  platform/audio-output adapter unchanged; component/package boundary changed
  privately (a private rate-configured exact-N tick scheduler with a
  deterministic test source). It prepares 44.1 and 48 kHz rates, does not
  advance on a zero-frame request, retains the partial tick, and performs the
  next advance exactly once; Q7 fixture and real mixing evidence remain
  outstanding. No module loading, `tfmxIrqIn`, audio.c, mixer, adapter,
  conversion, device, SDL, application, public API/port, locks, queues, or
  multi-tick behavior is included. Under the approved finite S5 replan, this
  S5.6 decision folds into finite-replan chunk 1 (the real legacy exact-N
  renderer) and is not implemented as a standalone scheduler.
- Decision (finite S5 replan): User approved a Track-only finite replan on
  2026-08-23: replace the open-ended S5 wording with exactly three remaining
  completion-oriented chunks: (1) the real legacy exact-N renderer with
  active-rate 44.1/48 kHz fixture and Q7 evidence, folding the standalone S5.6
  scheduler work into it; (2) the real private CoreAudio lifecycle/callback
  with control-side quiescence; (3) application cutover plus SDL, `-o`, and
  CLI deletion. S6 is validation-only and S7 is completion. Public
  port/Mixer/event-abstraction/live-input/hotplug/resampling/non-macOS/
  export/GUI/bit-identical compatibility remain deferred.
- Decision (S5 chunk 1 real legacy exact-N renderer): User approved the S5
  chunk-1 policy on 2026-08-23: C API/ABI, module compatibility/extension, and
  persistent DAW format/versioning unchanged; interpreter/timing/audio behavior
  intentionally changes privately (active-rate 44.1/48 kHz scheduling from the
  real legacy exact-N renderer, with bounded fixture and Q7 evidence);
  platform/audio-output adapter unchanged; component/package boundary changes
  privately (the real legacy renderer feeds the existing exact-N coordinator at
  the active rate). No device lifecycle, application, SDL/CLI, or public API
  work is included, and no bit-identical compatibility promise is made.
- Decision (S5 chunk 2 HAL Output Audio Unit): User approved the S5 chunk-2
  device API policy on 2026-08-23: the real private CoreAudio
  lifecycle/callback with control-side quiescence uses the macOS HAL Output
  Audio Unit as its private system API. C API/ABI unchanged (private only);
  module compatibility/extension unchanged; interpreter/timing/audio behavior
  changes only on the real active route (the HAL Output Audio Unit render
  callback drives the existing exact-N coordinator at the active rate);
  persistent DAW format/versioning unchanged; platform/audio-output adapter
  changed privately (HAL Output Audio Unit lifecycle and callback with
  control-side quiescence replace the fake-driven route); component/package
  boundaries changed privately (HAL Output Audio Unit callback, lifecycle, and
  conversion remain adapter-owned). No public API/port, resampler,
  application, SDL/CLI, or bit-identical compatibility work is included.
- Decision (S5 chunk 2 negotiated-rate strict-stereo gate): User approved the
  S5 chunk-2 rate and format policy on 2026-08-23: the adapter negotiates the
  active HAL Output Audio Unit rate, requires strict interleaved Float32
  stereo, and rejects any negotiated rate other than 44.1 or 48 kHz before
  activation, with no resampler. C API/ABI unchanged (private only); module
  compatibility/extension unchanged; interpreter/timing/audio behavior changes
  only on the real active route (44.1/48 kHz negotiated-rate rendering with
  bounded fixture and Q7 evidence); persistent DAW format/versioning unchanged;
  platform/audio-output adapter changed privately (pre-activation rate gate
  over the existing strict stereo interleaved Float32 acceptance);
  component/package boundaries changed privately (rate/format rejection
  remains adapter-private and adds no public boundary). No device-rate-change
  restart policy, resampler, application, SDL/CLI, or public API work is
  included.
- Decision (S5 chunk-2 lock-free OPEN/IN_FLIGHT admission gate and real HAL
  workspace-only delivery): User approved the S5 chunk-2 admission and delivery
  policy on 2026-08-23: the real HAL Output Audio Unit route admits callback
  requests through a private lock-free OPEN/IN_FLIGHT admission gate that
  performs no locking, blocking, or allocation, and converts through the
  preallocated workspace-only mode; the direct delivery mode remains valid only
  for the fake/test route. C API/ABI unchanged (private only; no public header,
  export, library, or target); module compatibility/extension unchanged (no TFMX
  bytes, loader, extension, or persistence changes); interpreter/timing/audio
  behavior changes only on the real active route (lock-free admission-gated
  exact-N rendering with workspace-only conversion at the active rate);
  persistent DAW format/versioning unchanged; platform/audio-output adapter
  changed privately (lock-free OPEN/IN_FLIGHT admission gate and workspace-only
  conversion on the real HAL Output Audio Unit route); component/package
  boundaries changed privately (admission gate, callback, lifecycle, and
  conversion remain adapter-owned; the direct mode is preserved for the fake
  route only). No locks, threads, queue, timer, public boundary, resampler,
  application, SDL/CLI, or bit-identical compatibility work is included.
- Decision (S5 chunk-2 HAL workspace-copy route): User approved the S5 chunk-2
  workspace-copy policy on 2026-08-23: C API/ABI unchanged (private only; no
  public header, export, library, or target); module compatibility/extension
  unchanged (no TFMX bytes, loader, extension, or persistence changes);
  interpreter/timing/audio unchanged because this is fake-route/test-only
  behavior with no renderer or legacy change; persistent DAW format/versioning
  unchanged; platform/audio-output adapter changed privately (the real HAL
  Output Audio Unit route converts signed-32 mixed values through the validated
  borrowed Float32 workspace and then copies the converted samples into the
  separate native output buffer; malformed (NULL or sample-count-overflow
  capacity) and undersized (capacity below the request) native buffers are
  rejected before any render, delivery, or copy, and a zero-frame request
  requires no native buffer and is accepted without rendering);
  component/package boundary changed privately (adapter-owned request-entry
  validation, conversion, and copy around the existing exact-N coordinator).
  The fake route keeps its current in-buffer conversion behavior. No device,
  locks/threads, resampler, application, SDL/CLI, public API/port, queue/timer,
  or bit-identical compatibility work is included.
- Decision (best-effort teardown): User approved the best-effort teardown
  policy on 2026-08-23: on a stop, quiesce, or dispose failure, the adapter
  still attempts the remaining cleanup, returns the first failure, and reaches
  a terminal inactive/error state with no retry. C API/ABI unchanged (private
  only); module compatibility/extension unchanged; interpreter/timing/audio
  unchanged because this governs control-side teardown only; persistent DAW
  format/versioning unchanged; platform/audio-output adapter changed privately
  (best-effort stop→quiesce→dispose with first-failure return and terminal
  inactive/error state); component/package boundary changed privately
  (adapter-owned teardown with no public boundary).

Plan (execution steps)
- [x] S1) Resolve Q4-Q7; record the approved architecture, test strategy,
  compatibility evidence, and complete version-impact decision while DRAFT.
- [x] S2) Move Track 015 to ACTIVE (folder, filename, and title status) only
  after user approval and check this Move-to-ACTIVE plan step.
- [x] S3) Define the first focused observable CoreAudio lifecycle/render-path
  contract and add its failing automated test. The first focused contract is
  `test_audio_output`'s `coreaudio_lifecycle_accepts_only_strict_stereo_float32`,
  driven by the test-only `fake_coreaudio_facade`. Its strict stereo interleaved
  Float32 acceptance intent covers 44.1 and 48 kHz, and it rejects an
  unsupported format or a façade failure before activation/render. It was
  deterministic runtime-red at the 44.1 kHz acceptance assertion (`expected
  STARTED, got START_UNAVAILABLE`); S4 resolved it green.
- [x] S4) Implement the smallest private CoreAudio route change that passes the
  approved S3 test; refactor and validate.
- [x] S5) Execute the remaining approved TDD chunks in this finite,
  completion-oriented order, updating the Track after each meaningful chunk:
  (1) real legacy exact-N renderer with active-rate 44.1/48 kHz fixture and
  Q7 evidence, folding the standalone S5.6 scheduler work into it; (2) real
  private CoreAudio lifecycle/callback with control-side quiescence; (3)
  application cutover plus SDL, `-o`, and CLI deletion.
- [x] S6) Validation only, no implementation: run relevant component,
   application, integration, and compatibility validation; capture results,
   inspect the linked roadmap, and reconcile it if the outcome materially
   changes it.
- [x] S6.1) Record the approved startup negotiated-rate replan, private
  failure-returning preparation contract, and revised six-dimension impact;
  retain completed S5/S6 evidence as the historical baseline.
- [x] S6.2) TDD chunk: prove private negotiated-format to control-side
  preparation ordering and rollback. A fake verified 44.1/48 kHz strict-stereo
  format must prepare exactly once after configure and before bind/start; no
  callback request may precede preparation. A preflight invalid requested route
  format must reject before facade lifecycle; a configure failure, unsupported
  reported negotiated format, or preparation failure must dispose to INACTIVE
  with no bind/start/render/delivery, while bind/start rollback remains covered.
- [x] S6.2a) TDD correction: prove the private unspecified startup-rate request
  admits only a facade-reported 44.1/48 kHz configured format, while explicit
  44.1/48 requests retain equality checks and other nonzero requests reject
  before lifecycle. The production facade and fake must share this behavior;
  a reported 48 kHz format must prepare/bind/start rather than fail because the
  application had selected 44.1 kHz before device open.
- [x] S6.3) TDD chunk: prove negotiated 44.1/48 kHz preparation drives the real
  exact-N renderer across the four approved fixtures, retaining zero-request,
  partial-tick, exactly-once advance, continuity, non-silent stereo Float32,
  and no-allocation evidence.
- [x] S6.4) TDD chunk: prove application composition supports a workspace-only
  fake default 48 kHz route using the unspecified startup-rate request (assert
  the application's requested rate is `0`) and without fixed-44.1 renderer
  initialization, through one
  open/configure/prepare/bind/start and normal stop/quiesce/dispose.
- [x] S6.4a) Record the approved private legacy-producer amplitude-normalization
  replan, explicit signed-16 wrapping policy for current multimode overflow,
  exact `voices_01` Float32 oracle, and revised six-dimension impact.
- [x] S6.4b) TDD chunk: prove `playback_legacy_mixer_render_frames` normalizes
  the legacy signed-16 PCM domain safely and deterministically into signed-32
  Frame Blocks before the generic adapter. Cover negative, zero, positive, and
  endpoint mapping; explicit unsigned-low-16 reconstruction for the approved
  multimode wrap policy (including 32768→-32768); and the exact first
  `voices_01` historical PCM frame L=1680/32768 and R=1392/32768 Float32 result
  through the real renderer/adapter route at both 44.1 and 48 kHz, retaining
  exact-N, timing, continuity, and no-allocation evidence. Establish
  `voices_01` as the bounded audible-smoke fixture without adding a loader
  layout.
- [x] S6.5) Validation and documentation only: run relevant component,
  application, integration, and bounded compatibility validation; reconcile the
  required documentation, run the supplemental real-device `voices_01` smoke,
  and recheck affected acceptance/milestone evidence.
- [x] S7) Completion only: on acceptance, move Track 015 to COMPLETED and
  check the completion transition step.

Current inventory
- `src/audio_output/audio_output.c/.h` defines the private Audio Frame Block
  boundary and dispatch. ADR-010 assigns its Track 015 private role as the
  synchronous N-frame demand coordinator; it has no native device callback,
  timer, queue, or native CoreAudio framework, lifecycle, or conversion code.
  The S5.1 exact-N coordinator implementation is green; its current conditional
  CoreAudio adapter dispatch is pre-Track-015 code to refactor into the
  approved responsibility split.
- `src/audio_output/adapters/coreaudio_adapter.c/.h` converts signed-32 mixed
  values to adapter-private interleaved Float32. Its request entry runs a
  lock-free OPEN/IN_FLIGHT admission gate and, for a `workspace_only` facade,
  a HAL workspace-copy route: malformed (NULL or sample-count-overflow
  capacity) and undersized (capacity below the request) native buffers are
  rejected pre-render, zero-frame requests are accepted without a buffer,
  converted samples are copied into the separate native buffer before the
  callback admission is released, and the direct delivery mode is valid only
  for the fake/test route. Instance start rolls back on open/configure/bind/
  start failure through admission close/drain and dispose before returning
  INACTIVE; teardown is best-effort with first-failure return and a terminal
  inactive/error state.
- `src/audio_output/adapters/coreaudio_facade.c/.h` is the adapter-private
  CoreAudio system-call façade. Its production default is now the real macOS
  HAL Output Audio Unit route: open (HAL Output Audio Unit with the default
  output device, `EnableIO` input bus 1 / output bus 0, `CurrentDevice`),
  configure (device nominal rate must equal the requested 44.1 or 48 kHz
  strict interleaved Float32 stereo stream format, set and re-read), bind
  (`SetRenderCallback`), start (open callback admission,
  `AudioUnitInitialize`, `AudioOutputUnitStart`), stop (drain callbacks,
  `AudioOutputUnitStop`), quiesce (drain), and dispose (quiescent/CLOSED only;
  uninitialize and dispose). The HAL render callback admits through a lock-free
  OPEN→IN_FLIGHT atomic exchange, validates the native buffer, drives the bound
  exact-N request, and releases admission only after the request returns. The
  non-APPLE default remains `UNAVAILABLE`.
- `src/audio.c` and `src/tfmx.c` are deleted; the SDL-era live callback,
  conversion/ring-buffer, throttle/drain, and synchronization machinery, and
  the legacy file-output and strict-profile paths are retired.
- `src/application.c/.h` now runs the private CoreAudio live route directly:
  `run_live_output` composes the legacy exact-N renderer with the adapter
  instance in workspace delivery mode, starts it via
  `audio_output_coreaudio_adapter_start_instance` (production default HAL
  facade), waits on stop/interrupt/completion, and stops via
  `audio_output_coreaudio_adapter_stop_instance`. The `-b`, `-8`, `-f`, `-o`,
  `-w`, and `-v` options are removed; each is an ordinary unknown option that
  prints usage and exits non-zero with no file side effect.
- `CMakeLists.txt` no longer discovers or links SDL; the CLI identity test's
  `FAIL_REGULAR_EXPRESSION` includes `tfmxplay|tfmx-play|/SDL`. APPLE-gated
  CoreAudio frameworks (AudioToolbox, AudioUnit, CoreAudio) are linked for the
  executable and relevant tests.
- Relevant tests include `tests/audio_output/test_audio_output.c` (31 tests),
  `tests/audio_output/test_legacy_exact_renderer.c`,
  `tests/application/test_application.c`,
  `tests/application/test_application_audio_lifecycle.c`, and
  `tests/application/test_application_removed_options.c`.
  `tests/audio_output/test_audio_routing.c`,
  `tests/audio_output/recording_sink.c/.h`, and
  `tests/application/test_application_strict_profile.c` are deleted with the
  retired routes.
- S3 evidence: `tests/audio_output/test_audio_output.c` now contains
  `coreaudio_lifecycle_accepts_only_strict_stereo_float32`, driven by the
  test-only `fake_coreaudio_facade`. Configure and build passed; the focused
  run `ctest --test-dir build -R '^test_audio_output$' --output-on-failure`
  produced the expected deterministic runtime-red failure at the 44.1 kHz
  acceptance assertion (`expected STARTED, got START_UNAVAILABLE`). Independent
  review confirmed no hardware use and no out-of-scope behavior.
- S4 evidence: the focused test is green. The injected private façade lifecycle
  accepts 44.1/48 kHz strict-stereo interleaved Float32 only after exactly
  open→configure→start; rejected format and façade-failure cases short-circuit
  with exact trace/call-count evidence before activation/render. The production
  default façade remains `START_UNAVAILABLE`/device-free, and no callback,
  render, or device behavior is implemented. Focused validation
  `cmake --build build --target test_audio_output --parallel 2 && ctest --test-dir build -R '^test_audio_output$' --output-on-failure`
  passed 1/1, and independent focused verification passed.
- S5.1 evidence: `tests/audio_output/test_audio_output.c` now contains
  `coordinator_accepts_zero_and_variable_exact_requests_and_rejects`, and the
  exact-N coordinator implementation is green. The coordinator synchronously
  calls its renderer with exactly N frames; a returned block whose frame count
  differs from N, or a nonzero block with NULL frame storage, is rejected
  without delivery, padding, or truncation; otherwise it delivers once with the
  same borrowed block and returns the delivery result. The test covers zero and
  variable 1/3/2 requests plus short/long and NULL-frame rejection cases; its
  delivery fake snapshots the temporary block wrapper by value while preserving
  payload pointer identity. Focused validation
  `cmake --build build --target test_audio_output --parallel 2 && ctest --test-dir build -R '^test_audio_output$' --output-on-failure`
  passed 1/1, and independent focused verification confirmed 13/13. S5 remains
  unchecked; later callback, lifecycle, application, and SDL work remains
  deferred.
- S5.2 evidence: `tests/audio_output/test_audio_output.c` now contains
  `coreaudio_bound_instance_hands_off_exact_requests_without_conversion`,
  green. The private adapter-owned bound instance validates the strict format
  and complete route: it performs exactly open→configure→bind request→start,
  with unsupported-format and failure cases short-circuiting before activation,
  and it becomes active only after a successful start. Active fake requests
  0/1/3/2 invoke the existing exact-N coordinator and non-converting sink with
  payload identity; a failed or inactive request reaches neither renderer nor
  sink. Focused validation
  `cmake --build build --target test_audio_output --parallel 2 && ctest --test-dir build -R '^test_audio_output$' --output-on-failure`
  passed 1/1, and independent focused verification passed. The S4 stateless
  start is unchanged; no device, conversion, allocation, legacy, application,
  SDL, or public API work is included. S5 remains unchecked; later lifecycle,
  quiescence, and real route work remains deferred.
- S5.3 evidence: `tests/audio_output/test_audio_output.c` now contains
  `coreaudio_stop_request_quiesces_after_current_request`, green. The private
  adapter-owned bound instance tracks active→stop-requested→quiescent with an
  in-request guard. A stop requested during a fake active N=3 request still
  completes that exact-N sink delivery exactly once with payload identity;
  after the coordinator returns the in-request guard clears, the instance
  becomes QUIESCENT, and the fake source quiesces. A later fake source request
  is refused at the source before any callback/render/sink work. No conversion
  path is exercised; real-device shutdown/threads and broader deferred work
  remain out of scope. Focused validation
  `cmake --build build --target test_audio_output --parallel 2 && ctest --test-dir build -R '^test_audio_output$' --output-on-failure`
  passed 1/1, and independent focused verification confirmed 15/15. S4 and
  S5.2 evidence is unchanged; S5 remains unchecked.
- S5.4 evidence: `tests/audio_output/test_audio_output.c` now contains
  `coreaudio_workspace_instance_converts_borrowed_capacity_and_rejects_oversize`,
  green. A separate workspace delivery mode preserves the S5.2 direct route; the
  borrowed static Float32 workspace has capacity 3. A 0-frame request is
  accepted with no observer delivery; 1- and 3-frame requests convert signed-32
  left/right to interleaved Float32 in place with payload pointer identity and
  zero allocation; an oversize 4-frame request is rejected before
  observer/write. Focused validation
  `cmake --build build --target test_audio_output --parallel 2 && ctest --test-dir build -R '^test_audio_output$' --output-on-failure`
  passed 1/1, and independent review confirmed the scope. The old stateless
  submit path remains separate and allocating; no device/framework,
  locks/threads, workspace release/close, legacy, application/SDL/CLI, public
  API/port, queue/timer, or rate-change work is included. Invalid workspace
  preparation and release/close remain deferred. S5 remains unchecked.
- S5.5 evidence: `tests/audio_output/test_audio_output.c` now contains
  `coreaudio_workspace_start_rejects_invalid_preparation_before_activation`,
  green. It encodes NULL samples, zero capacity, and capacity greater than
  `SIZE_MAX / 2`; each invalid preparation is rejected before facade lifecycle,
  bind, request, render, observer, or allocation work, with the instance
  INACTIVE and no in-request state. The direct S5.2 mode remains valid. Focused
  validation
  `cmake --build build --target test_audio_output --parallel 2 && ctest --test-dir build -R '^test_audio_output$' --output-on-failure`
  passed 1/1, and independent focused verification confirmed 17/17. Invalid
  storage-length proof and workspace release/close remain deferred; no
  device/locks/legacy/application/SDL/public scope is included. S5 remains
  unchecked.
- S5 chunk-1 evidence: `tests/audio_output/test_legacy_exact_renderer.c` now
  contains `fixture_driven_legacy_renderer_fulfills_exact_requests`, green. The
  finite chunk-1 renderer drives the existing exact-N coordinator over the four
  valid fixtures (`step8`, `loop_f1`, `envelope_tempo`, `voices_01`) at 44.1
  and 48 kHz (8 cases) with the request sequence 0, 7, 3, first-tick-frames +
  5, 1. The focused run passes 8/8 with the real legacy exact-N renderer:
  exact-N fulfillment per request, zero-frame requests with no tick advance,
  partial-tick retention and continuity across requests, exactly-once tick
  advance when the retained tick is exhausted, non-silent stereo Float32
  fixture-to-callback output, and zero adapter allocation. Focused validation
  `cmake --build build --target test_legacy_exact_renderer --parallel 2 && ctest --test-dir build -R '^test_legacy_exact_renderer$' --output-on-failure`
  passed 1/1 with all 8 fixture/rate cases green. The earlier deterministic
  runtime-red 0/1 record ("8 fixture/rate cases are still missing exact-N
  legacy rendering; first failure: step8 at 44100 Hz (ticks=4,
  last_tick_frames=882, expected=882)") and the test-only probe composition
  remain in the completion notes as the TDD red evidence. No device lifecycle,
  application, SDL/CLI, or public API work is included; the remaining chunk-1
  Q7 evidence and the S5.2-S5.5 adapter evidence are unchanged. S5 remains
  unchecked.
- S5 chunk-2 evidence: `tests/audio_output/test_audio_output.c` now contains
  `coreaudio_control_lifecycle_waits_for_callback_quiescence`, green. Control
  stop→quiesce→dispose occurs strictly after callback quiescence: a control
  stop requested during a fake callback request of 3 frames completes the
  current block exactly once (render 1, delivery 1, payload identity), and only
  then does control perform the facade lifecycle STOP, QUIESCE, DISPOSE. The
  lifecycle trace is exactly OPEN, CONFIGURE, BIND_REQUEST, START, STOP,
  QUIESCE, DISPOSE (7 entries) with zero callback-side lifecycle calls: no
  in-request quiesce, and no stop or dispose during the callback. A later fake
  source request is refused before any render/delivery work. Focused validation
  `cmake --build build --target test_audio_output --parallel 2 && ctest --test-dir build -R '^test_audio_output$' --output-on-failure`
  passed 1/1 (18/18 tests in the binary), and the full project suite passed
  10/10. No HAL Output Audio Unit, application, SDL/CLI, or public API work is
  included; the chunk-1 Q7 evidence remains outstanding. S5 remains unchecked.

Artifacts
- Living roadmap: `SynthTracker modernization roadmap`, revision 20, and
  `Phase 4 — Component extraction`, revision 13, in project memory.
- Predecessor: `.backlog/COMPLETED/2026/TRACK_014_COMPLETED_coreaudio_adapter.md`.
- Governing design: `docs/adr/ADR-005-target-daw-component-foundation.md`,
  `docs/adr/ADR-008-audio-frame-block-mixed-value-boundary.md`,
  `docs/ASR.md` (ASR-006 and ASR-009), and `docs/TESTING.md`.
- Callback-driven rendering decision:
  `docs/adr/ADR-009-callback-driven-audio-rendering.md`,
  `docs/AUDIO_RENDERING_DESIGN.md`, and `docs/ASR.md` (ASR-010).
- Private demand-coordination decision:
  `docs/adr/ADR-010-native-adapter-ownership-and-private-demand-coordination.md`.

Completion notes
- S3 (2026-08-23): Defined and added the first focused failing test. Configure
  and build passed; the focused ctest run produced the expected deterministic
  runtime-red failure at the 44.1 kHz acceptance assertion (`expected STARTED,
  got START_UNAVAILABLE`). Independent review confirmed no hardware use and no
  out-of-scope behavior. S4 next: the smallest private CoreAudio route change
  that passes the approved S3 test.
- S4 (2026-08-23): Implemented the smallest private CoreAudio route change that
  passes the approved S3 test and validated it. The injected private façade
  lifecycle accepts 44.1/48 kHz strict-stereo interleaved Float32 only after
  exactly open→configure→start; rejected format and façade-failure cases
  short-circuit with exact trace/call-count evidence before activation/render.
  The production default façade remains `START_UNAVAILABLE`/device-free; no
  callback, render, or device behavior is implemented. Focused validation
  `cmake --build build --target test_audio_output --parallel 2 && ctest --test-dir build -R '^test_audio_output$' --output-on-failure`
  passed 1/1, and independent focused verification passed. S5 next: the
  remaining approved TDD chunks.
- S5.1 (2026-08-23): Implemented and validated the exact-N coordinator. It
  synchronously calls its renderer with exactly N frames; a returned block
  whose frame count differs from N, or a nonzero block with NULL frame storage,
  is rejected without delivery, padding, or truncation; otherwise it delivers
  once with the same borrowed block and returns the delivery result. Focused
  test `coordinator_accepts_zero_and_variable_exact_requests_and_rejects`
  covers zero and variable 1/3/2 requests plus short/long and NULL-frame
  rejection cases, with its delivery fake snapshotting the temporary block
  wrapper by value while preserving payload pointer identity. Focused
  validation
  `cmake --build build --target test_audio_output --parallel 2 && ctest --test-dir build -R '^test_audio_output$' --output-on-failure`
  passed 1/1, and independent focused verification confirmed 13/13. S5 remains
  unchecked; later callback, lifecycle, application, and SDL work remains
  deferred.
- S5.2 (2026-08-23): Implemented the smallest passing change and validated it
  green. `tests/audio_output/test_audio_output.c` now contains
  `coreaudio_bound_instance_hands_off_exact_requests_without_conversion`,
  green. The private adapter-owned bound instance validates the strict format
  and complete route: it performs exactly open→configure→bind request→start,
  with unsupported-format and failure cases short-circuiting before activation,
  and it becomes active only after a successful start. Active fake requests
  0/1/3/2 invoke the existing exact-N coordinator and non-converting sink with
  payload identity; a failed or inactive request reaches neither renderer nor
  sink. Focused validation
  `cmake --build build --target test_audio_output --parallel 2 && ctest --test-dir build -R '^test_audio_output$' --output-on-failure`
  passed 1/1, and independent focused verification passed. The S4 stateless
  start is unchanged; no device, conversion, allocation, legacy, application,
  SDL, or public API work is included. S5 remains unchecked; later lifecycle,
  quiescence, and real route work remains deferred.
- S5.3 (2026-08-23): Implemented the smallest passing change and validated it
  green. `tests/audio_output/test_audio_output.c` now contains
  `coreaudio_stop_request_quiesces_after_current_request`, green. The private
  adapter-owned bound instance tracks active→stop-requested→quiescent with an
  in-request guard. A stop requested during a fake active N=3 request completes
  that exact-N sink delivery exactly once with payload identity; after the
  coordinator returns the in-request guard clears, the instance becomes
  QUIESCENT, and the fake source quiesces. A later fake source request is
  refused at the source before callback/render/sink. No conversion path is
  exercised; real-device shutdown/threads and broader deferred work remain out
  of scope. Focused validation
  `cmake --build build --target test_audio_output --parallel 2 && ctest --test-dir build -R '^test_audio_output$' --output-on-failure`
  passed 1/1, and independent focused verification confirmed 15/15. S4 and
  S5.2 are unchanged; S5 remains unchecked.
- S5.4 (2026-08-23): Implemented the smallest passing change and validated it
  green.
  `coreaudio_workspace_instance_converts_borrowed_capacity_and_rejects_oversize`
  uses a separate workspace delivery mode that preserves the S5.2 direct route,
  with a borrowed static Float32 workspace of capacity 3. A 0-frame request is
  accepted with no observer delivery; 1- and 3-frame requests convert signed-32
  left/right to interleaved Float32 in place with payload pointer identity and
  zero allocation; an oversize 4-frame request is rejected before
  observer/write. Focused validation
  `cmake --build build --target test_audio_output --parallel 2 && ctest --test-dir build -R '^test_audio_output$' --output-on-failure`
  passed 1/1, and independent review confirmed the scope. The old stateless
  submit path remains separate and allocating; no device/framework,
  locks/threads, workspace release/close, legacy, application/SDL/CLI, public
  API/port, queue/timer, or rate-change work is included. Invalid workspace
  preparation and release/close remain deferred. S5 remains unchecked.
- S5.5 (2026-08-23): Implemented the smallest passing change and validated it
  green. `tests/audio_output/test_audio_output.c` now contains
  `coreaudio_workspace_start_rejects_invalid_preparation_before_activation`,
  green. Workspace-mode startup rejects NULL samples, zero capacity, and
  capacity greater than `SIZE_MAX / 2` before any facade lifecycle,
  bind/request/render/observer/allocation work; the instance stays INACTIVE
  with no in-request state, and the direct S5.2 route remains valid. Focused
  validation
  `cmake --build build --target test_audio_output --parallel 2 && ctest --test-dir build -R '^test_audio_output$' --output-on-failure`
  passed 1/1, and independent focused verification confirmed 17/17. Invalid
  storage-length proof and workspace release/close remain deferred; no
  device/locks/legacy/application/SDL/public scope is included. S5 remains
  unchecked.
- S5 chunk-1 (2026-08-23): Recorded the deterministic runtime-red evidence for
  the approved finite chunk-1 focused test
  `fixture_driven_legacy_renderer_fulfills_exact_requests` in
  `tests/audio_output/test_legacy_exact_renderer.c`. It exercises the
  zero/variable/tick-retention/once-advance/non-silent stereo Float32 contract
  across the four valid fixtures (`step8`, `loop_f1`, `envelope_tempo`,
  `voices_01`) at 44.1 and 48 kHz. The expected 8-case red occurs because the
  test-only `legacy_tick_probe_render` composition discards the partial tick
  instead of retaining it and advancing exactly once when exhausted; no
  renderer is introduced. Focused validation
  `cmake --build build --target test_legacy_exact_renderer --parallel 2 && ctest --test-dir build -R '^test_legacy_exact_renderer$' --output-on-failure`
  built cleanly and produced the expected deterministic 0/1 red with the
  message "8 fixture/rate cases are still missing exact-N legacy rendering;
  first failure: step8 at 44100 Hz (ticks=4, last_tick_frames=882,
  expected=882)". S5 remains unchecked; the real legacy exact-N renderer is
  the next green change.
- S5 chunk-1 green (2026-08-23): Implemented the smallest passing real legacy
  exact-N renderer and validated the finite chunk-1 focused test green. The
  renderer retains the partial tick across requests and advances exactly once
  when the retained tick is exhausted, fulfilling exact-N requests from the
  existing exact-N coordinator. Focused validation
  `cmake --build build --target test_legacy_exact_renderer --parallel 2 && ctest --test-dir build -R '^test_legacy_exact_renderer$' --output-on-failure`
  passed 1/1 with all 8 fixture/rate cases green (four valid fixtures at 44.1
  and 48 kHz): exact N, zero-frame no advance, retention/continuity, exactly
  once advance, non-silent stereo Float32 output, and zero adapter allocation.
  The S5.2-S5.5 adapter evidence is unchanged; no device lifecycle,
  application, SDL/CLI, or public API work is included. The remaining chunk-1
  Q7 evidence is still outstanding; S5 remains unchecked.
- S5 chunk-2 red (2026-08-23): Recorded the deterministic runtime-red evidence
  for the approved finite chunk-2 focused test
  `coreaudio_control_lifecycle_waits_for_callback_quiescence` in
  `tests/audio_output/test_audio_output.c`. It drives the control lifecycle
  with the fake facade: a callback request of 3 frames completes the current
  block exactly once (render 1, delivery 1, payload identity) while control
  requests stop during that callback, and then control performs
  stop→quiesce→dispose. The expected lifecycle trace is exactly OPEN, CONFIGURE,
  BIND_REQUEST, START, STOP, QUIESCE, DISPOSE (7 entries), with quiescence and
  disposal deferred until after callback quiescence. The current adapter
  auto-quiesces inside the request path (`adapter_request` transitions
  STOP_REQUESTED→QUIESCENT and calls the facade quiesce), so the lifecycle trace
  records an extra in-request QUIESCE entry: the count assertion is red at
  expected 8 vs 7 trace entries. Focused validation
  `cmake --build build --target test_audio_output --parallel 2 && ctest --test-dir build -R '^test_audio_output$' --output-on-failure`
  built cleanly and passed 17/18 with this single expected red. No HAL Output
  Audio Unit, application, SDL/CLI, or public API work is included; the
  chunk-1 Q7 evidence remains outstanding. S5 remains unchecked; the
  control-side deferred-quiescence change is the next green change.
- S5 chunk-2 green (2026-08-23): Implemented the smallest passing
  control-side deferred-quiescence change and validated the finite chunk-2
  focused test green. The adapter request path no longer auto-quiesces; a
  control stop requested at a render-block boundary completes the current
  block exactly once (render 1, delivery 1, payload identity), and control
  performs stop→quiesce→dispose only after callback quiescence, with zero
  callback-side lifecycle calls (lifecycle trace exactly OPEN, CONFIGURE,
  BIND_REQUEST, START, STOP, QUIESCE, DISPOSE). Focused validation
  `cmake --build build --target test_audio_output --parallel 2 && ctest --test-dir build -R '^test_audio_output$' --output-on-failure`
  passed 1/1 with all 18 binary tests green, and the full project suite passed
  10/10. No HAL Output Audio Unit, application, SDL/CLI, or public API work is
  included; the chunk-1 Q7 evidence remains outstanding. S5 remains
  unchecked.
- S5 chunk-2 admission red (2026-08-23): Recorded the deterministic
  runtime-red evidence for the three admission tests of the pending lock-free
  OPEN/IN_FLIGHT atomic admission gate chunk in
  `tests/audio_output/test_audio_output.c`:
  `coreaudio_reentrant_start_is_rejected_without_second_lifecycle`,
  `coreaudio_stop_admission_is_delayed_until_callback_exit`, and
  `coreaudio_hal_route_rejects_direct_delivery_mode`. Focused validation
  `cmake --build build --target test_audio_output --parallel 2 && ctest --test-dir build -R '^test_audio_output$' --output-on-failure`
  built cleanly and produced the expected deterministic 19/22 with exactly
  these three reds:
  - Reentrant start: the facade start callback reenters
    `audio_output_coreaudio_adapter_start_instance`; the assertion at
    `test_audio_output.c:1600` fails `--- 1 != 0`
    (`reentrant_start_result` is `START_STARTED`, expected
    `START_REJECTED`), so the adapter still admits a second full
    open→configure→bind→start lifecycle instead of rejecting reentrant
    admission at the gate.
  - Deferred stop admission: the assertion at `test_audio_output.c:1693` fails
    (`fake.deferred_stop_admission_result is not true`), so a control stop
    requested during the callback is not yet admitted only at callback exit.
  - Direct HAL route: `audio_output_coreaudio_adapter_start_instance` returns
    `START_STARTED` instead of `START_REJECTED` (assertion at
    `test_audio_output.c:1731` fails `--- 1 != 0`) for a workspace-only HAL
    route configured with `AUDIO_OUTPUT_COREAUDIO_ADAPTER_DELIVERY_DIRECT`.
  The lock-free OPEN/IN_FLIGHT atomic admission gate and the workspace-only
  HAL admission validation are pending; per the approved S5 chunk-2 decision,
  the direct delivery mode remains valid only for the fake/test route. No HAL
  Output Audio Unit, application, SDL/CLI, or public API work is included; the
  chunk-1 Q7 evidence remains outstanding. S5 remains unchecked; the smallest
  passing admission-gate change is the next green change.
- S5 chunk-2 admission green (2026-08-23): Implemented the smallest passing
  lock-free OPEN/IN_FLIGHT atomic admission gate change and validated the
  three admission tests green. `coreaudio_adapter.c` now opens callback
  admission only after bind and before facade start, admits each callback
  request through the lock-free OPEN→IN_FLIGHT exchange, releases it back to
  OPEN (or CLOSED while closing) on exit, drains admission through CLOSING on
  stop, and rejects reentrant start (`START_REJECTED` with no second
  lifecycle), delays a control stop requested during the callback until
  callback exit, and rejects the direct delivery mode for a workspace-only
  facade. The S5.2-S5.5, chunk-1, and chunk-2 quiescence evidence is
  unchanged. Focused validation
  `cmake --build build --target test_audio_output --parallel 2 && ctest --test-dir build -R '^test_audio_output$' --output-on-failure`
  passed 1/1 with all 22 binary tests green. No HAL Output Audio Unit,
  application, SDL/CLI, or public API work is included; the chunk-1 Q7
  evidence remains outstanding. S5 remains unchecked.
- S5 chunk-2 HAL workspace-copy red (2026-08-23): Recorded the deterministic
  runtime-red evidence for the approved HAL workspace-copy focused test
  `coreaudio_hal_route_converts_into_borrowed_workspace_then_copies_to_native_buffer`
  in `tests/audio_output/test_audio_output.c`. It drives a workspace-only
  facade (HAL route) with a validated borrowed workspace of capacity 3: a
  3-frame request with a NULL native buffer, with an undersized native buffer
  (capacity 2), and with a sample-count-overflow capacity (`SIZE_MAX / 2 + 1`)
  must each be rejected pre-render with no render/delivery/copy; a 0-frame
  request with no native buffer is accepted without render; and an exact
  3-frame request with a separate capacity-3 native buffer converts into the
  borrowed workspace (observer payload identity) and then copies the converted
  samples into the native buffer with zero allocation. Focused validation
  `cmake --build build --target test_audio_output --parallel 2 && ctest --test-dir build -R '^test_audio_output$' --output-on-failure`
  built cleanly and produced the expected deterministic 22/23 with this single
  red: the current workspace-mode request entry still converts in place into
  the caller-provided native buffer (or the workspace fallback) and accepts a
  NULL-buffer nonzero request, so the first pre-render rejection assertion at
  `test_audio_output.c:1805` fails (`--- 0 != 1`: `AUDIO_OUTPUT_SUBMIT_ACCEPTED`
  instead of `AUDIO_OUTPUT_SUBMIT_REJECTED`). No HAL Output Audio Unit,
  application, SDL/CLI, or public API work is included; the chunk-1 Q7
  evidence remains outstanding. S5 remains unchecked; the smallest passing
  HAL workspace-copy change is the next green change.
- S5 chunk-2 HAL workspace-copy green (2026-08-23): Implemented the smallest
  passing HAL workspace-copy change and validated the focused test green. The
  adapter request entry now detects the HAL route via the facade's
  `workspace_only` flag and, before any render, delivery, or copy, accepts a
  zero-frame request with no native buffer (no render) and rejects a nonzero
  request whose native buffer is NULL, undersized (`frame_capacity <
  requested_frame_count`), or sample-count-overflowing (`frame_capacity >
  SIZE_MAX / 2`). On an accepted nonzero HAL request it converts signed-32
  mixed values into the validated borrowed workspace (observer payload
  identity on the workspace) and then copies the converted samples into the
  separate native buffer with zero allocation; the fake route keeps its
  in-buffer conversion behavior unchanged. Focused validation
  `cmake --build build --target test_audio_output --parallel 2 && ctest --test-dir build -R '^test_audio_output$' --output-on-failure`
  passed 1/1 with all 23 binary tests green, and the full project suite passed
  10/10. No HAL Output Audio Unit, application, SDL/CLI, or public API work is
  included; the chunk-1 Q7 evidence remains outstanding. S5 remains
  unchecked.
- S5 chunk-2 final (2026-08-23): Delivered the real private CoreAudio
  lifecycle/callback with control-side quiescence. The production default
  façade in `src/audio_output/adapters/coreaudio_facade.c` is now the real
  macOS HAL Output Audio Unit route: `hal_open` finds the HAL Output Audio
  Unit, enables input scope bus 1 and output scope bus 0, and sets the default
  output device; `hal_configure` requires the device's nominal rate to equal
  the requested 44.1 or 48 kHz strict interleaved Float32 stereo stream format
  and sets/re-reads the stream format; `hal_bind_request` installs
  `hal_output_render_callback` via `SetRenderCallback`; `hal_start` opens the
  lock-free callback admission, then `AudioUnitInitialize` and
  `AudioOutputUnitStart`; `hal_stop` drains callbacks and calls
  `AudioOutputUnitStop`; `hal_quiesce` drains; `hal_dispose` requires a
  quiescent instance with admission CLOSED before uninitialize/dispose. The
  render callback admits each device request through a lock-free OPEN→IN_FLIGHT
  atomic exchange (no allocation, locking, or blocking), validates the native
  buffer (one buffer, two channels, non-NULL data, sufficient byte size; zero
  frames need no buffer), drives the bound exact-N request, and releases
  admission only after the request returns. Instance start rolls back on
  failure: `rollback_instance_start` closes/drains admission and returns the
  instance to INACTIVE. New focused evidence in `tests/audio_output/
  test_audio_output.c`: `coreaudio_open_failure_rolls_back_with_dispose`,
  `coreaudio_stateless_configure_failure_rolls_back_with_dispose`,
  `coreaudio_stateless_start_failure_rolls_back_with_dispose`,
  `coreaudio_configure_failure_rolls_back_with_dispose`,
  `coreaudio_bind_failure_rolls_back_with_dispose`, and
  `coreaudio_start_failure_rolls_back_with_dispose` (each failed lifecycle step
  rolls back with DISPOSE and no render/delivery), plus
  `coreaudio_callback_routes_exact_n_into_preallocated_float32_output`,
  `coreaudio_stop_request_completes_current_request_without_quiescing`,
  `coreaudio_teardown_returns_first_failure_and_enters_error_without_retry`,
  and `coreaudio_hal_copies_native_output_before_releasing_admission` (an
  mprotect `PROT_NONE` SIGSEGV/SIGBUS probe proves the converted native-buffer
  copy completes while callback admission is still IN_FLIGHT, with the expected
  48 kHz Float32 values). Focused validation
  `cmake --build build --target test_audio_output --parallel 2 && ctest --test-dir build -R '^test_audio_output$' --output-on-failure`
  passed 1/1 with all 31 binary tests green. No application, SDL/CLI, or public
  API work is included. S5 remains unchecked.
- S5 chunk-3 (2026-08-23): Delivered the application cutover plus SDL, `-o`,
  and CLI deletion. `src/application.c` now composes the private CoreAudio live
  route directly in `run_live_output`: the legacy exact-N renderer feeds the
  adapter instance in workspace delivery mode, started with the production
  default (real HAL) façade and stopped via
  `audio_output_coreaudio_adapter_stop_instance` after interrupt/completion;
  the SDL-era `open_snddev`/`play_it`/`TfmxTakedown` composition, the strict
  live-profile gate, and the `-o` parsing/file-output lifecycle are removed.
  The `-b`, `-8`, `-f`, `-o`, `-w`, and `-v` options are removed from the
  getopt string; each is an ordinary unknown option that prints usage and
  exits non-zero with no side effect. `src/audio.c` and `src/tfmx.c` are
  deleted, and `CMakeLists.txt` no longer discovers or links SDL (the CLI
  identity test's `FAIL_REGULAR_EXPRESSION` covers `tfmxplay|tfmx-play|/SDL`;
  APPLE-gated AudioToolbox/AudioUnit/CoreAudio frameworks are linked).
  New application-level evidence: `tests/application/
  test_application_removed_options.c` (1/1) proves each of the six removed
  options is rejected as unknown with usage and exit status 2, and `-o` never
  creates its output file; `tests/application/test_application_audio_lifecycle.c`
  (1/1) proves the default live route reaches the private route with exactly
  one open/configure/bind/start, render requests, and one control
  stop/quiesce/dispose, returning exit 0 on the `step8` fixture.
  `tests/audio_output/test_audio_routing.c`,
  `tests/audio_output/recording_sink.c/.h`, and
  `tests/application/test_application_strict_profile.c` are deleted with the
  retired routes. Focused validation
  `cmake --build build --target test_application_removed_options test_application_audio_lifecycle --parallel 2 && ctest --test-dir build -R '^(test_application_removed_options|test_application_audio_lifecycle)$' --output-on-failure`
  passed 2/2. S5 remains unchecked.
- S5 final fresh suite (2026-08-23): Validated the complete S5 outcome from a
  fresh build. `rm -rf build && cmake -S . -B build` configured cleanly, the
  full build succeeded, and `ctest --test-dir build --output-on-failure` passed
  8/8: `test_synthtracker_cli_identity` (no `tfmxplay`/`/SDL` identity),
  `player_compile_probe`, `test_playback_context`, `test_application` (2/2),
  `test_application_removed_options` (1/1),
  `test_application_audio_lifecycle` (1/1), `test_audio_output` (31/31), and
  `test_legacy_exact_renderer` (1/1 with all 8 fixture/rate cases). The suite
  count fell from the earlier 10/10 to 8/8 because `test_audio_routing` and
  `test_application_strict_profile` retired with the SDL-era and strict-profile
  routes. Chunk-1 Q7 fixture evidence (four valid fixture pairs at 44.1 and 48
  kHz, zero/variable requests, frame retention, exactly-once tick advance,
  non-silent stereo Float32 callback output) is covered by
   `test_legacy_exact_renderer`; the supplemental macOS on-device smoke check
   remains outstanding for S6 validation. S5 is complete and checked below;
   S6 (validation only) and S7 (completion only) remain unchecked.
- S6 validation (2026-08-24): `cmake -S . -B build
  -DCMAKE_PREFIX_PATH="$(brew --prefix cmocka)"`, `cmake --build build
  --parallel 2`, and `ctest --test-dir build --output-on-failure` all passed.
  CTest passed 8/8: `test_synthtracker_cli_identity`, `player_compile_probe`,
  `test_playback_context`, `test_application`,
  `test_application_removed_options`, `test_application_audio_lifecycle`,
  `test_audio_output`, and `test_legacy_exact_renderer`. The supplemental
  on-device macOS smoke ran the production `./build/SynthTracker` with the
  `mdat.step8`/`smpl.step8` fixture pair for five seconds. It reached the
  engine's `Starting`/`TFMX Init`/`Starting Song` output, accepted SIGINT for
  its normal stop path, and exited with status 0; this exercises the real
  default CoreAudio device lifecycle rather than the test façade. Terminal
  output cannot independently assess listener-perceived audibility. The
  canonical roadmap index (revision 20) and Phase 4 record (revision 13) were
  reviewed: this scoped validation has not materially changed Phase 4, Stage 3,
  its sequence, or its next work before S7 completion, so both roadmap records
  remain current and no mutation is proposed. A6 remains unchecked because
  README, ASR, Glossary, ADR-009, and ADR-010 retain stale Track 015 status or
  implementation statements; their reconciliation needs separate approval. S6
  is complete and checked. S6.1 records the approved active-rate replan; S6.2
  through S6.5 and S7 remain unchecked.
- S6 external-module compatibility failure (2026-08-24): A user-supplied XOut2
  TFMX module/sample pair printed only `*** main: Starting` and exited with
  status 1, before `*** TfmxInit` or `*** StartSong`; CoreAudio and device-rate
  configuration therefore were not reached. An isolated Debug-build LLDB trace
  stopped at `src/playback/playback_context.c:53` with
  `TFMX_LOAD_INVALID_FORMAT`. The first rejecting loader rule is
  `src/playback/tfmx_loader.c:78`: at `trackstart` `0x2c0`, the non-`voices_01`
  layout requires the word at `trackstart + 2` to be `0xfe01`, while this module
  has `0xff00`. The current loader instead accepts the fixture-specific
  `0000 fe01 fe02 fe03 fe04 fe05 fe06 fe07` trackstep sequence followed by
  `effe`. This result does not establish whether the external module is
  malformed; it establishes that the bounded fixture evidence does not cover
  this real-module layout. The approved external-module loader deferral remains
  unchanged; A5 and M4 remain unchecked pending the active-rate replan's
  S6.2-S6.5 evidence, and S7 must not begin before that work completes.
- S6.2 negotiated-rate preparation red/green (2026-08-24): The focused red
  command `cmake --build build --target test_audio_output --parallel 2` failed
  as intended before production changes: the test-only fake could not supply a
  configured format through `configure`, and
  `audio_output_coreaudio_bound_route` had no `prepare` field. The smallest
  private change adds the facade's configured-format result and the route's
  boolean control-side `prepare(context, format)` callback. Instance startup
  now orders open → configure → validate the reported strict-stereo Float32
  44.1/48 kHz format → prepare → bind → admission → start. A missing callback
  preserves the existing no-op preparation behavior; preflight invalid requested
  formats reject before any facade call; configure failure, unsupported reported
  format, or preparation failure dispose to INACTIVE before bind/start/render/
  delivery. The focused tests cover 44.1 and 48 kHz ordering, exactly-once
  preparation, no request before preparation, all three post-open rollback
  cases, and the existing bind/start rollback. Green validation
  `cmake --build build --target test_audio_output --parallel 2 && ctest
  --test-dir build -R '^test_audio_output$' --output-on-failure` passed 1/1;
  `./build/test_audio_output` passed 33/33; the full build and
  `ctest --test-dir build --output-on-failure` passed 8/8. Independent test
  review found no gap against S6.2. S6.3 is next; no application, renderer,
  loader, documentation, public API, or runtime rate-change work is included.
- S6.3 real-renderer preparation red/green (2026-08-24): This is test-only
  integration composition over the production preparation contract delivered by
  S6.2; no production or application source changed. The focused red command
  `cmake --build build --target test_legacy_exact_renderer --parallel 2 &&
  ctest --test-dir build -R '^test_legacy_exact_renderer$'
  --output-on-failure` built, then failed 0/1 because the real renderer was
  intentionally unprepared: all eight fixture/rate cases failed, first `step8`
  at 44100 Hz (`ticks=0`, expected `882`). The green test composes the real
  renderer through the adapter route's existing `prepare` callback; its fake
  facade reports 44.1 or 48 kHz and the callback initializes the renderer only
  at that negotiated rate. It covers `step8`, `loop_f1`, `envelope_tempo`, and
  `voices_01` at both rates, asserting prepare-before-request ordering,
  zero-frame no-advance, variable exact-N acceptance, partial-tick retention,
  exactly-once advance, continuity, non-silent stereo Float32 output, and no
  allocation. The focused green command passed 1/1 with all eight cases; the
  full `ctest --test-dir build --output-on-failure` passed 8/8. Independent
  review confirmed this bounded integration evidence is valid under the
  approved test-only-composition allowance. S6.4 is next; no application,
  loader, documentation, public API, resampler, or runtime rate-change work is
  included.
- S6.2a unspecified-rate correction red/green (2026-08-24): The focused red
  command `cmake --build build --target test_audio_output --parallel 2 &&
  ctest --test-dir build -R '^test_audio_output$' --output-on-failure` built,
  then failed 34/36: a wildcard request (`sample_rate_hz == 0`) with a reported
  strict 48 kHz format was rejected, while an explicit 44.1/48 mismatch was
  incorrectly accepted. The private adapter now distinguishes a strict static
  startup request (rate 0 or explicit 44.1/48) from the reported configured
  format (44.1/48 only). The production HAL facade reads the default device
  nominal rate, accepts only 44.1/48, preserves equality checking for an
  explicit request, sets/re-reads that actual stream rate, and reports it; the
  fake mirrors wildcard and explicit matching behavior. Focused green validation
  `cmake --build build --target test_audio_output --parallel 2 && ctest
  --test-dir build -R '^test_audio_output$' --output-on-failure &&
  ./build/test_audio_output` passed 1/1 and 36/36. The three new focused cases
  prove wildcard 0 with reported 48 prepares/binds/starts, explicit 44.1/48
  mismatch disposes after open/configure, and invalid nonzero requests reject
  before lifecycle; the negotiated-format rollback table also proves a wildcard
  request with otherwise strict reported 32 kHz disposes after open/configure
  with no prepare/bind/start/render/delivery. Independent test review found no
  remaining S6.2a gap. Full CTest is currently 7/8 because the deferred S6.4
  application path still supplies explicit 44.1 against the fake's reported 48
  and correctly receives `START_REJECTED`; S6.4 must change that request to 0.
  S6.4 is next; no loader, public API, resampler, or runtime rate-change work is
  included.
- S6.4 corrected 48 kHz application composition red/green (2026-08-24): The
  focused red/green command `cmake --build build --target
  test_application_audio_lifecycle --parallel 2 && ctest --test-dir build -R
  '^test_application_audio_lifecycle$' --output-on-failure` first failed 0/1:
  `application_run` returned 1 because its explicit 44.1 kHz request correctly
  mismatched the fake's reported 48 kHz device. The smallest application change
  makes its initial strict stereo Float32 request rate 0 and retains renderer
  initialization exclusively in the private preparation callback from the
  reported configured format. The green run passed 1/1. The workspace-only fake
  asserts the requested rate is 0, reported/prepared renderer rate is 48 kHz,
  no request precedes preparation, and the exact lifecycle is
  open→configure→prepare→bind→start→stop→quiesce→dispose, once each; 64
  post-start requests render the `step8` fixture and the application returns 0
  inactive, quiescent, and disposed. Full `ctest --test-dir build
  --output-on-failure` passed 8/8. Independent test review confirmed no new
  adapter, facade, loader, public API, resampler, or runtime rate-change work.
  S6.4a records the approved legacy amplitude-normalization replan; S6.4b is
  next.
- S6.4b legacy amplitude normalization red/green (2026-08-24): The focused red
  command `cmake --build build --target test_legacy_exact_renderer --parallel
  2 && ctest --test-dir build -R '^test_legacy_exact_renderer$'
  --output-on-failure` built, then failed 0/1: both `voices_01` rate cases
  lacked the new exact audible-scale oracle, first at 44100 Hz (`ticks=3`,
  expected tick frames 882). The private
  `playback_legacy_mixer_render_frames` now reconstructs each blended lane from
  its unsigned low 16 bits (`low < 0x8000 ? low : low - 0x10000`) and expands
  that signed-16 PCM-domain result by 65536 before it becomes an Audio Frame
  Block. It applies no adapter, renderer, or application scaling; the generic
  adapter remains signed-32/2^31. A private test-only probe covers seven
  negative, zero, positive, endpoint, and wrapping vectors, including
  32768→-32768, without exposing a public API. The real renderer/adapter test
  now locates the first `voices_01` historical PCM frame and asserts exact
  signed-32 expansion and bit-exact Float32 L=1680/32768 and R=1392/32768 at
  both 44.1 and 48 kHz, while retaining the existing 8 fixture/rate exact-N,
  timing, continuity, stereo, and no-allocation checks. Focused green validation
  passed CTest 1/1 and `./build/test_legacy_exact_renderer` 2/2; the full build
  and CTest passed 8/8. Independent review found no S6.4b gap. `voices_01` is
  now the designated supplemental audible-smoke fixture; S6.5 validation and
  documentation is next.
- S6.5 supplemental real-device audible smoke (2026-08-24): Ran the production
  `./build/SynthTracker` with the self-authored `mdat.voices_01`/`smpl.voices_01`
  pair on the default 48 kHz output device. The process initialized TFMX, began
  the active two-voice Track 0000, reached Track 0001, and completed normally.
  The user listener-confirmed audible playback. This is supplemental real-device
  evidence only; the bounded automated 44.1/48 kHz fixture evidence remains the
  primary compatibility evidence. S6.5 remains unchecked pending final
  validation, documentation reconciliation, acceptance recheck, and roadmap
  reconciliation.
- S6.5 final validation, documentation, and roadmap reconciliation (2026-08-24):
  The post-S6.4b full build and `ctest --test-dir build --output-on-failure`
  passed 8/8; focused evidence remains `test_audio_output` 36/36 and
  `test_legacy_exact_renderer` 2/2 with all eight fixture/rate cases. The
  successful listener-confirmed real 48 kHz `voices_01` smoke is recorded above
  as supplemental device evidence. README, Architecture, ASR-010, ADR-009,
  ADR-010, Glossary, Audio Rendering Design, and Artifacts were reconciled and
  independently reviewed for current route, rate-selection, normalization,
  zero-frame, dispatch-refactor, loader-deferral, and historical-ADR accuracy;
  ADR index, Vision, and Testing required no change. The Phase 4 roadmap was
  revised to revision 14 and the canonical roadmap index last to revision 21:
  Track 015's delivered baseline is recorded while Stage 3 remains in progress
  pending acceptance/merge. A1-A6 and M1-M4 are checked. S6.5 is complete and
  checked; S7 was then ready for the completion transition.

Completion notes
- Completed on 2026-08-24. Track 015 delivered the private real-time CoreAudio
  HAL Output Audio Unit route, private 44.1/48 kHz startup selection and
  renderer preparation, legacy producer amplitude normalization, SDL live-audio
  and `-o` retirement, and bounded fixture evidence. Final validation passed
  CTest 8/8; `voices_01` supplied supplemental listener-confirmed real 48 kHz
  playback. General real-module loader compatibility remains deferred after the
  recorded XOut2 rejection; no public Audio Output Port, resampler, runtime
  device-rate-change policy, or target Mixer was added. Documentation and the
  Phase 4 roadmap (revision 14) and canonical index (revision 21) are
  reconciled. Stage 3 remains in progress pending its wider acceptance/merge.
