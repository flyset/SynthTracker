# TRACK TRACK_014 [COMPLETED]: coreaudio_adapter

Track
- ID: TRACK_014
- Repository: SynthTracker
- Branch: stage/04-03-audio-output-extraction
- Current path: .backlog/COMPLETED/2026/TRACK_014_COMPLETED_coreaudio_adapter.md
- Status: COMPLETED

Problems (PORE)
- P1: As a Stage 3 maintainer, I cannot exercise a CoreAudio adapter against
  the accepted Audio Frame Block boundary, because `audio_output` currently
  submits blocks only to the private null adapter
  (`audio_output_null_adapter_submit`, `src/audio.c` bridge) and no private
  CoreAudio adapter dispatch target exists.
- P2: As a future Audio Output implementer, I experience a risk that
  destination format adaptation escapes the adapter, because ADR-008 assigns
  destination conversion to Audio Output and its platform adapters, and a
  shared PCM converter component would couple adapters to the block producer
  and to each other.
- P3: As a maintainer, I cannot validate the CoreAudio adapter's conversion
  deterministically if it depends on a device, because device open, render
  callbacks, buffering, and device clock behavior are nondeterministic,
  real-time concerns owned by Track 015, and the Track 014 conversion contract
  must be proven without any device.

Objective
- Add a private, independently testable CoreAudio adapter that receives Audio
  Frame Blocks (raw signed-32 `{ left, right }`) through the private
  `audio_output` dispatch boundary and privately converts each block to its
  CoreAudio PCM/client representation with no shared PCM converter component,
  proven without opening or driving any device, while leaving all device
  lifecycle, rendering, buffering, and device-route changes to Track 015.

Non-negotiables
- This Track is ACTIVE. Implementation begins only after its Move-to-ACTIVE
  plan step is checked and the user approves the scope. Plan steps proceed one
  declared step or coherent TDD chunk at a time.
- ADR-008 and ASR-009 are normative. The Audio Frame Block is an ordered
  sequence of zero or more interleaved signed-32 `{ left, right }` frames; it
  is never serialized PCM and never device-native data. Track 014 leaves the
  block representation unchanged.
- `audio_output` dispatches blocks to the CoreAudio adapter on macOS (via the
  APPLE-gated `audio_output_dispatch_submit` boundary, the approved Q2
  mechanism, with the null adapter as the non-macOS fallback); the adapter
  privately converts blocks to its CoreAudio PCM/client representation (S4).
  No shared PCM converter component exists: conversion lives inside the
  adapter, and `audio_output` carries no PCM or device representation.
- Track 014 will perform no device open/close, no render-callback
  registration or invocation, no buffering, queueing, or backpressure, no
  device clock or scheduling, and will produce no audible output. All of
  those remain Track 015.
- Track 014 retires nothing: the null adapter remains as the non-macOS
  fallback, not the macOS production destination, the legacy `-o` setup,
  conversion, ring, and file-output behavior remains unchanged, and the macOS
  bridge destination changed to the CoreAudio adapter per the approved Q2
  mechanism (S3). Track 015 owns live real-time CoreAudio routing and
  any specified SDL-era audio-output retirement only as defined by the
  roadmap; this Track asserts no current SDL live route.
- No assumption about stereo blend or low-pass relocation is made: legacy
  stereo blend and low-pass filtering remain in the legacy renderer/`-o` path
  and are intentionally absent from the temporary live route, exactly as
  recorded by ADR-008 and ARTIFACTS. The adapter and dispatch touch neither.
- Track 013 deferred future Audio Output buffering, ownership, retention,
  queueing, timing, ordering, backpressure, and buffer-count policy; Track 014
  continues that deferral.
- This Track claims no target `Mixer` extraction and will not implement the
  target device-independent Audio Output Port (ADR-005, ASR-006); the private
  CoreAudio adapter is planned as a bounded Stage 3 step toward it.
- All new SynthTracker-owned production and test source is C23 or a later ISO
  C standard; C++ is not a project direction. New project-owned headers remain
  private and co-located; none may be added under retired `include/`.
- All behavior changes follow TDD: a focused failing automated test, the
  smallest passing change, refactoring, then validation and Track update.
- Direct private source composition only: no new library, public API, public
  header, export, or public target. Phase 4 compatibility is a temporary
  development scaffold, not a SynthTracker v1 compatibility promise.

Acceptance criteria
- [x] A1) [P1] Behavioral component evidence: component tests prove the
  private dispatch boundary delivers a block to the CoreAudio adapter with the
  exact frame count and frame order, including an accepted zero-frame block,
  with no device involvement.
- [x] A2) [P1, P2] Behavioral component evidence: component tests prove the
  adapter privately converts zero- and nonzero-frame blocks to its CoreAudio
  PCM/client representation under the resolved adapter-private PCM mapping,
  asserting the exact mapped output for every explicit range/rounding/clipping
  rule of that mapping without requiring source signed-32 values to be
  preserved exactly through conversion, and that conversion is deterministic
  and fully observable with no device involvement. A2 was re-checked when the
  reopened S4 TDD gate closed: the safe pre-green evidence for both overflow
  guards is recorded in the Decision log and the S4 step evidence.
- [x] A3) [P2, P3] Structural/composition evidence: explicit source/CMake
  composition review proves no shared PCM converter component exists —
  conversion code resides in the adapter and `audio_output` carries no PCM or
  device representation — and proves the absence of device lifecycle and audio
  behavior: no device open/close, no render-callback registration or
  invocation, no buffering, no device clock, and no audible output. These
  structural absences require that review, not component or composition
  tests; component tests are limited to observable dispatch and deterministic
  device-free conversion.
- [x] A4) [P1, P2, P3] The complete six-dimension version-impact decision is
  recorded in the Decision log before implementation, with an explicit reason
  for every dimension judged unchanged, and the recorded reasons do not assume
  stereo blend or low-pass relocation.
- [x] A5) [P1, P2, P3] C23 configure/build and the full CTest suite pass;
  TDD evidence, the exact changed-file inventory, and the compatibility impact
  are recorded; evidence proves direct private source composition with no new
  library, public API, public header, or public target, and that the temporary
  Track 013 null-adapter live route remains the non-macOS fallback (the audio
  bridge is planned to select the CoreAudio adapter on macOS per the approved
  Q2 mechanism) and legacy `-o` behavior remains unchanged. Its TDD-evidence
  clause is satisfied: the reopened S4 TDD gate closed with the recorded safe
  pre-green evidence for both overflow guards. A5 is checked: its remaining
  clauses (exact changed-file inventory, compatibility-impact recording, and
  the S6 documentation/validation gate outcomes) are satisfied and recorded
  in the S6 step evidence.

Why now / impact
- The living roadmap (Phase 4, Stage 3 Audio Output extraction) names Track 014
  as the next roadmap work: drafting it before implementation for an
  independently testable CoreAudio adapter. Track 013 delivered the private
  mixed-value routing evidence; the adapter step is planned to prove
  destination format adaptation against the same accepted boundary without any
  device dependency.
- The CoreAudio adapter work is planned to format the generic mixed-value
  boundary for CoreAudio; it implies no target Mixer internal processing.
  Keeping device lifecycle in Track 015 bounds this Track to deterministic,
  device-free, testable adapter behavior.
- Track 014 is planned to change no audible or legacy behavior: the adapter
  is a private, additive Stage 3 component step intended to preserve
  legacy behavior unchanged, keep the Track 013 null adapter as the non-macOS
  fallback, and keep the `-o` baseline; the macOS bridge destination changed
  to the CoreAudio adapter per the approved Q2 mechanism (S3).

