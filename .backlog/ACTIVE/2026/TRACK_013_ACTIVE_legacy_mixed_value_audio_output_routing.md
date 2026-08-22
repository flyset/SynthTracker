# TRACK TRACK_013 [ACTIVE]: legacy_mixed_value_audio_output_routing

Track
- ID: TRACK_013
- Repository: SynthTracker
- Branch: stage/04-03-audio-output-extraction
- Current path: .backlog/ACTIVE/2026/TRACK_013_ACTIVE_legacy_mixed_value_audio_output_routing.md
- Status: ACTIVE

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
- [ ] A1) [P1, P2] After S2 resolves the private validation/result API, focused
  component tests prove zero-frame and nonzero-frame signed-32 block behavior.
  Zero frames are accepted as an ordered empty block; nonzero blocks preserve
  every signed-32 left/right component exactly under the resolved private
  validation rules, with no serialized or device-native payload.
- [ ] A2) [P1, P2] A focused routing test using the test-only recording sink
  proves exact interleaved frame order and lane mapping: each `left` value comes
  from `tbuf[HALFBUFSIZE+i]`, each `right` value comes from `tbuf[i]`, and the
  sequence follows the existing `mixem` order.
- [ ] A3) [P2] The recording-sink test proves existing multimode in-mix clipping
  is preserved at the bridge and that the bridge adds no clipping, conversion,
  blending, filtering, or PCM packing.
- [ ] A4) [P1, P5] Focused tests prove the production null adapter synchronously
  accepts valid blocks, discards them, increments accepted-block and
  accepted-frame counters only, and retains neither caller pointers nor frame
  values. The test-only recording sink captures exact values independently.
- [ ] A5) [P3] Focused live-profile tests prove exactly 44.1 kHz with `-b 1` and
  `-b 2` is accepted, both settings submit raw unblended lanes, and every other
  `-b`, `-8`, `-w`, and non-44.1 kHz setting is rejected before legacy live
  initialization, playback, or submission.
- [ ] A6) [P4] A focused finite-live integration test proves the compatible route
  does not open, pause, invoke callbacks, use the ring queue or throttle, drain,
  or tear down SDL, safely completes required legacy cleanup, and returns
  finitely and deterministically.
- [ ] A7) [P3, P4] Focused and compatibility evidence proves `-o` remains on its
  current setup, conversion, ring, and file-output path, is exempt from the
  strict live profile, and does not use either temporary live sink.
- [ ] A8) [P5] Build/link and test evidence proves direct private source
  composition only: no new library, public API, public header, public target
  `Mixer`, serialized format, device adapter, shell interface, network path, or
  unrestricted filesystem interface is introduced.
- [ ] A9) [P1, P2, P3, P4] C23 configure/build and the full CTest suite pass,
  including component, application/composition, finite-live, strict-profile,
  null-counter/no-retention, lane/order/clipping, and `-o` evidence. The Track
  records the compatibility impact and exact changed-file inventory.
- [ ] A10) [P5] At S8, README, ARCHITECTURE, ASR, ADR, GLOSSARY, and ARTIFACTS
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
- [ ] M2) S2 resolves the private validation/result API, private component
  naming, batching/final-partial/lane-clearing/failure behavior, no-SDL cleanup
  ordering, test seam mechanics, CMake target lists, and the six-dimension
  impact decision.
- [ ] M3) TDD implementation proves the signed-32 block, exact lane/order
  mapping, existing multimode clipping, null counters/no retention, and strict
  profile behavior.
- [ ] M4) Compatible finite live playback bypasses the specified SDL lifecycle,
  returns finitely, and `-o` evidence remains unchanged.
- [ ] M5) C23/full CTest validation, compatibility evidence, and the S8
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

Open questions
- [ ] Q1) Which private validation and result API should validate the signed-32
  zero/nonzero blocks and report accepted/rejected submission without creating a
  public contract?
- [ ] Q2) Should the private implementation revise the existing component in
  place or use a privately named successor component, given the historical
  fixed-format implementation?
- [ ] Q3) What are the required block batching, final-partial-block, lane
  clearing, and failure behaviors for this synchronous temporary route?
- [ ] Q4) What exact cleanup ordering safely completes required legacy cleanup
  after the route bypasses SDL device open, pause, callbacks, ring queue,
  throttle, final drain, and SDL teardown?
- [ ] Q5) What private test seam mechanics allow the recording sink to capture
  exact values while the production null adapter retains neither pointers nor
  values?
- [ ] Q6) Which exact CMake source and target lists compose the private route
  into `SynthTracker` and the relevant component/application tests without
  creating a library or public target?

Decision log
- Decision (status): Track TRACK_013 remains ACTIVE on
  `stage/04-03-audio-output-extraction`; S1 is checked and S2-S9 remain
  unchecked.
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
  rate, before legacy live initialization, playback, or submission. Both
  allowed blend settings submit raw unblended lanes.
- Decision (file output): `-o` is exempt and retains full current legacy setup,
  conversion, ring, and file output; it does not use the temporary live sinks.
- Decision (lifecycle): Temporary live omits legacy blend/filter/PCM packing and
  bypasses SDL device open, pause, callback, ring queue, throttle, final drain,
  and SDL teardown while safely completing needed legacy cleanup. The exact
  cleanup ordering is Q4 and must be resolved at S2.
