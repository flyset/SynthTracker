# TRACK TRACK_013 [ACTIVE]: legacy_pcm_audio_output_routing

Track
- ID: TRACK_013
- Repository: SynthTracker
- Branch: stage/04-03-audio-output-extraction
- Current path: .backlog/ACTIVE/2026/TRACK_013_ACTIVE_legacy_pcm_audio_output_routing.md
- Status: ACTIVE

Problems (PORE)
- P1: As a SynthTracker maintainer, I cannot exercise the approved Audio
  Frame Block boundary from the legacy playback path, because `src/audio.c`
  currently sends its final PCM through the SDL-era ring buffer or the file
  output path rather than through Track 012's Audio Output contract.
- P2: As a Phase 4 contributor, I need a deterministic live-routing boundary
  check without opening a device, but the current live SDL route is still the
  active route and offers no proof that final legacy PCM can be submitted to
  Audio Output.
- P3: As a compatibility maintainer, I must prevent unsupported live formats
  from being silently converted or partially routed while this temporary
  routing change is introduced; 8-bit, mono, and non-44.1 kHz live settings
  must fail before playback.
- P4: As a maintainer, I need finite compatible live playback to complete
  without depending on SDL device delivery, because `src/audio.c` couples final
  PCM conversion to ring-buffer throttling and final-drain behavior.

Objective
- Route final PCM produced by the existing legacy `src/audio.c` mixer into
  valid fixed-format Audio Frame Blocks and synchronously submit those blocks
  to Track 012's null adapter, intentionally producing silence; preserve the
  legacy renderer and do not extract or claim a target `Mixer` component.

Non-negotiables
- This Track is planning only while DRAFT. No implementation—including tests
  intended to drive a code change—begins until the Track is moved to ACTIVE and
  its S1 Move-to-ACTIVE plan step is checked.
- All implementation follows TDD: a focused failing test, the smallest
  passing implementation, refactoring, and relevant validation.
- All new SynthTracker-owned production and test source is C23 or later ISO C;
  C++ is not a project direction. New project-owned headers remain private and
  co-located; none may be added under retired `include/`.
- The existing legacy renderer in `src/audio.c` remains the owner of mixing,
  stereo blending, filtering, clipping, and final PCM production. This Track
  adds routing/package use of that result; it does not create a new Mixer
  extraction or duplicate renderer behavior.
- Only 44.1 kHz, stereo, interleaved, signed 16-bit little-endian PCM is
  routable as an Audio Frame Block, with exactly four payload bytes per frame.
  Incompatible live modes—including 8-bit, mono, and non-44.1 kHz settings—
  must fail deterministically before playback begins.
- For a compatible live mode, the current live SDL output route is bypassed and
  final PCM is synchronously submitted to Track 012's null adapter through a
  defined private route. That route must bypass SDL device open, pause, callback
  delivery, ring-buffer delivery, final drain, and SDL teardown while safely
  releasing legacy resources. The null adapter discards accepted blocks, so
  this Track intentionally produces silence. The exact lifecycle boundary is
  an S2 decision; this is not a claim that the route is already implemented or
  a finished device-output implementation.
- Compatible finite live playback must not ring-buffer-throttle or stall in a
  final drain, must not open an audio device, and must terminate/return
  deterministically after safely releasing legacy resources.
- The coupling in `src/audio.c` among final PCM conversion, ring-buffer
  advancement, producer throttling, and final drain must be characterized
  before change and split only as minimally necessary. Preserve the renderer's
  behavior and legacy `-o` file output.
- Legacy `-o` file output remains unchanged and is out of scope. It is not
  routed through the null adapter by this Track.
- All Audio Output buffering, ownership, retention, queueing, timing,
  ordering, backpressure, and buffer count remain deferred. Synchronous
  submission is only the call behavior required by Track 012's null adapter;
  it does not settle a future consumer contract.
- The Track creates no public C API, ABI, library-header model, persistent
  project format, device adapter, shell interface, network path, or
  unrestricted filesystem interface.
- Phase 4 compatibility is a temporary development scaffold, not a
  SynthTracker v1 compatibility promise. Compatibility impact and evidence
  must cover TFMX modules, trackstep/pattern/macro semantics, timing,
  interpreter behavior, legacy rendering, file output, and the intentional
  live-output change.