Scope
- In scope:
  - The private CoreAudio adapter component, created at S3 at
    `src/audio_output/adapters/coreaudio_adapter.c` with its private
    co-located header `src/audio_output/adapters/coreaudio_adapter.h`, which
    receives Audio Frame Blocks and privately converts them to its CoreAudio
    PCM/client representation (S4).
  - A private `audio_output` dispatch boundary
    (`audio_output_dispatch_submit`) through which the audio bridge
    selects/routes to the CoreAudio adapter on macOS (APPLE-gated CMake
    composition with `SYNTHTRACKER_AUDIO_OUTPUT_USE_DISPATCH`) and selects
    the null adapter on non-macOS, with the null adapter remaining as the
    non-macOS fallback, not the macOS production destination; the production
    null adapter remains available and legacy behavior and the Track 013
    bridge behavior remain otherwise unchanged; the only live-route change is
    the macOS bridge destination selecting the CoreAudio adapter per the
    approved Q2 mechanism.
  - Private, device-free conversion of zero- and nonzero-frame signed-32
    blocks, implemented inside the adapter only (S4); no shared PCM converter
    component exists.
  - Focused behavioral component tests prove dispatch, deterministic
    conversion under the resolved adapter-private PCM mapping, and zero-frame
    acceptance with no device involvement; structural/composition evidence of
    no shared PCM converter and of the absence of device lifecycle,
    render-callback, buffering, and audible-route behavior remains to be
    recorded through explicit source/CMake composition review (S5), not
    component or composition tests.
  - TDD evidence, the six-dimension impact decision, the exact changed-file
    inventory, CMake changes required by the resolved private composition, and
    Track updates.
- Out of scope (Track 015):
  - Device open/close, AudioUnit/device lifecycle, render-callback
    registration and invocation, buffering, queueing, backpressure, device
    clock, scheduling, and any audible output.
  - Any SDL-era audio-output retirement, which Track 015 owns only as defined
    by the roadmap; any retirement of the temporary Track 013 null adapter or
    any change to its non-macOS live route (the macOS bridge selection of the
    CoreAudio adapter is the approved Q2 dispatch mechanism, not a null-route
    change), or any other change to temporary live playback behavior.
  - Any legacy `-o` setup, conversion, ring, file format, or writer change;
    stereo blend and low-pass relocation of any kind.
  - A shared PCM converter component; a public C API/ABI or library; the
    target device-independent Audio Output Port; a target `Mixer` extraction;
    file I/O, DAW persistence, shell, network, or unrestricted filesystem
    work.

Milestones
- [x] M1) Track is ACTIVE with synchronized folder, filename, title, status,
  and S1 checked, under explicit user approval.
- [x] M2) S2 resolves the remaining open questions (dispatch mechanics,
  conversion representation, composition; Q3 adapter placement is resolved as
  `src/audio_output/adapters/coreaudio_adapter.c` with its private co-located
  header) and records the complete six-dimension impact decision before
  implementation.
- [x] M3) TDD implementation proves behavioral evidence of private dispatch and
  deterministic adapter-private conversion of zero- and nonzero-frame blocks
  under the resolved adapter-private PCM mapping; structural/composition
  evidence of no shared PCM converter and no device lifecycle or audible
  behavior is recorded through explicit source/CMake composition review, not
  component or composition tests.
- [x] M4) C23/full CTest validation, direct private composition review, exact
  changed-file inventory, and the documentation gate are complete before
  completion.

Risks / decisions
- Risk: Conversion or dispatch semantics could leak into `audio_output` and
  couple the block producer to the adapter. Decision: `audio_output` performs
  dispatch only; all PCM/client conversion is private to the CoreAudio adapter.
- Risk: The adapter's private PCM/client representation could silently settle
  Track 015's device contract. Decision: the conversion representation is
  chosen for Track 014 testability only; device format negotiation, ASBD,
  lifecycle, and scheduling remain Track 015.
- Risk: A shared PCM converter could appear as a "reusable" helper. Decision:
  no shared PCM converter component exists; duplication or in-adapter
  conversion is preferred over a converter boundary.
- Risk: Work could creep into device or audible behavior. Decision: device
  open, render callbacks, buffering, device clock, audible output, any
  SDL-era audio-output retirement specified by the roadmap, and `-o` changes
  are explicit Track 015 scope and are rejected in Track 014.
- Risk: Assumptions about stereo blend or low-pass relocation could enter the
  adapter design. Decision: the adapter and dispatch make no such assumption;
  blend/filter remain in the legacy renderer/`-o` path per ADR-008 and
  ARTIFACTS.
- Risk: Track steps could be mistaken for blanket implementation authorization.
  Decision: S1 moved the Track to ACTIVE under explicit user approval; each
  remaining step requires its own approval and is executed in order.
- Decision: No new library, public API, public header, or public target is
  authorized; the executable and relevant tests directly compose private
  sources.

Version impact
- C API/ABI: Unchanged. The CoreAudio adapter, dispatch boundary, and
  conversion are private; no public declaration, library, or ABI is planned to
  be added.
- Module compatibility/extension: Unchanged. No TFMX module bytes, loader,
  format detection, module extension, or persistent module layout changes are
  planned.
- Interpreter/timing/audio: Unchanged. Legacy behavior remains unchanged:
  sequencing, mixing, `mixem` ordering, existing multimode in-mix clipping,
  legacy stereo blend, low-pass filtering, and the `-o` path are untouched,
  and no audible or timing behavior changes are planned. For the temporary
  Track 013 live route, the null adapter remains the non-macOS fallback,
  while the macOS bridge destination changed to the CoreAudio adapter per the
  approved Q2 mechanism (S3).
- Persistent DAW format/versioning: Unchanged. Blocks and converted
  representations are private in-memory values; no serialized DAW
  representation or version field changes are planned.
- Platform/audio-output: Changed privately and additively: at S3 for the
  dispatch boundary, adapter source, and composition, and at S4 for the
  interleaved Float32 client representation, which the adapter now implements
  (each signed-32 sample normalized by the 2^31 scale, INT32_MIN -> -1.0f and
  INT32_MAX -> +1.0f). A private macOS-only CoreAudio adapter and dispatch
  target exist at `src/audio_output/adapters/`, with no device opened, no
  render callback or buffer created, no device clock used, and no audible
  output produced; CMake composes the CoreAudio adapter only in macOS
  application and test builds, the audio bridge selects/routes to CoreAudio
  on macOS via `SYNTHTRACKER_AUDIO_OUTPUT_USE_DISPATCH`, and non-macOS builds
  select the null adapter, which remains the non-macOS fallback, not the
  macOS production destination; the temporary Track 013 null-adapter live
  route is not retired.
- Component/package boundaries: Changed privately at S3. Relevant macOS
  application/test composition gained the private adapter and dispatch target
  through direct source composition only (APPLE-gated
  `SYNTHTRACKER_COREAUDIO_ADAPTER_SOURCES`), while non-macOS composition
  selects the null adapter; tests compose/call the adapter directly through
  the `SYNTHTRACKER_AUDIO_OUTPUT_TEST_PROBE` observer; no public package,
  library, or target `Mixer` is created.

Open questions
- [x] Q1) What exactly is the CoreAudio PCM/client representation for Track 014
  (interleaved vs. deinterleaved layout, sample value mapping/normalization of
  signed-32 values, channel and rate metadata handling)? Resolved: the CoreAudio
  client representation is interleaved Float32. Each signed-32 sample value is
  normalized by the 2^31 scale (denominator 2147483648), i.e. the formula is
  `float32 = int32 / 2147483648.0f`: INT32_MIN (-2147483648) maps to -1.0f, and
  INT32_MAX (2147483647) maps to +1.0f, because binary32 rounding places
  (2^31 - 1)/2^31 at the nearest Float32 to 1.0f; every result is bounded
  inclusively within [-1.0f, +1.0f]. The stated endpoints assume the ambient
  IEEE 754 binary32 rounding behavior (round-to-nearest, ties-to-even) applied
  by Float32 division; no custom rounding or clipping rule beyond that division
  is specified. For TDD, an INT32_MAX assertion expects exactly +1.0f and an
  INT32_MIN assertion expects exactly -1.0f. Channel and rate metadata
  handling is not part of the Track 014 conversion contract and remains Track
  015 device-negotiation scope.
