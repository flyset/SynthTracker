# TRACK TRACK_013 [COMPLETED]: legacy_mixed_value_audio_output_routing

Track
- ID: TRACK_013
- Repository: SynthTracker
- Branch: stage/04-03-audio-output-extraction
- Current path: .backlog/COMPLETED/2026/TRACK_013_COMPLETED_legacy_mixed_value_audio_output_routing.md
- Status: COMPLETED

Problems (PORE)
- P1: As a SynthTracker maintainer, I cannot exercise the accepted Audio Frame
  Block boundary from the legacy playback path because `src/audio.c` currently
  couples its live delivery to the legacy SDL lifecycle, ring queue, and device
  path rather than submitting the renderer's mixed values to a private sink.
- P2: As a Phase 4 contributor, I need exact evidence for the legacy renderer's
  left/right lane mapping and multimode clipping at the new boundary, because a
  route placed after conversion, blending, filtering, or packing would test a
  different representation and could duplicate renderer behavior.
- P3: As a compatibility maintainer, I need unsupported temporary-live profiles
  to fail before legacy live initialization, playback, or submission, because
  silently adapting unsupported rate, bit-depth, channel, or width options would
  obscure the intentionally narrow route.
- P4: As a maintainer, I need finite compatible live playback to complete without
  opening or draining an SDL device, because the temporary route is a synchronous
  validation path and must not depend on device delivery, ring throttling, or a
  final drain.
- P5: As a future component maintainer, I need the route to remain private and
  narrowly composed, because this Track must exercise the accepted boundary
  without creating a public API, a library, or an implemented target `Mixer`.

Objective
- Add a private temporary live bridge immediately after `mixem` that submits
  interleaved signed-32 `{ left, right }` frames synchronously to a production
  null adapter, with a separate test-only recording sink for exact-value
  evidence, while preserving the existing legacy renderer and `-o` file path.

Non-negotiables
- This Track is ACTIVE. S1 is checked, and implementation proceeds only through
  the declared next unchecked plan step or TDD chunk.
- Every behavior change follows TDD: add a focused failing automated test,
  implement the smallest passing change, refactor only as needed, run the
  required validation, and record evidence in this Track.
- All new SynthTracker-owned production and test source is C23 or a later ISO C
  standard. C++ is not a project direction. New project-owned headers remain
  private and co-located; none may be added under retired `include/`.
- ADR-008 and ASR-009 are normative for this Track. The Audio Frame Block is an
  ordered sequence of zero or more interleaved signed-32 `{ left, right }`
  frames. It is never serialized PCM and never device-native data.
- Track 012 and ADR-007 are historical evidence of the superseded fixed-format
  design only. Neither defines a normative representation, validation contract,
  adapter contract, or requirement for this Track.
- The temporary legacy live bridge is a small live-only `src/audio.c` branch
  immediately after `mixem`. For each frame, `tbuf[HALFBUFSIZE+i]` maps to
  `left` and `tbuf[i]` maps to `right`. The bridge preserves existing multimode
  in-mix clipping and ordering and adds no clipping.
- The production null adapter accepts and discards blocks synchronously. It
  counts accepted blocks and accepted frames only, retains no pointers and no
  values after the call, and performs no device delivery. A separate test-only
  recording sink captures exact frame values for routing tests; it is not a
  production consumer or public API.
- No new library or public API is created. The executable and relevant tests
  use direct private source composition. Private component naming and CMake
  target lists are resolved at S2 before implementation.
- The temporary live strict profile accepts exactly 44.1 kHz and `-b 1` or
  `-b 2`. It rejects every other `-b`, `-8`, and `-w` setting, and every
  non-44.1 kHz rate, before any legacy live initialization, playback, or block
  submission. Both allowed blend settings submit raw, unblended lanes.
- `-o` is exempt from the strict live profile and retains the full current
  legacy setup, conversion, ring, and file-output behavior. It is not routed to
  either temporary live sink.
- The temporary live route intentionally omits legacy blend, filter, and PCM
  packing. It bypasses SDL device open, pause, callback, ring queue, throttle,
  final drain, and SDL teardown while safely completing the legacy cleanup that
  is required for a finite return.
- A future `Mixer` outputs the same Audio Frame Blocks, but its internal
  processing point and processing remain open. This Track does not extract,
  implement, or claim a target `Mixer`.
- All buffering, queueing, timing, backpressure, and general ownership policy
  remains deferred beyond synchronous-call completion. The null adapter's
  no-retention behavior is required for this Track and does not settle a future
  consumer contract.
- Phase 4 compatibility is a temporary development scaffold, not a
  SynthTracker v1 compatibility promise. Evidence must distinguish unchanged
  sequencing, mixing, and `-o` behavior from the intentional strict temporary
  live-output change.

Acceptance criteria
- [x] A1) [P1, P2] After S2 resolves the private validation/result API, focused
  component tests prove zero-frame and nonzero-frame signed-32 block behavior.
  Zero frames are accepted as an ordered empty block; nonzero blocks preserve
  every signed-32 left/right component exactly under the resolved private
  validation rules, with no serialized or device-native payload.
- [x] A2) [P1, P2] A focused routing test using the test-only recording sink
  proves exact interleaved frame order and lane mapping: each `left` value comes
  from `tbuf[HALFBUFSIZE+i]`, each `right` value comes from `tbuf[i]`, and the
  sequence follows the existing `mixem` order.
- [x] A3) [P2] The recording-sink test proves existing multimode in-mix clipping
  is preserved at the bridge and that the bridge adds no clipping, conversion,
  blending, filtering, or PCM packing.
- [x] A4) [P1, P5] Focused tests prove the production null adapter synchronously
  accepts valid blocks, discards them, increments accepted-block and
  accepted-frame counters only, and retains neither caller pointers nor frame
  values. The test-only recording sink captures exact values independently.
- [x] A5) [P3] Focused live-profile tests prove exactly 44.1 kHz with `-b 1` and
  `-b 2` is accepted, both settings submit raw unblended lanes, and every other
  `-b`, `-8`, `-w`, and non-44.1 kHz setting is rejected before legacy live
  initialization, playback, or submission.
- [x] A6) [P4] A focused finite-live integration test proves the compatible route
  does not open, pause, invoke callbacks, use the ring queue or throttle, drain,
  or tear down SDL, safely completes required legacy cleanup, and returns
  finitely and deterministically.