Acceptance criteria
- [ ] A1) [P1] A focused routing test proves that final PCM from the existing
  legacy renderer is packaged as an Audio Frame Block with 44.1 kHz stereo
  signed-16 little-endian interleaved representation, exact `frame_count × 4`
  payload sizing, and deterministic channel/sample bytes.
- [ ] A2) [P1, P2] A focused integration test proves that a compatible live
  configuration synchronously submits one or more valid blocks to Track 012's
  null adapter, with no SDL device open, pause, callback/ring-buffer delivery,
  final drain, or SDL teardown; the adapter's accepted-block/byte observability
  is the evidence of submission and no audible output.
- [ ] A3) [P1, P3] Focused tests prove zero-frame handling and more than one
  nonzero block size, including ASR-009 exact-length, missing-payload, and
  checked-overflow failures without partial submission.
- [ ] A4) [P3] Focused live-routing tests prove 8-bit, mono, and every
  non-44.1 kHz setting is rejected deterministically before playback or Audio
  Output submission, with a distinct failure from valid submission.
- [ ] A5) [P2, P3] A focused test proves the current SDL live route is bypassed
  only for compatible live playback and that legacy `-o` file output remains
  unchanged and does not use the null adapter.
- [ ] A6) [P1, P2] Tests and review prove synchronous call-boundary behavior
  without retained caller input, device, route, queue, or adapter references;
  buffering, ownership, retention, queueing, timing, ordering, backpressure,
  and buffer count remain unresolved.
- [ ] A7) [P2, P4] A focused finite-playback liveness test proves that a
  compatible live run neither ring-buffer-throttles nor stalls in final drain,
  does not open an audio device, and returns/terminates deterministically while
  releasing legacy resources.
- [ ] A8) [P1, P2, P3, P4] The routing change is private, independently testable,
  and limited to the legacy live path; no target Mixer extraction is claimed.
  C23 configure/build and the full CTest suite pass with compatibility and
  build-composition evidence, including intentional silence, unsupported-mode
  failures, and finite-playback liveness.
- [ ] A9) [P1, P2, P3, P4] Before final validation/completion, applicable
  README, Architecture, ASR, ADR, and Glossary records are updated for this
  implementation change, or the Track records an explicit reason each
  individual record remains unchanged, following `docs/AGENT_WORKFLOW.md`.

Why now / impact
- Track 012 established the private fixed-format Audio Frame Block validation
  and synchronous null adapter. The next Phase 4 step is to exercise that
  boundary from the real legacy final-PCM path rather than add an unwired
  producer or a new Mixer component. Compatible live playback will therefore
  stop using the SDL output route and intentionally become silent while the
  null adapter validates routing; this is temporary output compatibility work
  needed before a real adapter and future live-output policy.

Scope
- In scope:
  - A private bridge at the existing legacy `src/audio.c` final-PCM boundary.
  - Packaging the renderer's completed PCM into Track 012's private Audio
    Frame Block representation.
  - Synchronous submission of compatible live blocks to Track 012's null
    adapter, with accepted-block/byte evidence and intentional silence.
  - Deterministic pre-playback validation for exactly 44.1 kHz, stereo,
    signed 16-bit little-endian live PCM only.
  - Bypassing the current SDL live output route for compatible live playback
    as an explicit temporary Phase 4 routing/output compatibility change.
  - Focused component/application tests, bounded CMake target changes, TDD,
    compatibility checks, and Track evidence updates once ACTIVE.
  - S2 resolution and recording of private build composition: how the
    `SynthTracker` executable and relevant application/integration test targets
    privately compile/link the existing Audio Output implementation required by
    the bridge.
- Out of scope:
  - Extracting, implementing, or claiming a target `Mixer`; changing the
    legacy renderer's mixing, blending, filtering, clipping, or PCM behavior.
  - Any SDL, CoreAudio, or other device adapter, audible output, or replacement
    live-output implementation.
  - Audio Output buffering, ownership, retention, queueing, timing, ordering,
    backpressure, buffer count, or future scheduling policy.
  - Routing legacy `-o` file output, changing its format, or changing its
    existing file-writing path.
  - Format negotiation or conversion beyond rejecting incompatible live modes.
  - Loader, Player, Tracker, Synthesizer, Model, GUI, editor, or DAW format
    work; public API/ABI; durable memory; or Git history.
  - Establishing the CoreAudio adapter or retiring the SDL route as a real
    output route; those remain later Track 014/015 work.