- [x] Q2) How does the private dispatch boundary select the CoreAudio adapter,
  and does the production executable or only test targets compose it while the
  null adapter remains the Track 013 live-route destination? Resolved at S2:
  CMake is planned to compose the CoreAudio adapter only in macOS application
  and test builds; the audio bridge is planned to select/route to CoreAudio on
  macOS; non-macOS is planned to select the null adapter. The null adapter
  remains as the non-macOS fallback, not the macOS production destination, and
  the temporary Track 013 null-adapter live route is not retired; tests are
  planned to compose/call the adapter directly. The specific private
  build/dispatch composition mechanism is CMake platform-gated composition, so
  no S3 build/dispatch decision remains and Q2 is fully resolved. Implemented
  at S3: CMake composes the CoreAudio adapter only in macOS application and
  test builds, the audio bridge selects/routes to CoreAudio on macOS via
  `SYNTHTRACKER_AUDIO_OUTPUT_USE_DISPATCH`, non-macOS selects the null adapter,
  and `test_audio_output` composes/calls the adapter directly through the
  `SYNTHTRACKER_AUDIO_OUTPUT_TEST_PROBE` observer.
- [x] Q3) Where does the private CoreAudio adapter live: in-place within
  `src/audio_output/` or a new co-located private folder? Resolved: a private
  co-located `adapters/` folder; the adapter lives at
  `src/audio_output/adapters/coreaudio_adapter.c` with its private co-located
  header `src/audio_output/adapters/coreaudio_adapter.h` (created at S3).
  `src/audio_output/`
  is planned to retain shared Audio Frame Block/dispatch ownership, and
  `adapters/` is planned to group future device-specific adapters.
- [x] Q4) What are the exact conversion contract rules: zero-frame acceptance,
  rejected blocks, overflow/clip policy, and any bounds that must not settle
  Track 015 device-negotiation behavior? Resolved: zero-frame (empty) blocks are
  accepted no-ops; only nonempty blocks with missing sample data or an
  unsupported channel/sample layout are rejected, before any conversion or
  output, and rejected blocks produce no output; a block whose `frame_count`
  exceeds `SIZE_MAX / 2` is rejected for representability (the interleaved
  stereo sample count `frame_count * 2` cannot be represented) before any
  multiplication, allocation, conversion, or observer output; a block whose
  converted `sample_count = frame_count * 2` exceeds `SIZE_MAX / sizeof(float)`
  is rejected for byte-size representability before any allocation, conversion,
  or observer output; valid nonzero blocks are converted into checked temporary
  `malloc` storage (no VLA), and an allocation failure rejects the block before
  any conversion or observer output, with the temporary storage freed
  immediately after the synchronous observer delivery; every result is
  bounded inclusively within [-1.0f, +1.0f] under the Q1 rounding assumption
  (INT32_MIN maps to -1.0f and INT32_MAX maps to +1.0f); and the mapping is
  the adapter-private Track 014 conversion contract, which does not settle
  Track 015 device-negotiation behavior.

Decision log
- Decision (status): This Track moved to ACTIVE under explicit user approval.
  S1 is checked; each remaining plan step requires its own approval and is
  executed in order.
- Decision (locked design): Audio Frame Blocks remain raw signed-32 values;
  `audio_output` dispatches them to the CoreAudio adapter; the adapter
  privately converts blocks to its CoreAudio PCM/client representation; no
  shared PCM converter component exists.
- Decision (boundary): Track 014 contains no device open, render callback,
  buffering, device clock, audible output, or legacy `-o` behavior change;
  device-facing live real-time CoreAudio routing and any SDL-era audio-output
  retirement specified by the roadmap remain Track 015; the macOS bridge
  selection of the CoreAudio adapter is the approved Q2 dispatch mechanism, not
  device routing.
- Decision (six-dimension impact, finalized at S2): C API/ABI is unchanged
  because the adapter, dispatch target, and conversion are private and are
  planned to add no public declaration, library, or ABI; module
  compatibility/extension is unchanged because no TFMX module bytes, loader,
  format detection, module extension, or persistent module layout change is
  planned; interpreter/timing/audio is unchanged because no sequencing,
  mixing, mixem ordering, in-mix clipping, blend, filter, or `-o` behavior
  changes and no audible or timing behavior changes are planned; legacy
  behavior remains unchanged, the null adapter remains the non-macOS fallback
  for the temporary Track 013 live route, and the macOS bridge destination is
  planned to change to the CoreAudio adapter per the approved Q2 mechanism;
  persistent DAW format/versioning is unchanged because blocks and converted
  representations are private in-memory values and no serialized DAW
  representation or version field changes are planned; platform/audio-output
  is planned to change privately and additively because a macOS-only private
  CoreAudio adapter and dispatch target with an interleaved Float32 client
  representation is planned to be added, with no device lifecycle and no
  audible output, and because CMake is planned to compose the adapter only in
  macOS application and test builds with the audio bridge selecting/routing to
  CoreAudio on macOS, while non-macOS builds are planned to select the null
  adapter, which remains the non-macOS fallback, not the macOS production
  destination, and the temporary Track 013 null-adapter live route is not
  retired; and component/package boundaries are planned to change privately
  because relevant executable/test composition is planned to gain the private
  adapter and dispatch target through direct source composition only, with
  tests planned to compose/call the adapter directly and no public package,
  library, or target `Mixer` planned. This decision is finalized at S2 before
  implementation and makes no assumption about stereo blend or low-pass
  relocation.
- Decision (Q3 placement): the private CoreAudio adapter is planned to live at
  `src/audio_output/adapters/coreaudio_adapter.c` with its private co-located
  header `src/audio_output/adapters/coreaudio_adapter.h` (both created at S3).
  `src/audio_output/` is planned to retain shared Audio Frame Block/dispatch
  ownership; `adapters/` is planned as a private co-located folder grouping
  future device-specific adapters.
- Decision (Q1, resolved at S2): the CoreAudio client representation is
  interleaved Float32. Each signed-32 sample value is normalized by the 2^31
  scale (denominator 2147483648): `float32 = int32 / 2147483648.0f`, so INT32_MIN
  (-2147483648) maps to -1.0f, INT32_MAX (2147483647) maps to +1.0f, because
  binary32 rounding places (2^31 - 1)/2^31 at the nearest Float32 to 1.0f, and
  every result is bounded inclusively within [-1.0f, +1.0f]; the stated
  endpoints assume the ambient IEEE 754 binary32 rounding behavior (round-to-
  nearest, ties-to-even) applied by Float32 division, and no custom rounding or
  clipping rule beyond that division is specified. For TDD, an INT32_MAX
  assertion expects exactly +1.0f and an INT32_MIN assertion expects exactly
  -1.0f. This mapping is the adapter-private PCM mapping chosen for Track 014
  testability only and does not settle Track 015 device format negotiation,
  ASBD, lifecycle, or scheduling.
- Decision (Q2, resolved at S2): CMake is planned to compose the CoreAudio
  adapter only in macOS application and test builds; the audio bridge is planned
  to select/route to CoreAudio on macOS; non-macOS is planned to select the null
  adapter, which remains as the non-macOS fallback, not the macOS production
  destination; the temporary Track 013 null-adapter live route is not retired;
  tests are planned to compose/call the adapter directly. No S3
  build/dispatch composition decision remains for Q2. Implemented at S3 as
  described in the Q2 resolution.
- Decision (S3 implementation evidence): S3 implemented the private dispatch
  boundary and the private CoreAudio adapter. The focused dispatch tests
  failed in the red phase with unresolved `audio_output_dispatch_submit` and
  `audio_output_coreaudio_adapter_test_set_observer` symbols; the green change
  added `audio_output_dispatch_submit` to `src/audio_output/audio_output.c/.h`
  (macOS selects the CoreAudio adapter; non-macOS keeps the null-adapter
  fallback) and created `src/audio_output/adapters/coreaudio_adapter.c/.h`
  with the `SYNTHTRACKER_AUDIO_OUTPUT_TEST_PROBE` observer, composed only on
  Apple platforms via CMake. The bridge in `src/audio.c` routes through the
  dispatch   boundary on macOS (`SYNTHTRACKER_AUDIO_OUTPUT_USE_DISPATCH`) and
  through the null adapter otherwise. S3 delivers dispatch and zero-frame
  acceptance only: it adds no device/framework/lifecycle/buffering behavior
  and no Float32 conversion (the interleaved Float32 client representation
  remains the S4 conversion chunk), and it produces no audible output.