- [x] A7) [P3, P4] Focused and compatibility evidence proves `-o` remains on its
  current setup, conversion, ring, and file-output path, is exempt from the
  strict live profile, and does not use either temporary live sink.
- [x] A8) [P5] Build/link and test evidence proves direct private source
  composition only: no new library, public API, public header, public target
  `Mixer`, serialized format, device adapter, shell interface, network path, or
  unrestricted filesystem interface is introduced.
- [x] A9) [P1, P2, P3, P4] C23 configure/build and the full CTest suite pass,
  including component, application/composition, finite-live, strict-profile,
  null-counter/no-retention, lane/order/clipping, and `-o` evidence. The Track
  records the compatibility impact and exact changed-file inventory.
- [x] A10) [P5] At S8, README, ARCHITECTURE, ASR, ADR, GLOSSARY, and ARTIFACTS
  are each audited and updated or given an individual unchanged rationale based
  on the implementation. The documentation gate is satisfied before final
  validation and completion.

Why now / impact
- ADR-008 and ASR-009 establish the accepted mixed-value boundary, while the
  legacy path still has no private live route that exercises it. The smallest
  useful Phase 4 step is therefore a live-only bridge at the existing `mixem`
  boundary, with a device-free production sink and an exact-value test seam.
- The route intentionally makes compatible temporary live playback silent and
  bypasses SDL. This is a bounded development behavior that proves routing and
  liveness before any real device adapter or future `Mixer` extraction.
- The bridge preserves the renderer's sequencing, lane order, and multimode
  in-mix clipping, while making the strict live profile and output lifecycle
  change explicit and testable. The legacy `-o` path remains the comparison
  baseline.

Scope
- In scope:
  - The private live-only `src/audio.c` branch immediately after `mixem`.
  - Interleaved signed-32 `{ left, right }` frame blocks with no serialized PCM
    or device-native representation.
  - Exact `tbuf[HALFBUFSIZE+i]` to left and `tbuf[i]` to right mapping, existing
    ordering, and existing multimode in-mix clipping without added clipping.
  - A synchronous production null adapter that counts accepted blocks and
    frames only and retains no pointers or values.
  - A separate test-only recording sink that captures exact frame values.
  - Strict temporary-live validation for exactly 44.1 kHz and `-b 1` or `-b 2`,
    including pre-initialization rejection of all other `-b`, `-8`, `-w`, and
    non-44.1 kHz configurations.
  - Bypassing the specified SDL live lifecycle and ring/throttle/drain path,
    while retaining required legacy cleanup and finite return.
  - Evidence that `-o` retains its current setup, conversion, ring, and file
    output behavior and does not use the temporary live sinks.
  - Focused component and application/integration tests, direct private source
    composition, CMake changes required by the resolved private target lists,
    TDD evidence, compatibility evidence, and Track updates.
  - S2 resolution of the six open questions and the complete six-dimension
    impact decision before implementation.
- Out of scope:
  - A target `Mixer` extraction, Mixer implementation, or claim that the
    temporary bridge is the future Mixer's internal processing boundary.
  - Legacy live blend, filter, or PCM packing in the temporary route; those are
    intentionally omitted rather than relocated to Audio Output.
  - Any real SDL, CoreAudio, or other device adapter, audible temporary-live
    output, format negotiation, or destination conversion.
  - Changes to the legacy `-o` setup, conversion, ring, file format, or writer.
  - Buffering, queueing, timing, backpressure, scheduling, batching policy,
    final-partial policy, lane-clearing policy, or general ownership policy
    beyond the synchronous call and the required no-retention null behavior.
  - Loader, module format, tracker, model, GUI, editor, DAW persistence, shell,
    network, unrestricted filesystem, public API/ABI, or public package work.

Milestones
- [x] M1) Track is ACTIVE with the renamed path, synchronized title/status, and
  S1 checked.
- [x] M2) S2 resolves the private validation/result API, private component
  naming, batching/final-partial/lane-clearing/failure behavior, no-SDL cleanup
  ordering, test seam mechanics, CMake target lists, and the six-dimension
  impact decision.
- [x] M3) TDD implementation proves the signed-32 block, exact lane/order
  mapping, existing multimode clipping, null counters/no retention, and strict
  profile behavior.
- [x] M4) Compatible finite live playback bypasses the specified SDL lifecycle,
  returns finitely, and `-o` evidence remains unchanged.
- [x] M5) C23/full CTest validation, compatibility evidence, and the S8
  documentation gate are complete before review and completion.

Risks / decisions
- Risk: A route after legacy conversion or a newly introduced clip could hide
  lane/order regressions or change renderer output. Decision: bridge immediately
  after `mixem`, map the two `tbuf` lanes exactly, preserve existing multimode
  in-mix clipping/order, and add no clipping.
- Risk: Naming the bridge as a Mixer could prematurely fix an internal target
  boundary. Decision: this is private legacy live routing only; a future Mixer
  outputs the same blocks while its internal processing remains open, and no
  Mixer extraction is claimed.
- Risk: A null adapter that records values or caller addresses could silently
  establish an ownership contract. Decision: production accepts/discards
  synchronously, counts accepted blocks/frames only, and retains no pointers or
  values; exact values belong only to the separate test-only recording sink.
- Risk: Fixed-format assumptions from earlier work could re-enter the route.
  Decision: ADR-008 and ASR-009 govern; Track 012 and ADR-007 are historical
  only, and the block is never serialized PCM or device-native data.
- Risk: Unsupported live settings could initialize legacy state before failing.
  Decision: validate the exact temporary profile before any live initialization,
  playback, or submission: 44.1 kHz plus `-b 1` or `-b 2` only; reject every
  other `-b`, `-8`, `-w`, and non-44.1 kHz setting.
- Risk: Removing SDL delivery can leave resources or termination coupled to the
  old callback/ring path. Decision: bypass device open, pause, callback, ring
  queue, throttle, final drain, and SDL teardown, while resolving and testing
  the exact required legacy cleanup order at S2.
- Risk: Synchronous submission could be mistaken for a queue, timing, or
  ownership policy. Decision: all buffering, queueing, timing, backpressure, and
  general ownership policy beyond call completion remains deferred.