Milestones
- [ ] M1) Track is approved and moved to ACTIVE with S1 checked.
- [ ] M2) The private final-PCM routing call boundary, mode validation,
  block representation, null-adapter submission behavior, pre-playback failure
  semantics, exact SDL/resource lifecycle boundary, and build composition are
  resolved in the Decision log.
- [ ] M3) Compatible live routing submits valid blocks synchronously to the
  null adapter, bypasses SDL live delivery, and intentionally produces silence;
  `-o` output remains unchanged.
- [ ] M4) Unsupported live modes fail before playback, focused tests pass, and
  full C23/CTest and compatibility evidence is recorded.
- [ ] M5) Finite compatible live playback has deterministic liveness and
  termination evidence, and the documentation gate is satisfied before final
  validation/completion.

Risks / decisions
- Risk (planning gate): Implementation cannot begin while this Track is DRAFT;
  S1 is the immediate next step and no later plan step is pre-checked.
- Risk (intent confusion): Calling this a Mixer producer could imply target
  Mixer extraction or independent rendering. Decision: name and treat this as
  legacy final-PCM-to-Audio-Output routing only; `src/audio.c` remains the
  renderer.
- Risk (intentional compatibility change): Bypassing SDL live output makes
  compatible live playback silent until a later real adapter is available.
  Decision: this silence is intentional and limited to compatible live modes;
  it must be documented and tested as a temporary Phase 4 routing/output
  compatibility change.
- Risk (unsupported formats): 8-bit, mono, and non-44.1 kHz settings could be
  partially rendered or submitted. Decision: reject them deterministically
  before playback and before any Audio Output submission.
- Decision: Only 44.1 kHz, stereo, signed 16-bit little-endian PCM is routable;
  no conversion or negotiation is added.
- Decision: Legacy `-o` file output is unchanged and out of scope.
- Decision: Track 012's null adapter is called synchronously and discards valid
  blocks; its behavior intentionally yields silence. No deferred Audio Output
  buffering or lifetime policy is inferred from this call.
- Decision (lifecycle, pending S2): Compatible live playback must use a defined
  private route that bypasses SDL device open, pause, callback/ring-buffer
  delivery, final drain, and SDL teardown, while safely releasing legacy
  resources. S2 must record the exact boundary and cleanup ordering.
- Risk (coupled legacy loop): Splitting conversion, ring-buffer advancement,
  producer throttling, and final drain in `src/audio.c` could alter renderer or
  file-output behavior. Decision: characterize the coupling first and make the
  smallest split necessary; preserve legacy `-o` output.
- Decision (build composition, pending S2): S2 must record how `SynthTracker`
  and each relevant application/integration test target privately compile/link
  the existing Audio Output implementation required by the bridge. No public
  library/header boundary is implied.
- Dependency (sequencing): Track 014 independently tests the CoreAudio adapter.
  Track 015 establishes the live real-time CoreAudio route and SDL route
  retirement. Only after those dependencies may a later Mixer-extraction Track
  be planned; this Track does not change that roadmap direction.
- Version impact (C API/ABI): Unchanged. The routing seam and representations
  remain private; no public declaration or ABI is added.
- Version impact (module compatibility/extension): Unchanged. No TFMX bytes,
  loader behavior, format detection, or module layout changes.
- Version impact (interpreter/timing/audio behavior): Intentionally changed only
  for compatible live output: the legacy renderer remains unchanged, while
  SDL live delivery is bypassed and null-adapter submission produces silence.
  Unsupported live modes fail before playback. Trackstep, pattern, macro,
  interpreter, timing, and `-o` file-output behavior remain unchanged.
- Version impact (persistent DAW format/versioning): Unchanged. No serialized
  data or version field is introduced.
- Version impact (platform/audio-output adapter): Intentionally changed for
  the temporary live route: compatible live playback no longer reaches the
  current SDL route and instead reaches Track 012's device-free null adapter.
  No real platform adapter is implemented.
- Version impact (component/package boundaries): Changed privately. The legacy
  audio path gains a private routing seam to the existing private Audio Output
  component; no target Mixer or public package boundary is created.