- Decision (S4 implementation evidence): S4 implemented the adapter-private
  interleaved Float32 conversion inside `audio_output_coreaudio_adapter_submit`
  in `src/audio_output/adapters/coreaudio_adapter.c`. The TDD red evidence is
  recorded truthfully: the conversion red is an explicit rerun for evidence
  purposes, not contemporaneous historical provenance — the rerun temporarily
  used a safe non-converting adapter state that retained the oversized-frame
  guard and emitted no converted samples for nonempty blocks; the focused
  `test_audio_output` built and linked and failed behaviorally, with the
  expected converted sample counts 6 and 8 observed as 0, and the final
  approved implementation was restored afterward. The allocation-failure
  focused test had a safe behavioral red of its own (`0 != 1`: the submission
  result was `AUDIO_OUTPUT_SUBMIT_ACCEPTED` (0) where
  `AUDIO_OUTPUT_SUBMIT_REJECTED` (1) was expected), with no crash or hang. The
  original conclusion that the two overflow-guard tests had no clean
  pre-green assertion evidence (their unchecked pre-guard states imply unsafe
  crash/hang: the wrapped sample or byte count would reach a `malloc(0)` or an
  effectively unbounded conversion loop) is superseded: the reopened S4 gate
  produced a safe red for each guard separately via the test-only
  allocation-attempt counter under forced allocation failure, as recorded in
  the S4-reopened-gate decision below.
  The final green implementation rejects NULL and nonempty missing-frame
  blocks before any conversion or output; rejects `frame_count > SIZE_MAX / 2`
  before any multiplication, allocation, conversion, or observer output (the
  interleaved stereo sample count `frame_count * 2` must be representable);
  rejects `sample_count = frame_count * 2 > SIZE_MAX / sizeof(float)` before
  any allocation, conversion, or observer output (the byte size must be
  representable); accepts zero-frame blocks with no converted samples;
  converts each nonzero block's `{ left, right }` frames in order into checked
  temporary `malloc` storage (no VLA), rejecting the block without conversion
  or observer output if that allocation fails; converts via `(float)sample /
  2147483648.0f` (INT32_MIN -> -1.0f, INT32_MAX -> +1.0f, binary32 endpoint
  rounding, inclusive [-1.0f, +1.0f] bounds); delivers the converted samples
  synchronously to the observer; and frees the temporary storage immediately
  afterward. The deterministic forced-allocation-failure hook
  (`audio_output_coreaudio_adapter_test_set_allocation_failure`) is test-only
  and private: it is compiled only under
  `SYNTHTRACKER_AUDIO_OUTPUT_TEST_PROBE`, which CMake defines only for
  `test_audio_output`; production composition is a plain `malloc`. The final
  green focused suite (11/11) covers interleaved Float32 conversion/order, the
  2^31 mapping with endpoint rounding, inclusive bounds, zero-frame no output,
  malformed-input rejection before output, both representability guards, and
  allocation-failure rejection before output; validation passed with C23
  configure/build, full CTest 9/9, focused `test_audio_output` 11/11, and
  `git diff --check`. The "no safe red run was possible" conclusion for the
  two overflow-guard tests is superseded by the S4-reopened-gate decision
  below, which records the safe guard-red evidence and the final
  restored-green revalidation.
- Decision (S4 reopened gate, user-approved, completed): S4 was reopened
  because the two overflow-guard tests lacked safe pre-green evidence. The
  implemented conversion behavior, final green suite (11/11), and validation
  facts recorded above stand and are retained. The reopened gate is now
  satisfied under user approval and S4 is re-checked. A private test-only
  allocation-attempt counter was added to the adapter
  (`audio_output_coreaudio_adapter_test_reset_allocation_attempt_count` and
  `audio_output_coreaudio_adapter_test_allocation_attempt_count`,
  `SYNTHTRACKER_AUDIO_OUTPUT_TEST_PROBE`-gated): the checked allocation path
  increments the counter before the forced-allocation-failure check, so the
  counter observes whether allocation was attempted even when the hook
  prevents any real allocation. Each overflow guard was then removed alone in
  a temporary red run with the deterministic forced-allocation-failure hook
  enabled, and the safe red result is recorded separately per guard: (1) with
  only the sample-count guard (`frame_count > SIZE_MAX / 2`) absent, the
  sample-count overflow test failed cleanly because its expected
  allocation-attempt count 0 was observed 1 — the guard-removal state reached
  the checked allocation path, the forced failure returned NULL before any
  real allocation, and the submission was rejected with no crash, hang,
  conversion, or observer output; (2) with only the byte-size guard
  (`sample_count = frame_count * 2 > SIZE_MAX / sizeof(float)`) absent, the
  byte-size overflow test failed cleanly because its expected
  allocation-attempt count 0 was observed 1 — the same forced failure made
  the red run safe, with no crash, hang, conversion, or observer output. Both
  guards were then restored and remain restored in the final code; the
  test-only allocation-attempt counter remains as the probe (each overflow
  guard test asserts 0 allocation attempts, and the allocation-failure test
  asserts 1 attempt). Final revalidation with the guards restored is green:
  focused `test_audio_output` 11/11, C23 configure/build, full CTest 9/9, and
  `git diff --check`. A2 (its conversion suite includes the two overflow-guard
  tests) is re-checked with S4; A5's TDD-evidence clause is satisfied and A5
  stays pending on its remaining clauses; A1 and A4 remain checked; S1–S3,
  S5, S6, M1–M4, and all implemented behavior and validation facts are not
  altered.
- Decision (S5 structural review evidence, user-approved, completed): the
  read-only structural/composition review of the S3/S4 source/CMake state
  passed, and its concrete evidence is recorded in the S5 step evidence.
  Summary: conversion and temporary allocation are confined to the private
  adapter (`src/audio_output/adapters/coreaudio_adapter.c:75-82` conversion,
  `allocate_converted_samples` at lines 32-42 and the immediate free at line
  90; the file includes only its co-located private header and `<stdlib.h>`);
  `audio_output` carries only the raw signed-32 `audio_frame_block` and the
  dispatch boundary (`audio_output.h:7-15`, `audio_output.c:20-30`) with no
  PCM or device representation; no shared PCM converter source or CMake
  composition exists (`src/audio_output/` holds only `audio_output.[ch]` and
  `adapters/coreaudio_adapter.[ch]`); the adapter is composed by direct
  private source composition only on Apple platforms (`CMakeLists.txt:20-25`
  `if(APPLE)` gate into `SYNTHTRACKER_COREAUDIO_ADAPTER_SOURCES`, added
  directly to `SynthTracker`, `test_application`, and `test_audio_output` at
  lines 43, 119, and 178) with no CoreAudio/AudioToolbox library or framework
  link anywhere (all `target_link_libraries` link only `cmocka::cmocka` and
  `${SDL_LIBS}`); and the adapter has no device lifecycle, no render callback
  (the only observer is the test-probe-gated test observer,
  `coreaudio_adapter.c:5-30,84-88`), no buffering or queueing, no device
  clock or scheduling, and no audible output; the bridge
  (`src/audio.c:512-529`) retains raw-lane assembly and dispatch selection
  only. S5 changed no code, so the S4-era green validation stands. A3 and M3
  are checked with this evidence (M3's exact definition is satisfied by the
  already approved S3/S4 behavioral evidence plus this structural review).