- Decision: No new library or public API is authorized; the executable and
  relevant tests directly compose private sources.

Version impact
- C API/ABI: Unchanged. The bridge, block, sinks, validation, counters, and
  test seam are private; no public declaration, library, or ABI is added.
- Module compatibility/extension: Unchanged. No TFMX module bytes, loader,
  format detection, module extension, or persistent module layout changes.
- Interpreter/timing/audio: Intentionally changed only for the strict temporary
  live route. Sequencing, mixing, `mixem` ordering, existing multimode in-mix
  clipping, and `-o` remain unchanged; compatible temporary live delivery moves
  from SDL delivery to synchronous null submission, and unsupported profiles
  fail before playback.
- Persistent DAW: Unchanged. Blocks are in-memory private values only; no
  serialized DAW representation or version field changes.
- Platform/output: Intentionally changed for the temporary live route. SDL live
  delivery is replaced by null submission with no device lifecycle; no real
  platform adapter is implemented. `-o` retains its existing output path.
- Component/package: Changed privately. Relevant executable/test composition
  gains a private route and sinks, but no public package and no target Mixer are
  created.

S2 resolutions
- [x] Q1) Revise the private block in place to carry a frame count and borrowed
  interleaved signed-32 left/right frames, with no payload-byte length or
  device/PCM representation. Submission has only private accepted/rejected
  results: zero-frame blocks are accepted; nonzero blocks require frames.
- [x] Q2) Revise `src/audio_output/` in place. Its existing private names may
  remain where they accurately describe the mixed-value contract; no successor
  component, public header, library, or public API is introduced.
- [x] Q3) Submit exactly one synchronous block after each `mixem` call; no
  accumulation or flush state is added, so the final partial call is submitted
  at its actual frame count. After an accepted submission, clear both source
  lanes for that range exactly as legacy conversion did. Rejection is a fatal
  invariant failure; the route must stop rather than continue or grow an
  application error-propagation path. Copying the legacy planar signed-32 L/R
  lanes into ordered interleaved `audio_frame` values is required block
  assembly, not PCM/device packing, conversion, blending, filtering, or added
  clipping. The bridge uses one private reusable synchronous staging workspace
  with capacity for 65,536 `audio_frame` values (512 KiB for signed-32 L/R
  pairs), matching the current maximum renderer mix block. It is overwritten
  per submission, is not a queue or ownership policy, and an attempted larger
  submission is a fatal invariant failure. This does not expand deferred
  buffering, timing, backpressure, or ownership policy, or the public/product
  boundaries.
- [x] Q4) After strict-profile validation and normal blend/stereo normalization,
  temporary live initializes the required legacy renderer values and required
  mutex/condition objects without opening SDL: `multiplier` is 4 and
  `blocksize` is 32768. It renders with zero lock, wait, unlock, or signal
  operations; it then destroys the existing synchronization primitives in the
  required order before `TfmxTakedown` frees `smplbuf`. `TfmxTakedown` performs
  no SDL close or quit for temporary live.
- [x] Q5) The bridge calls one private submit symbol. Production targets compose
  the null-adapter translation unit; routing-test targets compose a test-only
  recording-sink translation unit implementing that same symbol. The recording
  sink owns its single-global, non-reentrant capture state and test-only access
  helpers; it is never linked into production. `test_audio_output` also compiles
  a private inspection probe only under a target-specific test definition. The
  probe exposes a mechanically derived snapshot of canonical persistent
  null-adapter state for automated no-retention evidence; it is not a public API
  or production behavior. The routing test duplicates the established
  `HALFBUFSIZE` numeric value as a test-local fixture constant; it adds no new
  production test seam.
- [x] Q6) Use direct source composition only. `SynthTracker` and
  `test_application` add `src/audio_output/audio_output.c` and its private
  include path. `test_audio_output` continues to compose that source for
  component evidence and compiles the private inspection probe only under its
  target-specific test definition. Add `test_audio_routing`, composed of
  `tests/audio_output/test_audio_routing.c`,
  `tests/audio_output/recording_sink.c`, `src/audio.c`, `src/player.c`, and
  `src/tfmx.c`, with the `src`, `src/audio_output`, and `tests/audio_output`
  private include paths plus CMocka and SDL linkage. It intentionally excludes
  `src/audio_output/audio_output.c` so the test-only recording sink is the sole
  bridge submit implementation.

Decision log
- Decision (status): Track TRACK_013 is COMPLETED on
  `stage/04-03-audio-output-extraction`; S1-S9 are checked, the Track moved to
  `.backlog/COMPLETED/2026/`, and commit/push/roadmap/changelog recording
  remain pending.
- Decision (normative basis): ADR-008 and ASR-009 govern the Audio Frame Block;
  `docs/ARTIFACTS.md`, the ADR index, and the Glossary are aligned references.
  Track 012 and ADR-007 are historical records of the superseded fixed-format
  design and are not normative for this Track.
- Decision (bridge): The temporary legacy live bridge is a small live-only
  `src/audio.c` branch immediately after `mixem`. `tbuf[HALFBUFSIZE+i]` maps
  left and `tbuf[i]` maps right. Existing multimode in-mix clipping and order
  are preserved, with no added clipping.
- Decision (block): A block is private interleaved signed-32 `{ left, right }`
  frames, never serialized PCM and never device-native.
- Decision (sinks): The production null adapter accepts/discards synchronously
  and counts accepted blocks/frames only, retaining no pointers or values. A
  separate test-only recording sink captures exact values.
- Decision (composition): No new library or public API is created. The
  executable and relevant tests directly compose private sources.
- Decision (strict live profile): Temporary live accepts exactly 44.1 kHz and
  `-b 1` or `-b 2`; it rejects all other `-b`, `-8`, and `-w`, plus every other
  rate, before legacy live initialization, playback, or submission. For
  `toOutFile == 0`, positional argument validation comes first; invalid profile
  settings are then rejected before `load_tfmx` and before any lifecycle,
  playback, or submission. Rejection returns nonzero without committing a
  message or exact code. No explicit `-b` retains the existing default mode 1.
  Repeated `-b` and `-f` options retain existing last-option-wins parsing. Raw
  blend is validated before normalization, and both allowed raw blend settings
  submit raw unblended lanes.