Open questions
- [ ] Q1) Which private call boundary in the existing `src/audio.c` loop best
  exposes final PCM without changing renderer ownership or `-o` output?
- [ ] Q2) What private routing result identifies unsupported live mode,
  packaging/validation failure, and successful null-adapter submission while
  preserving Track 012's existing result semantics?
- [ ] Q3) Which bounded fixture and existing playback checks prove renderer
  output, channel order, signed-16 little-endian bytes, frame sizing, and no
  SDL delivery without claiming a new Mixer?
- [ ] Q4) What is the minimum deterministic pre-playback validation point for
  rate, channel count, and sample representation across live configuration
  paths, while leaving `-o` unchanged?
- [ ] Q5) What existing TFMX/application compatibility evidence demonstrates
  unchanged sequencing, timing, interpreter, renderer, and file output plus the
  intentional compatible-live silence?

Decision log
- Decision (gate): No implementation, including new tests, occurs while this
  Track is DRAFT; S1 is the immediate next step and no implementation plan step
  is pre-checked.
- Decision (design basis): Track 012's private Audio Frame Block/null-adapter
  contract, ADR-007, ASR-009, `docs/ARTIFACTS.md`, and the Phase 4 compatibility
  policy govern this routing Track.
- Decision (routing): The existing legacy `src/audio.c` renderer supplies the
  final PCM. A private routing bridge packages and synchronously submits it to
  Track 012's null adapter for compatible live playback; the SDL live route is
  bypassed and silence is intentional.
- Decision (format gate): Only 44.1 kHz, stereo, signed 16-bit little-endian
  PCM is routable. 8-bit, mono, and non-44.1 kHz live modes fail before playback
  and before submission.
- Decision (file output): Legacy `-o` output remains on its existing path,
  unchanged and out of scope.
- Decision (deferred contract): Buffering, ownership, retention, queueing,
  timing, ordering, backpressure, and buffer count are not decided or
  implemented here.
- Decision (scope): This Track does not extract or claim a target Mixer. Tracks
  014–015 remain later planned real-adapter/live-output work and are not
  authorized by this Track. Track 014 independently tests CoreAudio; Track 015
  establishes the live real-time CoreAudio route and retires SDL; only then may
  a later Mixer-extraction Track be planned.
- Decision (documentation gate): Before final validation/completion of the
  implementation, update applicable README, Architecture, ASR, ADR, and
  Glossary material, or record an explicit unchanged rationale for each
  individual record. This gate is deferred until implementation and is not
  performed by this DRAFT revision.

Plan (execution steps)
- [x] S1) Move Track TRACK_013 to ACTIVE (folder, filename, and title status)
  after planning approval; this gates every step below.
- [ ] S2) Re-read this Track, state the next unchecked step, resolve Q1–Q5, and
  record the exact private routing boundary, mode-validation point, result
  behavior, fixture scope, SDL bypass/resource-cleanup lifecycle, and build
  composition before writing code. Characterize the `src/audio.c` coupling
  among conversion, ring-buffer advancement, producer throttling, and final
  drain, and choose the minimum required split. Keep all deferred Audio Output
  policies unresolved.
- [ ] S3) TDD chunk — compatible-mode gate and final-PCM packaging: add failing
  focused coverage, implement the smallest private bridge, validate exact
  fixed-format blocks, and update this Track.
- [ ] S4) TDD chunk — synchronous null-adapter routing: add failing live-path
  coverage, bypass SDL device open/pause/callback/ring-buffer delivery/final
  drain/SDL teardown for compatible live playback, prove intentional silence,
  adapter submission, safe legacy cleanup, and deterministic finite-playback
  liveness/return, validate, and update this Track.
- [ ] S5) TDD chunk — unsupported live modes: add failing coverage for 8-bit,
  mono, and non-44.1 kHz settings, reject before playback/submission, validate,
  and update this Track.
- [ ] S6) Confirm legacy renderer and `-o` file output are unchanged; record
  compatibility evidence and confirm no target Mixer extraction is claimed.
- [ ] S7) Run focused CTest and the full compliant validation set (`cmake -S .
  -B build -DCMAKE_PREFIX_PATH="$(brew --prefix cmocka)"`, `cmake --build build
  --parallel 2`, `ctest --test-dir build --output-on-failure`); record intentional
  live silence, unsupported-mode failures, TFMX/playback, and file-output
  evidence.