- Decision (Q4, resolved at S2): the conversion contract accepts zero-frame
  (empty) blocks as no-ops; only nonempty blocks with missing sample data or an
  unsupported channel/sample layout are rejected, before any conversion or
  output, and rejected blocks produce no output; a block whose `frame_count`
  exceeds `SIZE_MAX / 2` is rejected for representability (the interleaved
  stereo sample count `frame_count * 2` cannot be represented) before any
  multiplication, allocation, conversion, or observer output; a block whose
  converted `sample_count = frame_count * 2` exceeds `SIZE_MAX / sizeof(float)`
  is rejected for byte-size representability before any allocation, conversion,
  or observer output; valid nonzero blocks are converted into checked temporary
  `malloc` storage (no VLA), and an allocation failure rejects the block before
  any conversion or observer output, with the temporary storage freed
  immediately after the synchronous observer delivery; every result is
  bounded inclusively within [-1.0f, +1.0f] under the Q1 rounding assumption
  (INT32_MIN maps to -1.0f and INT32_MAX maps to +1.0f); and the mapping is
  the adapter-private Track 014 conversion contract. These rules do not settle
  Track 015 device-negotiation behavior.
- Decision (deferred): Track 013's deferred Audio Output buffering, ownership,
  retention, queueing, timing, ordering, backpressure, and buffer-count policy
  remain deferred; Track 015 owns device lifecycle and the live real-time
  CoreAudio route.
- Decision (S6 completion, user-approved, completed): the S6 gate closed under
  explicit user approval with all gates met and the required roadmap revisions
  complete (canonical `SynthTracker modernization roadmap` revision 20 and
  `Phase 4 — Component extraction` revision 13). The exact changed-file
  inventory (12 files, including this Track), the documentation gate outcomes
  (README/ARCHITECTURE/GLOSSARY updated; ADR-008 corrected only to point to
  completed Track 013 while its decision stayed unchanged; ADR index,
  ADR-005/006, ASR, and ARTIFACTS unchanged with their recorded rationales),
  and the final compliant validation (C23 configure/build with no warnings,
  full CTest 9/9, focused `test_audio_output` 11/11, and `git diff --check`
  clean) are recorded in the S6 step evidence. The
  six-dimension Phase-4 compatibility conclusion stands: the only actual
  change is the private macOS post-mix dispatch to the device-free
  adapter-private Float32 conversion; C API/ABI, module compatibility, and
  persistent DAW format are unchanged, and interpreter/timing/audio keeps
  legacy TFMX sequencing, mixing, `mixem` ordering, in-mix clipping, stereo
  blend, low-pass filtering, and the audible `-o` path unchanged, with the
  null adapter remaining the non-macOS fallback. A5 and M4 are checked with
  this gate, and the Track moves to COMPLETED.

Plan (execution steps)
- [x] S1) On explicit user approval, move Track 014 to ACTIVE with
  synchronized folder, filename, title, and status; check this step before any
  implementation.
- [x] S2) Re-read this Track, state the next unchecked step, resolve Q1, Q2,
  Q4 (dispatch mechanics, conversion representation, composition; Q3 adapter
  placement is resolved: `src/audio_output/adapters/coreaudio_adapter.c` with
  its private co-located header `coreaudio_adapter.h`), and record the
  finalized six-dimension impact decision before writing code or tests.
- [x] S3) TDD chunk: add failing focused dispatch coverage, implement the
  smallest private dispatch-to-CoreAudio-adapter behavior, prove behavioral
  evidence of exact frame count/order delivery and zero-frame acceptance with
  no device involvement, refactor, validate, and update this Track. Evidence:
  the red phase failed the focused `test_audio_output` link with unresolved
  `audio_output_dispatch_submit` and
  `audio_output_coreaudio_adapter_test_set_observer` symbols; the green change
  added `audio_output_dispatch_submit` (macOS -> CoreAudio adapter,
  non-macOS -> null fallback) and created the private CoreAudio adapter with a
  test observer, composed only on Apple platforms; the two new focused tests
  prove exact frame count and frame order delivery, zero-frame acceptance,
  and the macOS null-destination negative control
  (`fallback.accepted_block_count/frame_count` stay 0 on macOS; the non-macOS
  branch asserts the null fallback counters instead). Validation passed:
  C23 configure/build on macOS, full CTest 9/9, focused `test_audio_output`
  6/6, and a diff check scoping the change to the new
  `src/audio_output/adapters/` files plus the audio_output, audio bridge,
  test, and CMake edits.
- [x] S4) TDD chunk: add failing focused conversion coverage, implement the
  smallest adapter-private conversion of zero- and nonzero-frame signed-32
  blocks to the resolved CoreAudio PCM/client representation, and prove the
  resolved adapter-private PCM mapping behaviorally: exact mapped output under
  every explicit range/rounding/clipping rule, without requiring exact source
  value preservation, refactor, validate, and update this Track. Evidence:
  the TDD red evidence is recorded truthfully: the conversion red is an
  explicit rerun, not contemporaneous historical provenance — the rerun
  temporarily used a safe non-converting adapter state that retained the
  oversized-frame guard and emitted no converted samples for nonempty blocks;
  the focused `test_audio_output` built and linked and failed behaviorally,
  with the expected converted sample counts 6 and 8 observed as 0, and the
  final approved implementation was restored afterward. The allocation-failure
  focused test had a safe behavioral red of its own (`0 != 1`:
  `AUDIO_OUTPUT_SUBMIT_ACCEPTED` (0) where `AUDIO_OUTPUT_SUBMIT_REJECTED` (1)
  was expected), with no crash or hang. The original conclusion that the two
  overflow-guard tests had no clean pre-green assertion evidence (their
  unchecked pre-guard states imply unsafe crash/hang) is superseded by the
  reopened-gate safe red evidence recorded below: with the test-only
  allocation-attempt counter and forced allocation failure, each guard
  removal red is safe and clean (`0 != 1` on the expected allocation-attempt
  count). The final
  green conversion lives inside `audio_output_coreaudio_adapter_submit` only:
  zero-frame blocks are accepted with no converted output; NULL blocks and
  nonempty blocks with missing frames are rejected before any conversion or
  output; a `frame_count` above `SIZE_MAX / 2` is rejected before
  multiplication, allocation, conversion, and observer output; a `sample_count`
  above `SIZE_MAX / sizeof(float)` is rejected before allocation, conversion,
  and observer output; valid nonzero blocks convert into checked temporary
  `malloc` storage (no VLA) to interleaved Float32 via `(float)sample /
  2147483648.0f` per channel in frame order, with INT32_MIN mapping to -1.0f
  and INT32_MAX to +1.0f under binary32 rounding, every result bounded
  inclusively within [-1.0f, +1.0f]; an allocation failure rejects the block
  before conversion and observer output, and the temporary storage is freed
  immediately after the synchronous observer delivery. The deterministic
  forced-allocation-failure hook and the test-only allocation-attempt
  counter/reset/read probe are private
  (`SYNTHTRACKER_AUDIO_OUTPUT_TEST_PROBE`-gated, defined only for
  `test_audio_output`): the checked allocation path increments the counter
  before the forced-failure check, so a guard rejection is observable as 0
  allocation attempts and a guard-absent forced failure as 1 attempt. The
  final green suite (11/11) covers interleaved
  Float32 conversion/order, the 2^31 mapping with endpoint rounding, inclusive
  bounds, zero-frame no output, malformed-input rejection before output, both
  representability guards, and allocation-failure rejection before output.
  Validation passed: C23 configure/build, full CTest 9/9, focused
  `test_audio_output` 11/11, and a diff check scoping the change to the
  existing adapter, test, and Track files (no new files at S4). This step was
  REOPENED under user approval because the two overflow-guard tests lacked
  safe pre-green evidence, and is now re-checked: the reopened gate added the
  test-only allocation-attempt counter and produced a safe red for each guard
  separately — with only the sample-count guard (`frame_count > SIZE_MAX / 2`)
  removed, the sample-count overflow test failed cleanly because its expected
  allocation-attempt count 0 was observed 1, and with only the byte-size guard
  (`sample_count = frame_count * 2 > SIZE_MAX / sizeof(float)`) removed, the
  byte-size overflow test failed cleanly because its expected
  allocation-attempt count 0 was observed 1; in both runs the forced
  test-only allocation failure prevented any real allocation, and each
  dedicated test failed without crash or hang. Both guards were then restored
  and remain restored; revalidation with the guards restored is green:
  focused `test_audio_output` 11/11, C23 configure/build, full CTest 9/9, and
  `git diff --check`.