- Decision (S5 strict-profile parser/testing details): The `-w 0` case requires
  parser-local option-seen tracking so an explicitly supplied zero remains
  distinguishable from an omitted width. The S5 application evidence is a
  fake-composed application test target using link-time doubles for
  `load_tfmx`, lifecycle, playback, and submission observation; it is test-only
  evidence and does not claim implementation or alter the production
  composition. The accepted `-b 2` routing case records raw lanes after
  normalization as `blend=0, stereo=1`.
- Decision (file output): `-o` is exempt and retains full current legacy setup,
  conversion, ring, and file output; it does not use the temporary live sinks.
- Decision (private validation): Revise `src/audio_output/` in place. Its block
  has a frame count and borrowed interleaved signed-32 left/right frames, with
  no byte-length field; submission returns private accepted/rejected results.
  Empty blocks are accepted and nonempty blocks require frames. The null adapter
  counts accepted blocks and frames only.
- Decision (synchronous bridge): Submit one exact-size block after each `mixem`
  call. No accumulation, buffering, or flush state is introduced; a final
  partial call is an ordinary submission. Clear both lanes only after accepted
  submission. A rejected bridge submission is a fatal invariant failure.
  Copying the legacy planar signed-32 L/R lanes into ordered interleaved
  `audio_frame` values is required block assembly, not PCM/device packing,
  conversion, blending, filtering, or added clipping. The bridge uses one
  private reusable synchronous staging workspace with capacity for 65,536
  `audio_frame` values (512 KiB for signed-32 L/R pairs), matching the current
  maximum renderer mix block. It is overwritten per submission, is not a queue
  or ownership policy, and an attempted larger submission is a fatal invariant
  failure. This does not expand deferred buffering, timing, backpressure, or
  ownership policy, or the public/product boundaries.
- Decision (lifecycle): After strict-profile validation and normal
  blend/stereo normalization, temporary live assigns `multiplier = 4` and
  `blocksize = 32768` without opening SDL. It retains required mutex/condition
  initialization and ordered destruction, but performs zero lock, wait, unlock,
  or signal operations. It omits legacy blend/filter/PCM packing and bypasses
  device open, pause, callback, ring queue, throttle, final output, drain, and
  SDL teardown. The finite order is render, destroy the existing synchronization
  primitives, then let `TfmxTakedown` free `smplbuf`; temporary live calls
  neither `SDL_CloseAudio` nor `SDL_Quit`.
- Decision (recording seam): The bridge uses one private submit symbol. The
  production null adapter and test-only recording sink are alternate per-target
  implementations; the recording sink has test-owned single-global,
  non-reentrant capture helpers and is not linked into production. `test_audio_output`
  additionally compiles a private inspection probe only under a target-specific
  test definition; the probe exposes a mechanically derived snapshot of canonical
  persistent null-adapter state for automated no-retention evidence.
  The routing test duplicates the established `HALFBUFSIZE` numeric value as a
  test-local fixture constant and adds no new production test seam.
- Decision (CMake composition): Add `src/audio_output/audio_output.c` directly
  to `SynthTracker` and `test_application`, retaining it in
  `test_audio_output`, where the private inspection probe is compiled only under
  a target-specific test definition. Add direct `test_audio_routing` composition from its
  test, recording-sink, `audio.c`, `player.c`, and `tfmx.c` sources; it omits
  the production adapter. Add only the required private include paths and
  existing CMocka/SDL linkage; create no library or public target.
- Decision (future boundary): A future Mixer outputs the same blocks, but its
  internal processing remains open. This Track neither extracts nor claims a
  Mixer.
- Decision (deferred policies): Buffering, queueing, timing, backpressure, and
  general ownership policy remain deferred beyond synchronous-call completion.
- Decision (six-dimension impact, reaffirmed at S2): C API/ABI is unchanged
  because the block, sink, counters, and seam remain private. Module
  compatibility/extension is unchanged because no TFMX bytes, loader behavior,
  or extension changes. Interpreter/timing/audio is intentionally changed only
  for strict temporary live output: sequencing, `mixem` order, in-mix clipping,
  and `-o` stay unchanged while compatible live delivery becomes synchronous
  null submission. Persistent DAW format/versioning is unchanged because blocks
  are private in-memory values. Platform/audio-output is intentionally changed
  from SDL live delivery to no-device null submission. Component/package
  boundaries change privately through direct source composition only; no public
  package, library, or target Mixer exists. The `test_audio_output` inspection
  probe is a target-specific test-only definition exposing only a mechanically
  derived snapshot for automated no-retention evidence; it creates no public
  API/ABI, library, production runtime behavior, TFMX/module/persistent-format
  change, platform/device behavior, or buffering/ownership policy. The recorded
  six-dimension impact decision remains unchanged for public/product boundaries.
  These S4 clarifications do not change that six-dimension impact assessment:
  C API/ABI, module compatibility/extension, interpreter/timing/audio,
  persistent DAW format/versioning, platform/audio-output, and
  component/package boundaries remain as recorded above.
- Decision (S5/S6 boundary and impact reaffirmation): No S6 SDL, lifecycle,
  cleanup, finite-return, ring, throttle, drain, or `-o` behavior is changed or
  claimed by this S5 decision-recording step. The complete six-dimension
  public/product impact remains unchanged: C API/ABI, module
  compatibility/extension, interpreter/timing/audio, persistent DAW
  format/versioning, platform/audio-output, and component/package boundaries
  retain the assessment recorded above.
- Decision (TDD): S2 resolves Q1-Q6 and records the complete impact decision
  before any implementation TDD chunk. Each later chunk requires a focused red
  test, smallest green implementation, refactor, validation, and Track update.

Plan (execution steps)
- [x] S1) Confirm Track TRACK_013 is ACTIVE with synchronized folder, renamed
  filename, title, branch, status, and the approved scope.
- [x] S2) Re-read this Track, state the next unchecked step, resolve Q1-Q6, and
  record the private validation/result API, revise-in-place versus successor
  naming, batching/final-partial/lane-clearing/failure behavior, exact no-SDL
  cleanup ordering, recording-sink seam mechanics, and CMake target lists.
  Reaffirm the complete six-dimension impact decision before writing code or
  tests, and keep deferred buffering, queueing, timing, backpressure, and
  general ownership policy unresolved.