- Decision (future boundary): A future Mixer outputs the same blocks, but its
  internal processing remains open. This Track neither extracts nor claims a
  Mixer.
- Decision (deferred policies): Buffering, queueing, timing, backpressure, and
  general ownership policy remain deferred beyond synchronous-call completion.
- Decision (six-dimension impact): The full required impact decision is recorded
  now and must be reaffirmed at S2 before implementation: C API/ABI unchanged
  private; module compatibility unchanged; interpreter/timing/audio
  intentionally changed only for the strict temporary live route while
  sequencing/mixing and `-o` remain unchanged; persistent DAW unchanged;
  platform/output intentionally changed from SDL live delivery to null
  submission with no device; component/package privately changed with no public
  or target Mixer.
- Decision (TDD): S2 must resolve Q1-Q6 and record the complete impact decision
  before any implementation TDD chunk. Each later chunk requires a focused red
  test, smallest green implementation, refactor, validation, and Track update.

Plan (execution steps)
- [x] S1) Confirm Track TRACK_013 is ACTIVE with synchronized folder, renamed
  filename, title, branch, status, and the approved scope.
- [ ] S2) Re-read this Track, state the next unchecked step, resolve Q1-Q6, and
  record the private validation/result API, revise-in-place versus successor
  naming, batching/final-partial/lane-clearing/failure behavior, exact no-SDL
  cleanup ordering, recording-sink seam mechanics, and CMake target lists.
  Reaffirm the complete six-dimension impact decision before writing code or
  tests, and keep deferred buffering, queueing, timing, backpressure, and
  general ownership policy unresolved.
- [ ] S3) TDD chunk: add failing focused component coverage, implement the
  smallest private signed-32 zero/nonzero block and sink behavior, prove null
  accepted-block/accepted-frame counters and no retention, refactor, validate,
  and update this Track.
- [ ] S4) TDD chunk: add failing focused routing coverage, implement the
  immediate-after-`mixem` bridge, prove exact lane/order mapping and existing
  multimode clipping through the test-only recording sink, refactor, validate,
  and update this Track.
- [ ] S5) TDD chunk: add failing strict-profile coverage, implement exact
  44.1 kHz plus `-b 1`/`-b 2` acceptance and raw unblended submission, reject
  every other `-b`, `-8`, `-w`, and non-44.1 kHz setting before live
  initialization/playback/submission, validate, and update this Track.
- [ ] S6) TDD chunk: add failing finite-live and lifecycle coverage, implement
  the no-SDL/no-ring/no-throttle/no-drain route with required cleanup and finite
  return, prove `-o` remains unchanged and isolated, validate compatibility, and
  update this Track.
- [ ] S7) Confirm direct private source composition, no library/public API or
  target Mixer, and the complete six-dimension compatibility decision. Record
  exact changed files, target composition, and focused evidence.
- [ ] S8) Audit README, ARCHITECTURE, ASR, ADR, GLOSSARY, and ARTIFACTS against
  the implementation. Update each applicable record or record an individual
  unchanged rationale here; preserve ADR-008/ASR-009 authority and historical
  status for Track 012/ADR-007.
- [ ] S9) Run the focused validation and full compliant set (`cmake -S . -B
  build -DCMAKE_PREFIX_PATH="$(brew --prefix cmocka)"`, `cmake --build build
  --parallel 2`, `ctest --test-dir build --output-on-failure`), review every
  acceptance criterion and compatibility dimension, inspect `git diff --check`,
  and record final evidence without moving status or changing Git history in
  this Track.

Current inventory
- `src/audio.c`: current legacy renderer, including `mixem`, legacy lane data,
  multimode in-mix clipping/order, live SDL setup/delivery, and `-o` setup,
  conversion, ring, and file output. The requested private bridge does not yet
  exist.
- `src/audio_output/audio_output.c` and
  `src/audio_output/audio_output.h`: existing private implementation from the
  historical fixed-format effort. Its concrete validation and naming are not
  assumed to be the accepted mixed-value contract; S2 decides revise-in-place
  versus a private successor.
- `tests/audio_output/test_audio_output.c`: existing historical component
  evidence for the former implementation. A separate recording-sink seam and
  mixed-value tests are required under this Track after S2.
- `CMakeLists.txt`: existing executable and test composition. Exact private
  source/target lists for this route remain Q6 and must be recorded at S2.
- Existing compatibility evidence includes `tests/playback/`,
  `test_playback_context`, `tfmx_compile_probe`, `player_compile_probe`, and
  application/CLI tests. New evidence must add the temporary live strict
  profile, exact lane/order/clipping, null counters/no retention, finite return,
  no SDL lifecycle/ring/drain, and unchanged `-o` behavior.

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
- At S8, README, ARCHITECTURE, ASR, ADR, GLOSSARY, and ARTIFACTS must each be
  audited and either updated for the implementation or given an individual
  unchanged rationale based on the implementation. This Track does not treat
  the aligned baseline as evidence that the implementation gate is complete.

Completion notes
- Not yet complete. S1 is checked; S2-S9 and A1-A10 remain unchecked. No
  implementation, test execution, documentation-gate completion, status move,
  commit, or push is recorded by this rewrite.