- [x] S5) Structural/composition evidence: record explicit source/CMake
  composition review proving that the adapter performs no device open/close,
  no render-callback registration or invocation, no buffering, no device
  clock, and no audible output, and that no shared PCM converter component
  exists (conversion code resides in the adapter; `audio_output` carries no
  PCM or device representation). These structural absences require that
  review, not component or composition tests; component tests remain limited
  to observable dispatch and deterministic device-free conversion. Validate
  and update this Track. Evidence: the completed structural review (the
  read-only source/CMake composition review of the S3/S4 state performed
  under user approval, independently re-verified) passed and proved, with
  concrete file/line evidence: (1) conversion and temporary allocation exist
  only in the private adapter — the signed-32-to-Float32 conversion is the
  `(float)sample / 2147483648.0f` division per channel in frame order at
  `src/audio_output/adapters/coreaudio_adapter.c:75-82`, the sole temporary
  allocation is `allocate_converted_samples` (lines 32-42) freed immediately
  after the synchronous observer delivery (line 90), and the file includes
  only its private co-located header and `<stdlib.h>` (lines 1-3), with no
  CoreAudio or other framework include; (2) `audio_output` remains raw
  signed-32 dispatch — `src/audio_output/audio_output.h:7-15` defines only
  `audio_frame` (`int32_t left`/`right`) and `audio_frame_block`, and
  `audio_output.c:20-30` (`audio_output_dispatch_submit`) only dispatches
  (CoreAudio adapter on `__APPLE__`, null-adapter fallback otherwise) and
  carries no PCM or device representation; (3) no shared PCM converter source
  or CMake composition exists — `src/audio_output/` contains only
  `audio_output.c/.h` and `adapters/coreaudio_adapter.c/.h`, and CMake lists
  no converter source; (4) the adapter is composed only on Apple platforms by
  direct private source composition with no CoreAudio library/framework
  link — `CMakeLists.txt:20-25` appends `coreaudio_adapter.c` only inside
  `if(APPLE)` into `SYNTHTRACKER_COREAUDIO_ADAPTER_SOURCES`, which is added
  directly to `SynthTracker` (line 43), `test_application` (line 119), and
  `test_audio_output` (line 178), and every `target_link_libraries` in the
  file links only `cmocka::cmocka` and `${SDL_LIBS}`; and (5) the adapter
  performs no device open/close, registers no render callback (the only
  observer is the `SYNTHTRACKER_AUDIO_OUTPUT_TEST_PROBE`-gated test observer,
  `coreaudio_adapter.c:5-30,84-88`, not a device render callback), creates no
  buffer or queue (its sole transient allocation is the checked temporary
  storage freed at line 90), uses no device clock or scheduling, and produces
  no audible output; the bridge retains raw lanes and dispatch selection
  only — `src/audio.c:512-529` assembles raw signed-32 `{ left, right }`
  frames from `tbuf` into `audio_output_workspace` and submits through
  `audio_output_dispatch_submit` (line 525) under
  `SYNTHTRACKER_AUDIO_OUTPUT_USE_DISPATCH` or through
  `audio_output_null_adapter_submit` (line 528) otherwise, with no conversion
  added at the bridge. S5 changed no code: the review confirms the S3/S4
  state, so the recorded S4-era green validation (C23 configure/build, full
  CTest 9/9, focused `test_audio_output` 11/11, and `git diff --check`)
  stands unchanged. S5 is checked; A3 is checked with this evidence, and M3
  is checked because its exact definition is satisfied by the already
  approved S3/S4 behavioral evidence plus this structural review.
- [x] S6) Confirm direct private source composition and no library/public API/
  header/target `Mixer`; record the exact changed-file inventory, compatibility
  evidence, and documentation gate outcomes (README, ARCHITECTURE, ASR, ADR,
  GLOSSARY, ARTIFACTS each audited or given an individual unchanged
  rationale), then run the full compliant C23/CTest validation and record final
  evidence before completion. Evidence: the exact changed-file inventory is
  12 files — `CMakeLists.txt`; `README.md`; `docs/ARCHITECTURE.md`;
  `docs/GLOSSARY.md`;
  `docs/adr/ADR-008-audio-frame-block-mixed-value-boundary.md`;
  `src/audio.c`; `src/audio_output/audio_output.c`;
  `src/audio_output/audio_output.h`; the new private
  `src/audio_output/adapters/coreaudio_adapter.c` and
  `src/audio_output/adapters/coreaudio_adapter.h`; and
  `tests/audio_output/test_audio_output.c`, with this Track file as the
  twelfth — with the adapter pair and the Track file as the only new files.
  Direct private source composition with no new library, public API, public
  header, or public target `Mixer` is confirmed by the recorded S5 structural
  review (APPLE-gated `SYNTHTRACKER_COREAUDIO_ADAPTER_SOURCES` added only to
  `SynthTracker`, `test_application`, and `test_audio_output`; no
  CoreAudio/AudioToolbox link). The documentation gate outcomes are recorded
  individually: README, ARCHITECTURE, and GLOSSARY were updated for the
  CoreAudio Adapter, Audio Output dispatch boundary, and Audio Frame Block
  terminology; ADR-008 was corrected only to point to the completed Track 013
  (its ACTIVE-to-completed path/status reference), while ADR-008's decision
  stayed unchanged; and the ADR index (`docs/ADR.md`), ADR-005/006, ASR, and
  ARTIFACTS were audited and remain unchanged, with the recorded rationales
  that ASR-009 and ADR-008 already normatively cover the signed-32 mixed-value
  boundary and adapter-owned destination conversion and ARTIFACTS already
  carries the aligned Audio Frame Block vocabulary, so no further doc change
  was required. The final compliant validation passed: C23 configure/build on
  macOS/Clang with no warnings, full CTest 9/9, focused `test_audio_output`
  11/11, and `git diff --check` clean. The final reconciliation edit re-ran
  `git diff --check` and it passed clean with no whitespace errors. The six-dimension Phase-4 compatibility conclusion
  records the only actual change as the private macOS post-mix dispatch to the
  device-free adapter-private Float32 conversion, with legacy
  TFMX/module/interpreter/timing/audible `-o`/blend/low-pass behavior
  unchanged as applicable. S6 is checked; A5 and M4 are checked with it.

Current inventory
- `src/audio_output/audio_output.h` and `src/audio_output/audio_output.c`:
  the private signed-32 Audio Frame Block implementation — `audio_frame`
  (`int32_t left`, `int32_t right`), `audio_frame_block` (frame count plus
  borrowed interleaved frames), `audio_output_submit_result`, the synchronous
  production null adapter with accepted block/frame counters and a
  target-guarded test inspection probe, and (S3) the private dispatch boundary
  `audio_output_dispatch_submit`, which routes to the CoreAudio adapter on
  macOS (`__APPLE__`) and to the null-adapter fallback elsewhere. This is the
  dispatch home for Track 014.