- [x] S3) TDD chunk: add failing focused component coverage, implement the
  smallest private signed-32 zero/nonzero block and sink behavior, prove null
  accepted-block/accepted-frame counters and no retention, refactor, validate,
  and update this Track.
- [x] S4) TDD chunk: add failing focused routing coverage, implement the
  immediate-after-`mixem` bridge, prove exact lane/order mapping and existing
  multimode clipping through the test-only recording sink, refactor, validate,
  and update this Track.
- [x] S5) TDD chunk: add failing strict-profile coverage, implement exact
  44.1 kHz plus `-b 1`/`-b 2` acceptance and raw unblended submission, reject
  every other `-b`, `-8`, `-w`, and non-44.1 kHz setting before live
  initialization/playback/submission, validate, and update this Track.
- [x] S6) TDD chunk: add failing finite-live and lifecycle coverage, implement
  the no-SDL/no-ring/no-throttle/no-drain route with required cleanup and finite
  return, prove `-o` remains unchanged and isolated, validate compatibility, and
  update this Track.
- [x] S7) Confirm direct private source composition, no library/public API or
  target Mixer, and the complete six-dimension compatibility decision. Record
  exact changed files, target composition, and focused evidence.
- [x] S8) Audit README, ARCHITECTURE, ASR, ADR, GLOSSARY, and ARTIFACTS against
  the implementation. Update each applicable record or record an individual
  unchanged rationale here; preserve ADR-008/ASR-009 authority and historical
  status for Track 012/ADR-007.
- [x] S9) Run the focused validation and full compliant set (`cmake -S . -B
  build -DCMAKE_PREFIX_PATH="$(brew --prefix cmocka)"`, `cmake --build build
  --parallel 2`, `ctest --test-dir build --output-on-failure`), review every
  acceptance criterion and compatibility dimension, inspect `git diff --check`,
  and record final evidence without moving status or changing Git history in
  this Track.

S3 evidence
- Red initially failed while compiling the superseded fixed-PCM contract.
- Green revised `src/audio_output/audio_output.h/.c` and
  `tests/audio_output/test_audio_output.c` for signed-32 frames,
  accepted/rejected semantics, accepted block/frame counters, and bounded
  no-retention evidence.
- The test-only probe red failed to link because the inspector was missing; its
  corrected C23 semantic snapshot red also failed to link.
- The final target-guarded inspector passed
  `cmake --build build --target test_audio_output` and
  `ctest --test-dir build -R '^test_audio_output$' --output-on-failure` (4/4).
- `git diff --check` passed. Independent review approved the target-only guard,
  actual-state probe, shape guards, and bounded evidence.
- Changed files: `CMakeLists.txt`, `src/audio_output/audio_output.h`,
  `src/audio_output/audio_output.c`, `tests/audio_output/test_audio_output.c`,
  and this Track.

S4 evidence
- Initial red routing coverage proved the recording capture was empty before
  the bridge submitted a block.
- Green `src/audio.c` now submits immediately after each `mixem` call through a
  private live-only bridge. The bridge maps
  `tbuf[HALFBUFSIZE+i]` to `left` and `tbuf[i]` to `right`, preserving exact
  `mixem` order. The recording-sink test proves multimode in-mix clipping is
  preserved (`16384`/`-16384` becomes `16383`/`-16383`) and that the bridge
  adds no clipping, conversion, blending, filtering, or PCM packing.
- The route clears both source lanes only after accepted submission. The
  recording sink remains a per-target test-only replacement for the production
  null adapter, so exact values are captured independently of production
  counters.
- Focused routing coverage proves one submission per `mixem` call for full and
  partial blocks (4 then 2 frames), and accepts a single 65,536-frame bridge
  submission through the renderer workspace.
- The focused `test_audio_routing` target built successfully and its focused
  CTest invocation passed. `git diff --check` passed. Independent final review
  approved the S4 routing bridge, recording-sink evidence, lane clearing,
  submission boundaries, and 65,536-frame capacity.
- S4 intentionally does not claim strict live-profile or lifecycle behavior;
  those changes remain deferred to S5 and S6.

S5 decision record
- For `toOutFile == 0`, validate positional arguments first, then reject an
  invalid strict-profile setting before `load_tfmx` and before any lifecycle,
  playback, or submission. The rejection is nonzero and commits neither a
  message nor an exact code. No explicit `-b` keeps the existing default mode
  1; repeated `-b` and `-f` retain last-option-wins parsing. Validate raw blend
  before normalization. Explicit `-w 0` requires parser-local option-seen
  tracking.
- Testing decision: use a fake-composed application test target with link-time
  doubles to directly observe no load, lifecycle, or playback calls for rejected
  live profiles. Because submission is only reachable after playback, zero
  `play_it` calls prove the submission route was not entered. Direct
  submitted-frame evidence remains S4 routing evidence, not S5 application
  evidence. This is test-only evidence, not implementation evidence. Include an
  accepted raw `-b 2` routing case whose normalized state is `blend=0, stereo=1`
  while submitted lanes remain raw and unblended.
- This record does not check S5, M3, or any acceptance criterion and does not
  claim implementation. S6 SDL/lifecycle behavior remains unchanged and
  deferred. The full six-dimension public/product impact remains unchanged.

S6 decision record
- Testing decision: use one direct-composition application/audio lifecycle test
  target. Its target-local doubles provide SDL, pthread, allocation, file-I/O,
  and the private submit symbol; no production test API or production test seam
  is added.
- Lifecycle decision: the temporary live route retains required mutex/condition
  initialization and ordered destruction, but performs zero lock, wait, unlock,
  or signal operations. The intended live behavior remains no SDL, no ring, no
  throttle, and no drain.
- `-o` testing decision: fake file I/O proves the existing conversion, ring,
  write, and close path and proves temporary-sink isolation. The test uses no
  real files and does not assert PCM-byte equivalence.
- Finite-return testing decision: a fake interrupt ends execution after one
  render cycle and asserts one submission plus a deterministic return. This is
  bounded lifecycle evidence and does not define a general timing policy.
- This decision record established the S6 behavior now evidenced below: no SDL,
  no ring, no throttle, and no drain for live playback while `-o` remains
  unchanged. The complete six-dimension public/product impact remains
  unchanged.