- [ ] S8) Apply the documentation gate for this implementation change: update
  applicable README, Architecture, ASR, ADR, and Glossary material, or record
  an explicit per-record unchanged rationale, before final validation.
- [ ] S9) Review changed files, all acceptance criteria, all compatibility
  dimensions, and roadmap alignment; do not activate or complete this DRAFT
  until the required approvals and evidence exist.

Current inventory
- `src/audio.c`: existing legacy mixer and final PCM conversion owner. It
  supports SDL live output and legacy file output, with configurable rate,
  channel count, and 8/16-bit paths; no Track 013 routing bridge exists.
- `src/audio_output/audio_output.h` and `.c`: Track 012's private fixed-format
  Audio Frame Block validation and synchronous null adapter. Its block carries
  `frame_count`, payload, and payload length; valid blocks are accepted and
  discarded without device output or payload retention.
- `tests/audio_output/test_audio_output.c`: Track 012 focused evidence for
  zero-frame validity, exact sizing, distinct failures, no retention, and
  checked overflow.
- `CMakeLists.txt`: C23 build with the existing standalone Audio Output test;
  the main `SynthTracker` target and current application tests still use the
  legacy audio path. S2 must explicitly choose and record the private compile/
  link composition needed for the bridge in `SynthTracker` and relevant
  application/integration test targets.
- Existing compatibility evidence: `tests/playback/`,
  `test_playback_context`, `tfmx_compile_probe`, `player_compile_probe`, and
  application/CLI tests. Additional live-routing and file-output evidence is
  required by this Track, including no-device finite-playback liveness and
  deterministic termination.
- Sequencing dependency: Track 014 independently tests CoreAudio; Track 015
  establishes the live real-time CoreAudio route and SDL route retirement. A
  later Mixer-extraction Track is not planned until both are complete.

Artifacts
- Living roadmap: `SynthTracker modernization roadmap`, Phase 4 — Component
  extraction; Stage 3 — Audio Output extraction. Track 012 is delivered and
  this Track is the planned legacy final-PCM routing step. Later adapter/live
  output work remains planned.
- [`docs/adr/ADR-007-audio-frame-block-boundary-and-fixed-first-format.md`](../../../docs/adr/ADR-007-audio-frame-block-boundary-and-fixed-first-format.md)
  — fixed Audio Frame Block format and boundary decision.
- [`docs/ASR.md#asr-009--audio-frame-block-boundary-invariants`](../../../docs/ASR.md)
  — Audio Frame Block validity invariants.
- [`docs/ARTIFACTS.md`](../../../docs/ARTIFACTS.md) — artifact vocabulary and
  deferred ownership/timing/output questions.
- [`docs/ARCHITECTURE.md`](../../../docs/ARCHITECTURE.md) — current `audio.c`
  ownership, target-only Mixer/Audio Output flow, and Phase 4 policy.
- [`docs/TESTING.md`](../../../docs/TESTING.md) and
  [`tests/AGENTS.md`](../../../tests/AGENTS.md) — evidence, TDD, and test
  ownership rules.
- [`docs/AGENT_WORKFLOW.md`](../../../docs/AGENT_WORKFLOW.md) — approval,
  compatibility, TDD, and validation gates.
- `README.md`, `docs/ARCHITECTURE.md`, `docs/ASR.md`, applicable ADRs, and
  `docs/GLOSSARY.md` — documentation-gate records to update or explicitly
  justify as unchanged before implementation completion; they are not changed
  by this DRAFT-only planning revision.
- [`.backlog/README.md`](../../README.md) and [`.backlog/PORE.md`](../../PORE.md)
  — Track lifecycle, PORE, and implementation gates.
- [Track 012](../../COMPLETED/2026/TRACK_012_COMPLETED_audio_output_null_adapter.md)
  — completed private Audio Output null adapter and synchronous submission
  foundation.

Completion notes
- Not applicable while DRAFT. No implementation, activation, test execution,
  roadmap mutation, durable-memory mutation, or Git-history mutation is
  authorized by this Track draft. The required documentation gate is deferred
  until the implementation change and must occur before final validation and
  completion; this revision changes only this Track file.