- Created at S3:
  `src/audio_output/adapters/coreaudio_adapter.c` and its private co-located
  header `src/audio_output/adapters/coreaudio_adapter.h` — the Track 014
  CoreAudio adapter under the shared `audio_output` dispatch home:
  `audio_output_coreaudio_adapter_submit` accepts valid blocks (rejecting a
  NULL block or a nonempty block with missing frame data before any
  conversion or output) and exposes the `SYNTHTRACKER_AUDIO_OUTPUT_TEST_PROBE`
  observer; zero-frame blocks are accepted no-ops with no converted samples;
  blocks whose `frame_count` exceeds `SIZE_MAX / 2` are rejected before any
  multiplication, allocation, conversion, or observer output, and blocks whose
  converted `sample_count = frame_count * 2` exceeds `SIZE_MAX / sizeof(float)`
  are rejected before any allocation, conversion, or observer output; S4
  implemented the adapter-private interleaved Float32 conversion into checked
  temporary `malloc` storage (no VLA) (`(float)sample / 2147483648.0f` per
  channel in frame order, with the observer receiving the converted samples),
  rejecting the block without conversion or observer output if the allocation
  fails, and freeing the temporary storage immediately after the synchronous
  observer delivery. The deterministic forced-allocation-failure hook
  (`audio_output_coreaudio_adapter_test_set_allocation_failure`) is test-only
  and private: compiled only under `SYNTHTRACKER_AUDIO_OUTPUT_TEST_PROBE`,
  which CMake defines only for `test_audio_output`; production composition
  performs a plain `malloc`. The private test-only allocation-attempt
  counter/reset/read probe
  (`audio_output_coreaudio_adapter_test_reset_allocation_attempt_count` and
  `audio_output_coreaudio_adapter_test_allocation_attempt_count`, same
  test-only gate) observes whether allocation was attempted: the checked
  allocation path increments the counter before the forced-failure check, so
  a guard rejection is observable as 0 attempts and a guard-absent forced
  failure as 1 attempt. It contains no CoreAudio framework, device
  lifecycle, render callback, buffering, device clock, or audible output. It
  is composed only on Apple platforms.
- `src/audio.c`: the legacy renderer plus the Track 013 private live-only
  bridge immediately after `mixem` that assembles interleaved signed-32 frames
  and submits through `audio_output_dispatch_submit` (line 525) under
  `SYNTHTRACKER_AUDIO_OUTPUT_USE_DISPATCH` on macOS, and through
  `audio_output_null_adapter_submit` (line 528) otherwise. Its
  strict-profile, no-SDL lifecycle, and `-o` behavior remain unchanged by Track
  014, and the null adapter remains the non-macOS fallback; the macOS bridge
  destination selects the CoreAudio adapter per the approved Q2 mechanism.
- `tests/audio_output/test_audio_output.c`: component evidence for the null
  adapter (zero/nonzero blocks, counters, rejection, no retention); (S3) two
  dispatch tests proving exact frame count/order delivery and zero-frame
  acceptance through the `SYNTHTRACKER_AUDIO_OUTPUT_TEST_PROBE` observer on
  macOS (the dispatch order test asserts the exact interleaved Float32
  converted values), with the null-destination negative control (the
  `fallback` counters stay at 0 on macOS) and the null-fallback counter
  assertions on non-macOS; and (S4) five adapter-safety/conversion tests (11
  registered tests total) proving the final conversion contract: exact
  interleaved Float32 mapped output under the 2^31 mapping with endpoint
  rounding (INT32_MIN ->
  -1.0f, INT32_MAX -> +1.0f, INT32_MIN + 1 -> -1.0f, INT32_MAX - 1 -> +1.0f,
  +/-0.5f, -0x1p-31f) with inclusive [-1.0f, +1.0f] bounds; malformed-input
  (NULL block, nonempty block with missing frames) rejection before any
  converted output; `frame_count > SIZE_MAX / 2` rejection before any
  converted output; `sample_count > SIZE_MAX / sizeof(float)` rejection before
  any converted output; and deterministic allocation-failure rejection before
  any converted output. The two overflow-guard tests and the
  allocation-failure test use the test-only allocation-attempt counter probe
  (`audio_output_coreaudio_adapter_test_reset_allocation_attempt_count` and
  `audio_output_coreaudio_adapter_test_allocation_attempt_count`): each guard
  test sets the forced-allocation-failure hook and asserts the counter is 0
  (the guard rejects before any allocation attempt), while the
  allocation-failure test asserts the counter is 1 (the checked allocation
  path increments it, then the forced failure rejects without any real
  allocation). All three test hooks — observer, forced-allocation failure,
  and allocation-attempt counter — are compiled only under the
  `SYNTHTRACKER_AUDIO_OUTPUT_TEST_PROBE` definition that CMake applies only
  to `test_audio_output`.
- `tests/audio_output/recording_sink.c/.h` and
  `tests/audio_output/test_audio_routing.c`: test-only recording sink and
  routing evidence for the Track 013 bridge; not linked into production.
- `tests/application/test_application_strict_profile.c` and
  `tests/application/test_application_audio_lifecycle.c`: application-level
  strict-profile and no-SDL lifecycle evidence; unchanged by Track 014.
- `CMakeLists.txt`: direct private composition for `SynthTracker`,
  `test_application`, `test_application_strict_profile`,
  `test_application_audio_lifecycle`, `test_audio_output`, and
  `test_audio_routing`; no library or public target. Track 014 added (S3) the
  APPLE-gated `SYNTHTRACKER_COREAUDIO_ADAPTER_SOURCES` composition of the
  private CoreAudio adapter into `SynthTracker`, `test_application`, and
  `test_audio_output`, the macOS-only
  `SYNTHTRACKER_AUDIO_OUTPUT_USE_DISPATCH` definition for `SynthTracker` and
  `test_application`, and the `SYNTHTRACKER_AUDIO_OUTPUT_TEST_PROBE` definition
  for `test_audio_output`.
- Validation baseline: C23 on macOS/Clang, CMocka via Homebrew, SDL 1.2-era
  linkage; commands in `docs/TESTING.md`. S3 validated with configure/build,
  full CTest 9/9, focused `test_audio_output` 6/6, and a scoped diff check;
  S4 validated with C23 configure/build, full CTest 9/9, focused
  `test_audio_output` 11/11, and `git diff --check` with a scoped diff check
  (the S4 diff adds no files: conversion and tests changed within the
  existing adapter and test files). The reopened S4 gate revalidated green
  with both guards restored: focused `test_audio_output` 11/11, C23
  configure/build, full CTest 9/9, and `git diff --check`. S5 is an
  evidence-recording step: the structural/composition review changed no code,
  so the guards-restored green validation above stands unchanged.   S6
  completed the final compliant validation: C23 configure/build with no
  warnings, full CTest 9/9, focused `test_audio_output` 11/11, and
  `git diff --check` clean. The final reconciliation edit re-ran
  `git diff --check` and it passed clean with no whitespace errors.

Artifacts
- Living roadmap: `SynthTracker modernization roadmap` (project memory
  `project/synthtracker/roadmaps`, canonical index
  `mem_2bb709917c2d4f3fbeed23c715b52dd0`, revision 20) and `Phase 4 — Component
  extraction` (`mem_2d50e6f918124a8bb0074b413804e9eb`, revision 13), Stage 3
  Audio Output extraction on branch `stage/04-03-audio-output-extraction`.
  The roadmap names Track 014 as next roadmap work: draft it before
  implementation for an independently testable CoreAudio adapter; Track 015
  remains planned for the live real-time CoreAudio route and, as defined by the
  roadmap, SDL-era audio-output retirement; their adapter work formats the
  generic mixed-value boundary for CoreAudio and implies no target Mixer
  internal processing.
- [`docs/adr/ADR-008-audio-frame-block-mixed-value-boundary.md`](../../../docs/adr/ADR-008-audio-frame-block-mixed-value-boundary.md)
  — accepted normative mixed-value boundary; destination format adaptation is
  an adapter responsibility; blend/filter/PCM packing stay in the legacy
  renderer path.
- [`docs/ASR.md`](../../../docs/ASR.md) — ASR-009 (signed-32 ordered-block
  invariants, Target) and ASR-006 (isolated future audio output behind a
  device-independent port, Target).
- [`docs/adr/ADR-005-target-daw-component-foundation.md`](../../../docs/adr/ADR-005-target-daw-component-foundation.md)
  — target foundation; CoreAudio and future adapters implement the
  device-independent Audio Output boundary (target only, not implemented by
  this Track).
- [`docs/ARTIFACTS.md`](../../../docs/ARTIFACTS.md) — artifact vocabulary and
  deferred contract questions for the Audio Frame Block.
- [`docs/ARCHITECTURE.md`](../../../docs/ARCHITECTURE.md),
  [`docs/GLOSSARY.md`](../../../docs/GLOSSARY.md), and
  [`docs/ADR.md`](../../../docs/ADR.md) — aligned references for Audio Output,
  CoreAudio Adapter, and Audio Frame Block terminology.