S5 evidence
- Red initially failed because invalid live profiles reached the fake lifecycle;
  the focused test therefore demonstrated the pre-guard failure rather than an
  absent test path.
- Green `src/application.c` now validates the strict live profile after
  positional-argument validation and before `load_tfmx`. It validates raw
  `-b 1`, `-b 2`, the omitted default, and last-option-wins behavior for
  repeated `-b`/`-f`; parser-local `-w` tracking rejects explicit `-w 0` as
  well as other `-w` settings. Invalid live profiles return nonzero with no
  load, lifecycle, playback, or submission-route entry; zero `play_it` calls
  provide the submission-route evidence. `-o` is exempt.
- The accepted normalized `-b 2` routing case (`blend=0, stereo=1`) proves
  submitted lanes remain raw and unblended through the separate S4 recording
  sink evidence.
- The fake-composed `test_application_strict_profile` target passed 6/6. Its
  focused build and CTest run passed, as did `git diff --check`. Independent
  review approved S5, A5, and M3.
- `build/Testing/Temporary/LastTestsFailed.log` currently names
  `test_application_audio_lifecycle`, while the latest focused `LastTest.log`
  shows that lifecycle test passing. Both temporary logs are non-authoritative
  until S9 full validation.
- S6 SDL/lifecycle, cleanup, finite-return, ring/throttle/drain, and `-o`
  path-isolation evidence is recorded below. Full validation is not claimed.

S6 evidence
- The valid red harness reached live SDL/ring failures after timing and harness
  correction.
- Green `src/audio.c` configures `multiplier = 4` and `blocksize = 32768`
  without SDL, skips live conversion, ring, throttle, final drain, and SDL
  teardown, and retains ordered synchronization cleanup followed by `free`.
- The hardened `test_application_audio_lifecycle` target proves finite one-submit
  live playback, unchanged ring sentinels, zero SDL calls, zero sync
  lock/wait/unlock/signal activity, zero file activity, and the exact cleanup
  order. The incompatible `-o` case proves file conversion, ring, write, drain,
  and close, with zero temporary submission/SDL activity and close-before-free.
- The focused lifecycle build and CTest run passed, as did `git diff --check`.
  Independent review approved the S6 lifecycle implementation and evidence.

S7 evidence
- Direct private source composition only; no library, public API, public
  header, export, or public target is created.
  - Production `SynthTracker`: `src/main.c`, `src/application.c`, `src/audio.c`,
    `src/audio_output/audio_output.c`, `src/player.c`, `src/tfmx.c` plus SDL
    linkage.
  - `test_application`: `tests/application/test_application.c` with
    `src/application.c`, `src/tfmx.c`, `src/player.c`, `src/audio.c`,
    `src/audio_output/audio_output.c` plus CMocka/SDL linkage.
  - `test_application_strict_profile`: `tests/application/test_application_strict_profile.c`
    with `src/application.c` only; target-local link-time doubles; CMocka only.
  - `test_application_audio_lifecycle`: `tests/application/test_application_audio_lifecycle.c`
    with `src/application.c` and `src/audio.c`; target-local doubles for SDL,
    pthread, allocation, file I/O, and the private submit symbol; CMocka only.
  - `test_audio_output`: `tests/audio_output/test_audio_output.c` with
    `src/audio_output/audio_output.c` plus the target-only inspection probe
    definition; CMocka only.
  - `test_audio_routing`: `tests/audio_output/test_audio_routing.c` and
    `tests/audio_output/recording_sink.c` with `src/audio.c`, `src/player.c`,
    `src/tfmx.c` plus CMocka/SDL linkage; it intentionally excludes the
    production adapter so the recording sink is the sole submit implementation.
- Exact 12-file changed inventory: `CMakeLists.txt`, `src/application.c`,
  `src/audio.c`, `src/audio_output/audio_output.c`,
  `src/audio_output/audio_output.h`, `tests/audio_output/test_audio_output.c`,
  and the new `tests/application/test_application_audio_lifecycle.c`,
  `tests/application/test_application_strict_profile.c`,
  `tests/audio_output/recording_sink.c`, `tests/audio_output/recording_sink.h`,
  and `tests/audio_output/test_audio_routing.c`, plus this Track.
- S7 confirms no library/public target/export, no public header, no target
  `Mixer`, no serialized format, no device adapter, no shell interface, no
  network path, and no unrestricted filesystem interface is introduced by the
  implementation. The pre-existing `tfmx_compile_probe` and
  `player_compile_probe` OBJECT targets are compatibility probes only, not new
  libraries.
- The complete six-dimension impact decision is reaffirmed unchanged: C
  API/ABI unchanged; module compatibility/extension unchanged;
  interpreter/timing/audio intentionally changed only for the strict temporary
  live route; persistent DAW format/versioning unchanged; platform/audio-output
  intentionally changed from SDL live delivery to no-device null submission;
  component/package boundaries changed privately through direct source
  composition only, with no public package, library, or target Mixer.
- S7 confirms composition and inventory only; it does not check A1, A8-A10, M5,
  or S8-S9, and does not claim documentation or full validation. The S8
  documentation audit remains required (see Documentation gate).

S8 evidence
- The S8 documentation gate is checked against the implemented behavior. Each
  audited record was reviewed against the implementation; individual outcomes
  follow.
- README: updated. It now describes the temporary compatible live route as a
  silent, device-free private null submission immediately after the legacy mix,
  documents the strict profile (exactly 44.1 kHz with `-b 1` or `-b 2`, all
  other `-b`, `-8`, `-w`, and rate settings rejected before playback), and
  states that legacy `-o` remains on the full legacy file path and is exempt
  from the strict profile. Stereo blending and low-pass filtering are now
  attributed to the legacy `-o` path and noted as intentionally absent from the
  temporary live route.
- ARCHITECTURE: updated. It now describes the private audio-output live route:
  the private bridge immediately after `mixem` in `src/audio.c` (one block per
  `mixem` call, `tbuf[HALFBUFSIZE+i]` to `left` and `tbuf[i]` to `right`,
  existing multimode in-mix clipping, post-acceptance lane clearing), the
  private `src/audio_output/` null adapter (frame-count plus borrowed
  interleaved signed-32 `{ left, right }` frames, accepted/rejected results,
  accepted block/frame counters, no retention), the strict profile, the
  device-free lifecycle split (no SDL open/pause/callback/ring/throttle/drain/
  teardown, retained mutex/condition init and ordered destruction, finite
  return), the `-o` exemption, and the corrected target composition (direct
  private composition; `SynthTracker` and `test_application` compose the
  production null adapter; other application/audio tests use target-local
  doubles or the test-only recording sink; no library, public API, public
  header, or public target `Mixer`).
- ADR-008: updated while preserving its authority. The renamed Track 013 path
  (`TRACK_013_ACTIVE_legacy_mixed_value_audio_output_routing.md`) replaces the
  stale former PCM-route link, and the pre-rescope wording now records that
  Track 013 was renamed and re-scoped under this decision and implemented the
  private temporary legacy live route, with the former final-PCM packaging
  abandoned. ADR-008 remains the accepted normative mixed-value boundary.
- ASR: unchanged. ASR-009's target requirement (Audio Frame Block as an ordered
  sequence of zero or more interleaved signed-32 `{ left, right }` frames,
  never serialized PCM or device-native data) remains accurate against the
  implemented bridge and null adapter, so no update was warranted.
- ADR index (`docs/ADR.md`): unchanged. ADR-008 is already listed as Accepted
  and ADR-007 as Superseded by ADR-008, so the index status is already correct.
- GLOSSARY: unchanged. No new canonical terminology was added by the
  implementation; Audio Frame Block and related terms were already recorded.
- ARTIFACTS: unchanged. It remains aligned with the implemented mixed-value
  boundary, the private null-adapter composition, and the deferred Mixer
  boundary, so no update was warranted.
- The optional test/legacy documentation audit observed that
  `tests/README.md` and `docs/TESTING.md` under-describe the new tests and
  that `docs/TFMXLegacy/AUDIO.md` is stale but subject to the legacy-doc
  provenance policy; these records were deliberately excluded from the
  approved S8 scope.
- The documentation review is complete and `git diff --check` passed. S8 does
  not claim full validation: S9 remains required for the focused validation and
  full compliant CTest set, the complete acceptance-criterion and
  compatibility review, and final evidence recording.

S9 evidence
- S9a exact validation evidence, run from the repository root in this order,
  all passed: `cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix
  cmocka)"` configured cleanly; `cmake --build build --parallel 2` built all
  targets; `ctest --test-dir build --output-on-failure` passed 9/9 named
  tests (`test_synthtracker_cli_identity`, `tfmx_compile_probe`,
  `player_compile_probe`, `test_playback_context`, `test_application`,
  `test_application_strict_profile`, `test_application_audio_lifecycle`,
  `test_audio_output`, `test_audio_routing`); and `git diff --check` passed.
- S9 reviews every acceptance criterion and compatibility dimension and checks
  A1, A8, A9, and M5. With S1-S8 already checked, every acceptance criterion
  (A1-A10), milestone (M1-M5), and plan step (S1-S9) is now checked.
- Final six-dimension/TFMX compatibility summary: C API/ABI is unchanged
  because the bridge, block, sinks, validation, counters, and test seam are
  private. Module compatibility/extension is unchanged because no TFMX module
  bytes, loader, format detection, module extension, or persistent module
  layout changes. Interpreter/timing/audio is unchanged for sequencing,
  mixing, `mixem` ordering, existing multimode in-mix clipping, and the `-o`
  path; it is intentionally changed only for the strict temporary live route,
  whose synchronous no-device null submission is a temporary-live limitation,
  not a product behavior. Persistent DAW format/versioning is unchanged
  because blocks are private in-memory values with no serialized
  representation. Platform/audio-output is intentionally changed only for the
  temporary live route (SDL live delivery replaced by null submission with no
  device lifecycle); `-o` remains unchanged on its existing setup, conversion,
  ring, and file-output path. Component/package boundaries changed privately
  through direct source composition only; no public package, library, or
  target `Mixer` exists.
- Final changed-file inventory, exactly 15 files: this Track; `CMakeLists.txt`;
  `README.md`; `docs/ARCHITECTURE.md`;
  `docs/adr/ADR-008-audio-frame-block-mixed-value-boundary.md`;
  `src/application.c`; `src/audio.c`; `src/audio_output/audio_output.c`;
  `src/audio_output/audio_output.h`;
  `tests/application/test_application_audio_lifecycle.c`;
  `tests/application/test_application_strict_profile.c`;
  `tests/audio_output/test_audio_output.c`;
  `tests/audio_output/recording_sink.c`; `tests/audio_output/recording_sink.h`;
  and `tests/audio_output/test_audio_routing.c`. The earlier S7 record of 12
  changed files predates the S8 documentation updates (README, ARCHITECTURE,
  and ADR-008), which bring the final inventory to 15.
- The stale `build/Testing/Temporary/LastTestsFailed.log` is non-authoritative
  historical residue from an earlier focused run; the latest full CTest run
  passes all 9/9 named tests.
- S9 records execution evidence only. The Track remains ACTIVE with unchanged
  status, folder, filename, title, and branch; no completion, status move,
  commit, or other Git-history change occurs in this step.

Current inventory
- `src/audio.c`: current legacy renderer plus the private live-only bridge
  immediately after `mixem`. It assembles interleaved signed-32 frames in a
  reusable 65,536-frame workspace, submits through the private sink symbol, and
  clears both source lanes after acceptance. Existing multimode in-mix
  clipping/order and the `-o` setup, conversion, ring, and file-output path
  remain present. Temporary live configures `multiplier = 4` and
  `blocksize = 32768` without SDL, skips live conversion/ring/throttle/final
  drain/SDL teardown, and performs ordered synchronization cleanup before free.
- `src/application.c`: application-level strict temporary-live validation runs
  after positional parsing and before `load_tfmx`, including raw blend,
  explicit-width, rate, default, and repeated-option handling. Invalid live
  profiles stop before load/lifecycle/playback; `-o` remains exempt and retains
  its existing setup, conversion, ring, and file-output behavior.
- `src/audio_output/audio_output.c` and
  `src/audio_output/audio_output.h`: private in-place signed-32 interleaved
  frame-block implementation with accepted/rejected validation and accepted
  block/frame counters. A test-target-only bounded inspection probe exposes
  the adapter's counter snapshot for no-retention evidence.