- [Track 013](../../COMPLETED/2026/TRACK_013_COMPLETED_legacy_mixed_value_audio_output_routing.md)
  — completed Stage 3 routing Track whose null-adapter live route and `-o`
  baseline this Track preserves.
- [Track 012](../../COMPLETED/2026/TRACK_012_COMPLETED_audio_output_null_adapter.md)
  — historical private fixed-PCM evidence under superseded ADR-007; not
  normative for this Track.

Completion notes
- ACTIVE: this Track records the PORE problems, objective, locked design,
  scope, risks, open questions, decision log, plan, inventory, and artifact
  citations for TRACK_014. S1 and S2 are complete (S2 resolved Q1, Q2, and Q4,
  finalized the six-dimension impact decision, and recorded the Q2
  build/dispatch composition mechanism), and M2 is checked. S3 is complete
  and checked as recorded in its step evidence. S4 was reopened under user
  approval because its two overflow guards lacked safe pre-green evidence,
  and is now re-checked: the reopened gate produced the required safe red
  evidence and both guards are restored. S5 is complete and checked with the
  recorded source/CMake composition review evidence; A3 and M3 are checked
  with it. The TDD red evidence recorded is
  truthful — the conversion red
  is an explicit rerun, not contemporaneous historical provenance (the rerun
  temporarily used a safe non-converting adapter state that retained the
  oversized-frame guard and emitted no converted samples for nonempty blocks;
  the focused `test_audio_output` built and linked and failed behaviorally,
  with the expected converted sample counts 6 and 8 observed as 0, and the
  final approved implementation was restored afterward); the allocation-
  failure focused test had a safe behavioral red of its own (`0 != 1`:
  `AUDIO_OUTPUT_SUBMIT_ACCEPTED` (0) where `AUDIO_OUTPUT_SUBMIT_REJECTED` (1)
  was expected, no crash or hang). The original conclusion that the two
  overflow-guard tests had no clean pre-green assertion evidence — their
  unchecked pre-guard states imply unsafe crash/hang, so their evidence would
  be green/verification only — is superseded: the reopened gate added the
  test-only allocation-attempt counter, and with only the sample-count guard
  (`frame_count > SIZE_MAX / 2`) removed the sample-count overflow test
  failed cleanly because its expected allocation-attempt count 0 was observed
  1, and with only the byte-size guard (`sample_count = frame_count * 2 >
  SIZE_MAX / sizeof(float)`) removed the byte-size overflow test failed
  cleanly because its expected allocation-attempt count 0 was observed 1; in
  each red run the forced test-only allocation failure prevented any real
  allocation, with no crash or hang. Both guards were then restored and
  remain restored. The final green
  implementation privately converts zero- and nonzero-frame signed-32 blocks
  to the interleaved Float32 client representation inside
  `audio_output_coreaudio_adapter_submit` only: zero-frame blocks are accepted
  with no converted output; NULL blocks and nonempty missing-frame blocks are
  rejected before any conversion or output; `frame_count > SIZE_MAX / 2` is
  rejected before multiplication, allocation, conversion, and observer output;
  `sample_count > SIZE_MAX / sizeof(float)` is rejected before allocation,
  conversion, and observer output; valid blocks convert into checked temporary
  `malloc` storage (no VLA) via `(float)sample / 2147483648.0f` per channel in
  frame order with INT32_MIN -> -1.0f, INT32_MAX -> +1.0f, and inclusive
  [-1.0f, +1.0f] bounds, rejecting the block without conversion or observer
  output if the allocation fails, and freeing the temporary storage
  immediately after the synchronous observer delivery; the deterministic
  forced-allocation-failure hook and the allocation-attempt counter/reset/read
  probe are test-only and private
  (`SYNTHTRACKER_AUDIO_OUTPUT_TEST_PROBE`-gated, defined only for
  `test_audio_output`; production composition is a plain `malloc`). The
  final green suite (11/11) covers interleaved Float32 conversion/order, the
  2^31 mapping with endpoint rounding, inclusive bounds, zero-frame no
  output, malformed-input rejection before output, both representability
  guards, and allocation-failure rejection before output, with the
  allocation-attempt counter asserting 0 attempts under each guard and 1
  attempt under the forced allocation failure. Validation: C23
  configure/build passed, full CTest 9/9 passed, focused `test_audio_output`
  11/11 passed, and `git diff --check` passed with the diff scoped to the
  existing adapter, test, and Track files (no new files at S4). S4 is
  checked with the reopened-gate safe red evidence recorded above (each
  guard-removal red failed cleanly on the expected allocation-attempt count 0
  observed as 1, with no crash or hang, under forced test-only allocation
  failure), and the guards-restored revalidation is green: focused
  `test_audio_output` 11/11, C23 configure/build, full CTest 9/9, and
  `git diff --check`. A2 (its
  conversion suite includes the two overflow-guard tests) is re-checked with
  S4; A5's TDD-evidence clause is satisfied and A5 remains pending on its
  remaining clauses; A1 (dispatch
  delivery evidence) and A4 (six-dimension decision recorded before
  implementation) remain checked; A3 is checked with S5.
- S6 is complete and checked under explicit user approval with all gates met:
  the exact changed-file inventory (12 files: CMakeLists; README; ARCHITECTURE;
  GLOSSARY; the ADR-008 record corrected only to point to completed Track 013;
  audio.c; audio_output.c/.h; the new private adapters coreaudio_adapter.c/.h;
  test_audio_output.c; and this Track) is recorded with the adapter pair and
  the Track as the only new files; direct private source composition with no
  library, public API, public header, or public target `Mixer` is confirmed by
  the S5 structural review; the documentation gate records
  README/ARCHITECTURE/GLOSSARY as updated, ADR-008 as corrected only to point
  to the completed Track 013 with its decision unchanged, and the ADR index
  (`docs/ADR.md`), ADR-005/006, ASR, and ARTIFACTS as audited and unchanged
  with their recorded rationales (the normative ADR-008 and ASR-009 boundary
  plus the aligned ARTIFACTS vocabulary already cover this Track's outcome, so
  no further doc change was required); and the final compliant validation
  passed: C23 configure/build with no warnings, full CTest 9/9, focused
  `test_audio_output` 11/11, and `git diff --check` clean, with the final
  reconciliation edit re-running `git diff --check` clean (no whitespace
  errors). The
  six-dimension Phase-4 compatibility conclusion names the only actual change
  as the private macOS post-mix dispatch to the device-free adapter-private
  Float32 conversion, with legacy TFMX/module/interpreter/timing/audible
  `-o`/blend/low-pass behavior unchanged as applicable. A5 and M4 are checked
  with S6. The transparent S4 evidence limitations are preserved above:
  the conversion red is an explicit rerun, not contemporaneous historical
  provenance, and the two overflow-guard reds are the reopened-gate safe
  guard-removal runs under forced test-only allocation failure, with no
  crash or hang.
  No device behavior and no audible output are implemented; device
  lifecycle, render callbacks, buffering, device clock, and audible output
  remain Track 015.
- Roadmap reconciliation (completed at S6): the linked Phase 4 Stage 3 roadmap
  was inspected against this Track's outcome under the required revisions —
  canonical `SynthTracker modernization roadmap` revision 20 and `Phase 4 —
  Component extraction` revision 13 — which mark Track 014 as delivered and
  name Track 015 as the next roadmap work (live real-time CoreAudio route and
  SDL live-route retirement). The delivered outcome matches the revised
  roadmap's Stage 3 content and sequencing, so no further roadmap revision is
  needed and the roadmap remains current.
- This Track's status was moved to COMPLETED: the folder, filename, title, and
  status are synchronized at
  `.backlog/COMPLETED/2026/TRACK_014_COMPLETED_coreaudio_adapter.md`, and all
  plan steps (S1-S6), acceptance criteria (A1-A5), and milestones (M1-M4) are
  checked.
- Commit, push, and changelog recording remain pending and not yet performed;
  no Git-history change occurs in this move.