- `tests/audio_output/test_audio_output.c`: focused component evidence covers
  accepted zero/nonzero blocks, accepted block/frame counters, rejection of
  invalid nonzero blocks, and bounded no-retention through the test-only probe.
  The routing bridge is tested separately by `test_audio_routing`.
- `tests/audio_output/recording_sink.c` and
  `tests/audio_output/recording_sink.h`: test-only recording sink and helpers
  capture exact submitted frames and per-submission frame counts; they are not
  linked into production.
- `tests/audio_output/test_audio_routing.c`: focused routing test for immediate
  post-`mixem` lane/order mapping, multimode clipping, post-accept lane
  clearing, per-target sink composition, full/partial submission boundaries,
  and the 65,536-frame bridge capacity.
- `CMakeLists.txt`: direct private composition now includes the production
  adapter in `SynthTracker` and `test_application`, while `test_audio_routing`
  composes `audio.c`, `player.c`, `tfmx.c`, its routing test, and the test-only
  recording sink without the production adapter or a new library/public target;
  it also registers the fake-composed `test_application_strict_profile` target
  and the direct-composed `test_application_audio_lifecycle` target
  (`tests/application/test_application_audio_lifecycle.c` with
  `src/application.c` and `src/audio.c` and target-local doubles).
- `tests/application/test_application_strict_profile.c`: fake-composed
  application-level strict-profile evidence covers accepted default and
  `-b 1`/`-b 2` profiles, last-option-wins parsing, invalid live-profile
  rejection before load/lifecycle/playback, positional validation precedence,
  and `-o` exemption.
- `tests/application/test_application_audio_lifecycle.c`: hardened direct-
  composition lifecycle evidence covers finite one-submit live playback,
  unchanged ring sentinels, zero SDL/sync-operation/file activity, ordered
  synchronization cleanup before free, and isolated incompatible `-o` file
  conversion/ring/write/drain/close behavior.
- Existing compatibility evidence includes `tests/playback/`,
  `test_playback_context`, `tfmx_compile_probe`, `player_compile_probe`, and
  application/CLI tests. S3/S4 evidence for exact lane/order/clipping and null
  counters/no retention, plus S5 strict-profile evidence, is recorded. S7
  composition/inventory evidence, S8 documentation-gate evidence, and S9
  full-validation evidence are recorded above. All plan steps (S1-S9),
  acceptance criteria (A1-A10), and milestones (M1-M5) are checked, and all
  execution evidence is complete; the Track status is COMPLETED and
  synchronized across the folder, filename, title, and status. Commit, push,
  and roadmap/changelog recording remain pending; no Git-history change has
  occurred.

Artifacts
- Living roadmap: SynthTracker modernization roadmap, Phase 4 component
  extraction, Stage 3 Audio Output extraction. This Track is the private legacy
  mixed-value routing step; no future Mixer extraction is claimed.
- [`docs/adr/ADR-008-audio-frame-block-mixed-value-boundary.md`](../../../docs/adr/ADR-008-audio-frame-block-mixed-value-boundary.md)
  — accepted normative mixed-value boundary.
- [`docs/ASR.md#asr-009--audio-frame-block-boundary-invariants`](../../../docs/ASR.md)
  — normative signed-32 ordered-block invariants.
- [`docs/ARTIFACTS.md`](../../../docs/ARTIFACTS.md) — aligned artifact
  vocabulary and deferred contract questions.
- [`docs/ADR.md`](../../../docs/ADR.md) — aligned ADR index; ADR-007 is
  superseded historical context and ADR-008 is accepted.
- [`docs/GLOSSARY.md`](../../../docs/GLOSSARY.md) — aligned terminology for
  Audio Frame Block, Mixer, and Audio Output.
- [`docs/ARCHITECTURE.md`](../../../docs/ARCHITECTURE.md) — current legacy
  ownership and target-only Mixer/Audio Output flow to audit at S8.
- [`docs/TESTING.md`](../../../docs/TESTING.md),
  [`tests/AGENTS.md`](../../../tests/AGENTS.md), and
  [`docs/AGENT_WORKFLOW.md`](../../../docs/AGENT_WORKFLOW.md) — TDD,
  ownership, compatibility, and validation gates.
- [Track 012](../../COMPLETED/2026/TRACK_012_COMPLETED_audio_output_null_adapter.md)
  — historical private fixed-format/null-adapter work only; not a normative
  contract for this Track.
- [`docs/adr/ADR-007-audio-frame-block-boundary-and-fixed-first-format.md`](../../../docs/adr/ADR-007-audio-frame-block-boundary-and-fixed-first-format.md)
  — superseded historical record only; not a normative requirement here.

Documentation gate
- ADR-008, ASR-009, ARTIFACTS, the ADR index, and the Glossary are already
  architecturally aligned with this Track's accepted mixed-value boundary.
- The S8 documentation gate is complete: README, ARCHITECTURE, ASR, ADR,
  GLOSSARY, and ARTIFACTS were each audited against the implementation and
  updated or given an individual unchanged rationale (see S8 evidence above).
  ADR-008 now records the renamed Track 013 path and completed re-scope while
  preserving its accepted authority; ASR-009, the ADR index, GLOSSARY, and
  ARTIFACTS carry individual unchanged rationales.
- `git diff --check` passed as part of the S8 documentation review. The S8 gate
  records documentation completion only and does not claim full validation;
  S9 full validation remains required, and this Track does not treat the
  documentation gate as evidence that the validation gate is complete.

Completion notes
- All plan, acceptance, and milestone work is complete: S1-S9, A1-A10, and M1-M5
  are all checked, with S9 recording the full-validation evidence (S9a commands
  and 9/9 CTest pass, the final six-dimension/TFMX compatibility summary, the
  exact 15-file changed inventory, and the non-authoritative stale
  `LastTestsFailed.log` note). S9 validation passed and every acceptance
  criterion and compatibility dimension was reviewed.
- This Track's status was moved to COMPLETED: the folder, filename, title, and
  status are synchronized at
  `.backlog/COMPLETED/2026/TRACK_013_COMPLETED_legacy_mixed_value_audio_output_routing.md`,
  and the decision-log status is corrected to S1-S9 checked.
- Commit, push, and roadmap/changelog recording remain pending and not yet
  performed; no Git-history change occurs in this move.
- All execution evidence and technical decisions above are preserved unchanged.
