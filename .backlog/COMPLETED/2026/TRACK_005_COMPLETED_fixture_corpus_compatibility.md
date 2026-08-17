# TRACK 005 [COMPLETED]: fixture_corpus_compatibility

Track
- ID: TRACK_005
- Repository: TFMX.cpp
- Branch: main
- Current path: .backlog/COMPLETED/2026/TRACK_005_COMPLETED_fixture_corpus_compatibility.md

Problems (PORE)
- P1: As a compatibility maintainer, I cannot demonstrate preservation of loop, effect/timing, or multi-voice behavior, because the existing self-authored fixture exercises only one short voice-0 path.
- P2: As a loader maintainer, I cannot review whether malformed modules are rejected consistently across structural and semantic failures, because the current malformed checks are derived from mutations of one narrow fixture.

Objective
- Establish a self-authored fixture corpus with focused automated compatibility evidence for finite loops, selected effects and tempo behavior, multiple voices, and malformed module rejection at the existing private playback boundary.

Non-negotiables
- This ACTIVE Track authorizes work only through its checked plan steps; no implementation, test, fixture, configuration, or production-source change is authorized outside the declared scope and gates.
- All implementation follows TDD: a focused failing test, the smallest passing implementation, then refactoring and validation.
- Preserve the compatibility floor: existing TFMX modules must load and play correctly; bit-identical playback is not required.
- Keep all TFMX-owned production and test source in C23; no C++ port is planned.
- Each fixture must be self-authored, checked in, and accompanied by a human-readable layout specification. Do not use copied legacy-module data or user-local media.
- Before creating or quoting legacy-format layout content, read and follow `docs/TFMXLegacy/PROVENANCE.md`.
- Before each ACTIVE TDD chunk, record the bounded behavior to test, its local-code evidence, and the self-authored fixture intent. Exact tick traces, PCM values, and completion positions are TDD evidence: record them only after the focused test establishes them. These contracts describe this repository's legacy implementation only; they do not claim a universal TFMX specification.
- Evidence must use the private load, start, tick, snapshot, render, and completion boundary. A bounded private multi-voice snapshot extension is authorized only through its declared ACTIVE-Track TDD chunk; it must not add a public C API, public MCP contract, SDL/device execution, or `main()` execution.
- Platform validation remains limited to the macOS/Clang boundary unless a separate roadmap decision expands it.

Acceptance criteria
- [x] A1) [P1] Focused automated evidence loads and plays a self-authored pattern-level finite-loop fixture, proving the selected finite-loop behavior and eventual completion through the private boundary.
- [x] A2) [P1] Focused automated evidence loads and plays a self-authored envelope/tempo fixture, proving that the selected effect advances on engine ticks while `tempo[0] = 2` prescales pattern processing through the private boundary.
- [x] A3) [P1] Focused automated coverage proves the bounded private multi-voice snapshot extension returns one same-tick fixed eight-voice value snapshot, with documented independent `active`, `pitch`, and `volume` state for voices 0 and 1, without changing the public API.
- [x] A4) [P1] Focused automated evidence loads and plays a self-authored voices-0-and-1 fixture, proving independent voice behavior and current mixer routing through the bounded private multi-voice snapshot extension.
- [x] A5) [P2] Focused automated evidence rejects each documented self-authored malformed-fixture pair as `TFMX_LOAD_INVALID_FORMAT` and preserves a previously loaded valid module after every failed replacement load.
- [x] A6) [P1, P2] Each accepted fixture has a reviewed layout specification that identifies its intent, permitted loader assumptions, and TDD-established observable evidence without claiming broad format coverage.
- [x] A7) [P1, P2] The Track records focused and full validation evidence, compatibility mismatches, deferred safeguards, and the required `docs/ARCHITECTURE.md` update; `README.md` and `docs/GLOSSARY.md` remain unchanged only if their documented public boundaries remain unaffected.

Why now / impact
- Track 002 established one reproducible minimal playback path but explicitly deferred fixture coverage for loops, effects, multiple voices, tempo, and malformed modules. A bounded fixture corpus supplies compatibility evidence that supports later evaluation of broader loader work or audio fingerprints without establishing an ordering dependency.

Scope
- In scope:
  - Define bounded behavioral hypotheses for self-authored finite-loop, effects/tempo, and multiple-voice fixtures from the legacy implementation and private playback boundary; establish exact traces only through ACTIVE TDD.
  - Add a bounded private multi-voice snapshot extension that returns a caller-owned fixed eight-voice value snapshot, solely to provide same-tick fixture observations.
  - Adjust only the current narrow loader/fixture validation needed to accept and validate the documented loop, envelope/tempo, and active voice-0/voice-1 fixture contracts, plus their selected malformed counterparts.
  - Add the corresponding checked-in binary fixtures and human-readable layout specifications.
  - Add focused automated loading, tick/snapshot, rendering where meaningful, completion, rejection, and transactional-loading evidence.
  - Add a self-authored malformed-fixture corpus covering the selected structural and semantic rejection cases.
  - Update `docs/ARCHITECTURE.md` in the implementation change to record the expanded self-authored fixture corpus and bounded private all-voice snapshot seam, without representing either as a general loader or public playback API.
  - Record observed compatibility mismatches rather than weakening assertions silently.
- Out of scope:
  - Full legacy-module compatibility coverage or a complete format specification.
  - Broader loader representation/ownership design, acceptance of arbitrary legacy-module variants, or Loader/Writer → Module Domain Model → Playback Engine contract decisions.
  - Audio reference recordings, fingerprints, bit-perfect comparison, or a new render-tolerance policy.
  - Multi-context or reentrant playback, SDL/device behavior, `main()` execution, GUI, editing, persistence, CoreAudio, or platform expansion.
  - Public C API, public MCP contract, endpoint, or version changes.

Milestones
- [x] M1) Define and review each fixture's bounded behavior, local-code evidence, layout approach, and selected malformed cases before activation; do not claim unexecuted traces as acceptance evidence.
- [x] M2) With activation complete, execute the finite-loop fixture coherent TDD chunk.
- [x] M3) Execute the effects/tempo fixture coherent TDD chunk.
- [x] M4) Execute the bounded private multi-voice snapshot-extension coherent TDD chunk.
- [x] M5) Execute the multiple-voice fixture coherent TDD chunk using that extension.
- [x] M6) Execute the malformed-fixture corpus coherent TDD chunk and validate the complete corpus.

Risks / decisions
- Risk: A self-authored corpus can demonstrate only its documented subset of TFMX semantics and must not be represented as comprehensive legacy-module compatibility.
- Risk: Fixture expectations may accidentally encode a bridge artifact instead of a legacy semantic; expectations require legacy-source and current-boundary review before implementation.
- Risk: A multi-voice snapshot could become an accidental general observability API; its selected voices, fields, lifecycle, and private status must be bounded before implementation.
- Risk: Relaxing the current loader's inactive-voice validation could accidentally broaden module acceptance beyond the self-authored fixture contract; loader changes must remain limited to the documented voice-0/voice-1 bindings and their structural/semantic validation.
- Risk: The private renderer currently uses fixed eClocks, so this Track's tempo evidence must remain tick/prescale evidence; render-frame timing or an eClock handoff requires separate scope.
- Risk: A detected defect may require changing production behavior, expanding the intended component boundary, or resolving an open loader/domain contract. Such work must be explicitly re-scoped before it begins.
- Decision: The corpus covers four bounded categories: finite loops, selected effects and tempo behavior, multiple voices, and malformed inputs.
- Decision: The existing private playback boundary is the evidence mechanism; no public-contract version impact is introduced because no public contract change is in scope.
- Decision: A bounded private multi-voice snapshot extension is in scope exclusively to prove the selected fixture's independent voice behavior. It returns all eight voices in one caller-owned value array, indexed 0–7, with only `active`, `pitch`, and `volume`; it is not a public or general-purpose observability API.
- Decision: The multiple-voice fixture selects voices 0 and 1, which the legacy player steps without multimode. The loader change is limited to validating this fixture's documented active voice-0/voice-1 bindings; it does not authorize broader module acceptance.
- Decision: The finite-loop fixture uses the pattern-level `0xF1` loop command rather than a trackstep `EFFE` loop, because it avoids changing bridge-owned global loop state. Its exact layout, trace, rendering, and completion evidence are established by the ACTIVE finite-loop TDD chunk.
- Decision: The effects/tempo fixture uses the supported envelope behavior and `tempo[0] = 2`. It tests the distinction between per-engine-tick effects and prescaled pattern processing. Its exact trace, rendering, and completion evidence are established by the ACTIVE effects/tempo TDD chunk; dynamic eClock behavior remains out of scope.
- Decision: The malformed corpus will add ten distinct self-authored pairs: truncated MDAT, unaligned track pointer, out-of-range pattern pointer, invalid active binding, invalid inactive binding, invalid stop step, invalid pattern contract, invalid macro ordering, sample-range overflow, and silent sample payload. Each is expected to return `TFMX_LOAD_INVALID_FORMAT`; no new loader error status is introduced.
- Decision: The loader work is a finite-fixture recognizer, not a general loader expansion. It preserves the existing `mdat.step8`/`smpl.step8` contract and adds only the three named self-authored valid layouts for the finite loop, envelope/tempo, and voices-0-and-1 fixtures, plus their ten named malformed counterparts. Acceptance of arbitrary legacy-module layouts, representation redesign, and the open Loader/Writer → Module Domain Model → Playback Engine contract remain out of scope.
- Decision: Audio fingerprinting and multi-context playback remain separate future safeguards, because they require policy or ownership decisions beyond this corpus.

Open questions
- [x] Q1) Resolved: use the supported envelope behavior and `tempo[0] = 2`; ACTIVE TDD establishes the exact trace and completion without render-frame timing assertions.
- [x] Q2) Resolved: use the pattern-level `0xF1` finite loop; ACTIVE TDD establishes its exact trace and completion.
- [x] Q3) Resolved: use the ten documented structural and semantic malformed pairs, each returning `TFMX_LOAD_INVALID_FORMAT` and preserving a valid loaded module after failed replacement.
- [x] Q4) Resolved: the current voice-0-only snapshot is insufficient. A bounded private multi-voice snapshot extension is required before the multiple-voice fixture TDD chunk.

Decision log
- Decision (Q1): User approved the supported envelope behavior with `tempo[0] = 2`. ACTIVE TDD establishes the exact envelope, rendering, and completion evidence. It excludes dynamic-eClock assertions because the current private renderer uses fixed eClocks.
- Decision (Q2): User approved the pattern-level `0xF1` finite loop. ACTIVE TDD establishes its exact fixture layout, pitch trace, rendering, and completion evidence. The `0xF1` count interpretation remains repository-implementation evidence, not a universal format claim.
- Decision (Q3): User approved ten self-authored malformed pairs: truncated MDAT; unaligned track pointer; out-of-range pattern pointer; invalid active binding; invalid inactive binding; invalid stop step; invalid pattern contract; invalid macro ordering; sample-range overflow; and silent sample payload. Each must return `TFMX_LOAD_INVALID_FORMAT`; after each rejection, a valid previously loaded module must remain loaded. Distinct fixture identity/layout records the failure class; new error statuses are out of scope.
- Decision (Q4): User approved a bounded private multi-voice snapshot extension. It returns all eight voice records in one same-tick caller-owned value array indexed 0–7. Each record contains only `active`, `pitch`, and `volume`; voice 0 and voice 1 are the selected fixture observations. Null context/output yields invalid argument; an unstarted context yields not started; output is written only on success; the last successful tick supplies the cached snapshot; a start or replacement resets all cached records. The context remains caller-owned and must remain alive for the call; returned values contain no pointers and outlive the call. This remains private and single-global/non-reentrant.
- Decision (Q5): User approved the finite-fixture-recognizer boundary. The loader preserves the existing fixture and recognizes only the three new named valid self-authored layouts and their ten named malformed counterparts. This is evidence scope, not a claim of general legacy-module support.
- Decision (Q6): The behavioral contracts below are approved planning scope. Exact fixture bytes, tick traces, PCM values, and completion positions are not accepted in DRAFT; each declared ACTIVE TDD chunk establishes and records them. A mismatch is compatibility evidence and must be recorded; it must not silently trigger a legacy-semantic change.
- Decision (Q7): Compatibility impact is assessed in the matrix below. The private snapshot declaration is additive and does not change a public C API/ABI, public MCP contract, persistent DAW format, platform scope, audio-output adapter, or component/package boundary. The loader's accepted self-authored fixture set changes only within the finite fixture recognizer. `docs/ARCHITECTURE.md` is updated in the implementation change because its current test-layout inventory must record this new private compatibility evidence. `README.md` and `docs/GLOSSARY.md` are reviewed but remain unchanged only if no documented public boundary changes.
- Decision (S6a): The private clean-start bridge runtime reset is authorized as a corrective TDD chunk. Root-cause evidence established that the reset omitted bridge-owned runtime state and stale `CurPeriod` leaked after previous playback. Public C API/ABI/MCP are unchanged because the reset remains private and changes no declaration, endpoint, or public behavior. Module/file loader semantics are unchanged because no loader path, validation rule, module representation, or file interpretation is changed. Interpreter/tick order is unchanged because the reset only establishes clean bridge-owned runtime state before playback and does not reorder or alter tick processing. Corrective private playback restart isolation is the intended change: the first Step8 tick after a fresh start is independent of prior playback. DAW persistence is unchanged because no persistence state or format is read or written. The platform/audio adapter is unchanged because no SDL/device, renderer, or platform boundary is modified. The bridge boundary remains private, single-global, and non-reentrant because no context ownership or concurrency model is introduced. The authorized four-file scope is `tests/playback/test_playback_context.c`, `src/playback/playback_legacy_bridge.c`, `docs/ARCHITECTURE.md`, and this Track; loader, interpreter, public API, CMake, fixtures, and audio-adapter changes are excluded.

## ACTIVE-TDD behavioral contracts

These are scope hypotheses, not fixture layouts or trace promises. Each ACTIVE
chunk creates only the self-authored pair and layout required for its focused
test, cites local code/symbols under `docs/TFMXLegacy/PROVENANCE.md`, and records
the observed trace, render result, and completion behavior after validation.

| Fixture family | Bounded behavior | ACTIVE TDD evidence |
| --- | --- | --- |
| Finite loop | A self-authored pattern-level `0xF1` loop repeats a selected musical event and eventually reaches the existing completion boundary without changing bridge-global loop state. | Start, tick, snapshot, render, and completion tests establish the actual loop count, pitch/state trace, audio activity, and completion semantics. |
| Effects and tempo | A self-authored fixture combines one selected supported envelope behavior with `tempo[0] = 2`, showing per-engine-tick effects and prescaled pattern processing as distinct behaviors. | Focused tests establish the actual effect trace, pattern-service cadence, render activity, and completion behavior; no dynamic eClock/frame-count assertion is in scope. |
| Voices 0 and 1 | A self-authored non-multimode fixture drives voices 0 and 1 independently and exercises current mixer routing/blend. | The private all-voice snapshot plus focused render tests establish selected voice states and one bounded routing observation; no whole-buffer fingerprint is in scope. |

An unexpected trace is compatibility evidence. Record it, retain the failing
test, and pause for Track revision before changing a legacy semantic or
acceptance criterion.

### Selected malformed corpus

The ACTIVE malformed-fixture TDD chunk creates one self-authored pair for each
selected family: truncated MDAT, unaligned track pointer, out-of-range pattern
pointer, invalid active binding, invalid inactive binding, invalid stop step,
invalid pattern contract, invalid macro ordering, sample-range overflow, and
silent sample payload. Its layout specification records the exact bytes and
the local loader check exercised by each pair.

Each focused test expects `TFMX_LOAD_INVALID_FORMAT` and operational
transactional preservation: load valid step8; attempt replacement; observe
rejection; then start subsong 0, tick once, and obtain a valid prior-module
snapshot—not merely `is_loaded()`. New loader error statuses are out of scope.

### Exact private multi-voice snapshot proposal

The proposed private declaration is additive in
`src/playback/playback_context.h` only:

```c
enum { TFMX_PLAYBACK_SNAPSHOT_VOICE_COUNT = 8 };

typedef struct tfmx_voice_snapshot_set {
    tfmx_voice_snapshot voice[TFMX_PLAYBACK_SNAPSHOT_VOICE_COUNT];
} tfmx_voice_snapshot_set;

tfmx_snapshot_status tfmx_playback_context_snapshot_all(
    const tfmx_playback_context *context,
    tfmx_voice_snapshot_set *snapshot);
```

It reuses the existing snapshot status enum. `NULL` context or output returns
`TFMX_SNAPSHOT_INVALID_ARGUMENT`; an unstarted context returns
`TFMX_SNAPSHOT_NOT_STARTED`; neither failure writes output. On success, the
call copies all eight cached value records indexed 0–7. The cache samples all
eight voices after each successful tick and resets after successful start or
replacement load. Existing voice-0 `snapshot()` behavior is preserved. This
contract is private, SDL-free, single-global/non-reentrant, returns no
pointers, and does not add a public API.

### Compatibility-impact and documentation decision matrix

| Dimension | Decision and reason |
| --- | --- |
| C API/ABI and MCP contract | Unchanged publicly. The proposed snapshot type/function is additive and private under `src/playback/`; no public header, endpoint, or MCP behavior changes. |
| Existing fixture/module compatibility | Preserve all 15 predecessor CMocka cases, including step8's five-tick trace, silent t1–t2/non-silent t3 render, and t29 completion. No on-disk extension; after approved implementation, the loader accepts only the three named new self-authored layouts in addition to step8. |
| Interpreter, timing, and audio behavior | No intended `player.c` semantic change. Fixtures witness current ordering, loop, envelope, prescale, routing, and blend behavior. Dynamic eClock/render timing is expressly unchanged and out of scope. |
| Persistent DAW format/versioning | Unchanged: no DAW persistence or format-versioning feature exists or is introduced. |
| Platform and audio-output adapter | Unchanged: macOS/Clang and the private SDL-free target remain the validation boundary; no SDL device, `main()`, Audio Output Port, or CoreAudio work. |
| Component/package boundaries | Unchanged publicly: context, bridge, loader, and mixer remain private playback implementation seams. No ownership redesign, reentrancy, or general observability boundary is introduced. |
| README, architecture, and glossary | Update `docs/ARCHITECTURE.md` in the implementation change so its test-layout/current-inventory record includes the expanded self-authored corpus and private all-voice snapshot seam. Review `README.md` and `docs/GLOSSARY.md`; no update is planned because no public CLI, C API, user-visible module-support claim, or canonical public term changes. Update either in the same change if the implementation expands such a boundary. |

### Required validation evidence

- For every coherent TDD chunk, record focused red evidence, smallest green change, refactor review, and the Track update immediately after validation.
- Preserve the predecessor's 21 focused CMocka cases before adding new evidence; the corrective S6a and S7 suite reaches 32 cases.
- Run and record: CMake configuration with the Homebrew CMocka prefix; focused `test_playback_context` build and execution; full `ctest --test-dir build --output-on-failure`; full C23 build; `git diff --check`; and confirmation that the focused target remains free of SDL runtime linkage.
- Validate each behavioral contract through its focused TDD evidence, the private snapshot's null/unstarted/output-write/reset behavior, and every malformed pair's operational transactional preservation.
- Review every fixture layout for self-authorship, local code/symbol citations, required **[inferred]** or **[unverified]** markers, and absence of copied module/sample content.

Plan (execution steps)
- [x] S1) Move Track TRACK_005 to ACTIVE (folder, filename, and title status); resolve required decisions and obtain approval before implementation.
- [x] S2) Re-read the ACTIVE Track; inspect `docs/TFMXLegacy/PROVENANCE.md`, relevant legacy interpreter/loader evidence, the current private playback boundary, and Track 002; declare the selected bounded behavior and next coherent TDD chunk. Do not pre-commit exact fixture bytes or traces; record them only as focused TDD evidence. Confirm that fixture-specific loader validation remains bounded to the approved contracts.
- [x] S3) Write focused failing coverage and create only the self-authored pattern-level finite-loop fixture/layout required by that chunk; implement the smallest passing behavior only if the failure identifies a production gap within approved fixture-validation scope; validate and update this Track.
- [x] S4) Write focused failing coverage and create only the self-authored envelope/tempo fixture/layout required by that chunk; implement the smallest passing behavior only if the failure identifies a production gap within approved fixture-validation scope; do not add render timing/eClock behavior; validate and update this Track.
- [x] S5) Write focused failing coverage for the bounded private fixed eight-voice snapshot extension, including null/unstarted behavior, success output, and reset behavior; implement the smallest passing private extension, validate it, and update this Track before creating the multiple-voice fixture.
- [x] S6) Write focused failing coverage and create only the self-authored multiple-voice fixture/layout required by that chunk; make only the bounded loader validation adjustment for its documented active voice-0/voice-1 bindings; prove those selected voices through the private extension, validate, and update this Track.
- [x] S6a) Write focused failing coverage for private clean-start playback restart isolation, implement the smallest bridge-owned runtime reset so a fresh Step8 first tick is independent of prior playback, update `docs/ARCHITECTURE.md`, validate, and update this Track; limit changes to `tests/playback/test_playback_context.c`, `src/playback/playback_legacy_bridge.c`, `docs/ARCHITECTURE.md`, and this Track, with no loader, interpreter, public API, CMake, fixture, or audio-adapter change.
- [x] S7) Write focused failing rejection and transactional-loading coverage and create only the ten selected self-authored malformed-fixture pairs/layout required by that chunk; assert `TFMX_LOAD_INVALID_FORMAT` for each and preservation of a prior valid load; implement the smallest passing behavior only if within approved fixture-validation scope; validate and update this Track.
- [x] S8) Run focused and full automated validation, update `docs/ARCHITECTURE.md`, inspect fixture/layout and documentation evidence plus the exact diff, then record inventory, validation, mismatches, and deferred safeguards.
- [x] S9) Inspect the linked roadmap, record whether it remains current or has a user-approved revision proposal, and complete only after all acceptance evidence is captured.

Current inventory
- `TRACK_002_COMPLETED_compatibility_safeguards.md` established the private playback-context boundary and one self-authored dual-file fixture (`mdat.step8`/`smpl.step8`) with a documented voice-0 trace, canonical rendering, and completion evidence.
- S2/S3 finite-loop chunk changed only `tests/playback/test_playback_context.c`, `tests/fixtures/mdat.loop_f1`, `tests/fixtures/smpl.loop_f1`, `tests/fixtures/loop_f1_layout.md`, and `src/playback/tfmx_loader.c`; the layout and loader recognition remain limited to the named self-authored pattern-level `0xF1` fixture.
- S4 effects/tempo chunk changed only `tests/playback/test_playback_context.c`, `tests/fixtures/mdat.envelope_tempo`, `tests/fixtures/smpl.envelope_tempo`, `tests/fixtures/envelope_tempo_layout.md`, and `src/playback/tfmx_loader.c`; the layout and loader recognition remain limited to the named self-authored envelope/tempo fixture.
- `src/playback/playback_context.h`/`.c`, `src/playback/playback_legacy_bridge.h`/`.c`, and `src/playback/playback_legacy_mixer.h`/`.c` implement the current private, SDL-free playback evidence boundary; the copied legacy bridge remains single-global and non-reentrant. S5 adds only a context-owned post-success-tick cache of eight value records, each containing `active`, `pitch`, and `volume`; the bridge samples voices 0–7 after an IRQ without escaping pointers.
- `src/playback/tfmx_loader.h`/`.c` owns the separate MDAT/SMPL candidate loader, structural validation, normalized metadata, and transactional context commit for the narrow current fixture contract.
- The S5 snapshot cache records all eight voices after a successful tick. S6 adds only fixture-specific validation and evidence of the documented active voice-0/voice-1 bindings; broader layout acceptance remains out of scope.
- The current loader accepts a fixed three-word pattern and eight-word macro contract for the existing fixture; the selected loop and envelope/tempo fixtures need similarly bounded, documented fixture validation rather than broad module-format acceptance.
- The approved loader boundary preserves the existing valid fixture and is limited to three additional named self-authored valid layouts plus ten named malformed counterparts; arbitrary layout acceptance remains out of scope.
- `tests/playback/test_playback_context.c` provides the existing focused CMocka coverage; `tests/fixtures/mdat.step8`, `tests/fixtures/smpl.step8`, and `tests/fixtures/step8_layout.md` are the existing shared self-authored fixture evidence.
- S6 adds the self-authored `mdat.voices_01`/`smpl.voices_01` fixture pair and `tests/fixtures/voices_01_layout.md`; `src/playback/tfmx_loader.c` now recognizes only the exact named voices-0-and-1 layout and validates its documented active bindings, including the `EFFE 0000` stop marker.
- `docs/ARCHITECTURE.md` identifies file-format, interpreter, timing, and audio semantics as preserved layers, and reserves `tests/fixtures/` for shared self-authored fixtures.
- S8 strict documentation is complete. The approved documentation changes are `docs/TFMXLegacy/PROVENANCE.md` (an S8 documentation correction, not an unrelated change), `docs/ARCHITECTURE.md`, the four valid-layout documents (`step8_layout.md`, `loop_f1_layout.md`, `envelope_tempo_layout.md`, and `voices_01_layout.md`), and the ten malformed-layout documents (`truncated_mdat_layout.md`, `unaligned_track_layout.md`, `out_of_range_pattern_layout.md`, `invalid_active_binding_layout.md`, `invalid_inactive_binding_layout.md`, `invalid_stop_step_layout.md`, `invalid_pattern_contract_layout.md`, `invalid_macro_ordering_layout.md`, `sample_range_overflow_layout.md`, and `silent_sample_payload_layout.md`). `README.md`, `docs/GLOSSARY.md`, CMake, and public headers remain unchanged because no public terminology or boundary changed.
- The current focused boundary exercises four valid self-authored fixture pairs and ten malformed self-authored fixture pairs. The private loader remains a fixture-specific byte-layout recognizer, and the private all-voice snapshot remains a bounded eight-record value snapshot; neither is a general parser or public observability API.
- The Loader/Writer → Module Domain Model → Playback Engine contract remains open; this Track does not resolve it.
- S6 validation: focused CMocka passed 21/21; CTest passed 3/3; the full parallel build passed; `git diff --check` passed; and the focused target uses CMocka and `libSystem` only, with no SDL linkage.
- The current focused suite contains 32 tests after the S6a clean-start case and ten S7 malformed-replacement cases. S8 validation and documentation review are complete; S9 roadmap reconciliation and Track closure are complete.

Artifacts
- `TRACK_002_COMPLETED_compatibility_safeguards.md` — predecessor private-boundary and first-fixture evidence, including explicitly deferred Phase 2 safeguards.
- `docs/ARCHITECTURE.md` — preserved semantic layers, current implementation, and test-layout decisions.
- `docs/TFMXLegacy/PROVENANCE.md` — required provenance and citation policy before fixture-layout work.
- Living roadmap: `TFMX.cpp modernization roadmap`, Phase 2 — compatibility safeguards remain in progress; this Track is a bounded roadmap-derived safeguard.

Completion notes
- S2 inspection/declaration: complete. The ACTIVE Track, `docs/TFMXLegacy/PROVENANCE.md`, relevant local interpreter/loader evidence, the private playback boundary, and Track 002 were reviewed. The selected bounded chunk was a self-authored pattern-level `0xF1` finite loop using only the private load/start/tick/snapshot/render/completion boundary; exact bytes and traces were deferred to the focused test. The independent review verified binary/layout self-authorship, local-code/provenance labels, narrow fixture-specific loader recognition, intact predecessor coverage, and no copied external module or sample content.
- S3 finite-loop TDD chunk: complete. Changed files were `tests/playback/test_playback_context.c`, `tests/fixtures/mdat.loop_f1`, `tests/fixtures/smpl.loop_f1`, `tests/fixtures/loop_f1_layout.md`, and `src/playback/tfmx_loader.c`.
- S3 red evidence: 16 tests ran; the 15 predecessor tests passed and the new finite-loop case failed with `TFMX_LOAD_INVALID_FORMAT` (`3 != 0`) because the loader rejected the named fixture layout.
- S3 green evidence: focused validation passed 16/16. The finite loop completed at tick 85; the selected state repeated 6 times; 6 renders were non-silent; and completion was observed within the 128-tick bound.
- S3 refactor and validation: removed a redundant duplicate metadata `memset`; focused 16/16 validation passed again, full CTest passed 3/3, the parallel build passed, and `git diff --check` passed. `otool -L build/test_playback_context` listed CMocka and `libSystem` only, with no SDL linkage.
- S3 compatibility and scope: no legacy interpreter, bridge, mixer, public API/ABI/MCP, DAW persistence, platform/audio boundary, CMake, or Git history changed. Acceptance is limited to the named loop fixture; no generic parser was added. The tick-85 completion and six observations are simple behavioral bounds for this repository fixture, not an exact compatibility assertion. Later multi-voice, malformed-fixture, architecture-documentation, and roadmap work remain unchecked/deferred.
- S4 effects/tempo TDD chunk: changed files were `tests/playback/test_playback_context.c`, `tests/fixtures/mdat.envelope_tempo`, `tests/fixtures/smpl.envelope_tempo`, `tests/fixtures/envelope_tempo_layout.md`, and `src/playback/tfmx_loader.c`.
- S4 red evidence: 17 tests ran; the 16 predecessor tests passed and the new case failed with `TFMX_LOAD_INVALID_FORMAT` before loader recognition.
- S4 green evidence: `tempo[0]=2`, observed pre=2; the pattern command was `0xF7` with no macro `0x0F`; voice 0 had pitch 0 at tick 1 and pitch `0x06AE` from tick 2 onward, with the active tick-3 and envelope behavior preserved; volumes were 0 at ticks 1–2, 15 at ticks 3–14, 12 at ticks 15–16, 9 at ticks 17–18, and 6 at tick 19. There were 17 non-silent caller-owned renders of 3524–3528 bytes; completion occurred at tick 19 and the subsequent tick remained complete.
- S4 refactor and independent review: the loader change is an exact-layout fixture branch; predecessor acceptance remains intact. The fixtures and layout are self-authored and provenance-compliant, with no review concern.
- S4 validation: focused 17/17 passed; full CTest passed 3/3; the parallel build passed; `git diff --check` passed; and the focused target has no SDL linkage, using CMocka and `libSystem` only.
- S4 compatibility and scope: no interpreter, context, bridge, mixer, public API/ABI/MCP, persistence, platform/audio boundary, CMake, or Git history changed. The change is fixture-specific loader recognition only, not a general parser. Snapshot S5 and eClock/render-frame timing work remain out of scope and unchecked.
- S5 all-voice snapshot TDD chunk: changed only `src/playback/playback_context.h`, `src/playback/playback_context.c`, `src/playback/playback_legacy_bridge.h`, `src/playback/playback_legacy_bridge.c`, and `tests/playback/test_playback_context.c`. Red evidence was the 17/17 baseline followed by focused S5 tests failing at compile because the snapshot set, count, and API were missing.
- S5 green evidence: focused validation passed 20/20 while preserving all 17 predecessor cases; CTest passed 3/3; the full parallel build passed; `git diff --check` passed; and the focused test target linked only CMocka and `libSystem`, with no SDL. Tests establish null/unstarted no-write sentinels, eight zeroed records after start, voice-0 equivalence with the old single snapshot after tick, stable repeated snapshots without a tick, and reset of all records after restart and successful reload/restart. No voice-1 active claim was made.
- S5 boundary and review: the extension is a private additive seam only, with eight caller-owned value records containing `active`, `pitch`, and `volume`; the context owns the cache and the bridge samples voices 0–7 after an IRQ. No pointers escape, and old voice-0 snapshot behavior is unchanged. No fixtures, loader, CMake, public API/ABI, legacy interpreter, audio/device, persistence, documentation, or Git history changed. Independent review confirms cache assignment occurs only after a successful bridge tick; a forced started-tick failure is not injectable through the current seam, so that case is implementation-reviewed rather than separately forced by test. Later S7–S9 work remains unchecked.
- S6 multiple-voice TDD chunk: changed files were `tests/playback/test_playback_context.c`, `tests/fixtures/mdat.voices_01`, `tests/fixtures/smpl.voices_01`, `tests/fixtures/voices_01_layout.md`, and `src/playback/tfmx_loader.c`.
- S6 red evidence: 21 tests ran; 20 predecessor tests passed and the new dual-voice case failed with `TFMX_LOAD_INVALID_FORMAT` before fixture recognition.
- S6 green evidence: focused validation passed 21/21. The first joint active tick was 3; voice 0 had active pitch `0x06AE` and volume 18; voice 1 had active pitch `0x064E` and volume 30; voices 2–7 were inactive. The caller-owned 3528-byte render was nonzero with exact observed first-frame left 1680 and right 1392. Completion occurred at tick 35 and both selected voices were inactive. The behavioral-bounds routing test strategy remains approved; these exact figures are observations, not a new exact audio contract.
- S6 review and validation: independent review verified the self-authored, provenance-cited layout, the `[inferred]` routing label, same-tick independent snapshots, the bounded nonzero left-greater-than-right observation, and exact named loader metadata. The review's missing stop-marker trailing-zero validation was corrected; post-fix verification confirms the branch validates `EFFE 0000`, focused 21/21 passes, CTest passes 3/3, the parallel build and diff check pass, and no SDL linkage or remaining concern exists.
- S6 compatibility and scope: acceptance is limited to the exact named `voices_01` fixture; no parser generalization was added. No API/context/bridge/mixer/interpreter/CMake/public/MCP/SDL/device/persistence/Git history change occurred. S7 malformed-fixture work, S8 architecture and final corpus validation, and S9 roadmap reconciliation remain unchecked and deferred.
- S6a clean-start corrective TDD chunk: the pre-implementation decision was already recorded. The bounded source scope was the bridge reset and focused test, with `docs/ARCHITECTURE.md` as the documentation scope; no public, module, loader, interpreter, mixer, CMake, API, or device change occurred. Root cause was that bridge reset omitted mutable runtime state, allowing a fresh start to inherit prior `CurPeriod` and effect fields.
- S6a red evidence: 32 tests ran; 20 passed and 12 failed: the clean-start test, the dual-voice test, and ten malformed tests. Per-test red output was not preserved. Green evidence was 32/32, CTest 3/3, the parallel build, and `git diff --check`; the focused target used CMocka and `libSystem` only, with no SDL linkage.
- S6a deterministic evidence and implementation: after the envelope-tempo tick-14 state was active/pitch `0x06AE`/volume 15, a fresh Step8 start at tick 1 was exactly inactive/pitch 0/volume 0. The bridge now clears bridge-owned mutable runtime before existing `TfmxInit()`/`StartSong` rebinding. The single-global, non-reentrant condition is preserved, and all S7 transactional checks pass after reset.
- S7 malformed-fixture TDD chunk: ten self-authored pairs/layouts and ten tests were added. Each returns `TFMX_LOAD_INVALID_FORMAT`; after a valid Step8 load and failed replacement, the prior module still starts, ticks, and snapshots clean `0/0/0`. All predecessor mutation tests were retained. With fixture files absent, red evidence was 31 total: 21 predecessors passed and the ten new cases returned IO errors. After the fixtures and bridge correction, green evidence was 32/32, full CTest 3/3, the parallel build, `git diff --check`, and no SDL linkage for the focused target.
- S7 scope caveats: the sample-range-overflow pair returns invalid format through earlier exact macro-contract validation, not a direct range predicate; this is a deferred direct-range-validation safeguard, not proof of range-predicate coverage. Finite-loop exact-layout tightening remains a separate deferred corrective chunk by user decision; the loader was not altered for it.
- Track activation is complete with user approval. No source, tests, fixtures, build configuration, documents outside this Track, durable-memory, commit, or push change occurred in this activation step; no implementation completion is claimed.
- M1 pre-activation evidence: complete. The bounded fixture behavior and local-code evidence are recorded in this Track's behavioral contracts, decision matrix, and current inventory, with the predecessor's verified step8 load/tick/snapshot/render/completion boundary and malformed transactional checks documented in `TRACK_002_COMPLETED_compatibility_safeguards.md`. The self-authored layout approach and verified existing trace are documented in `tests/fixtures/step8_layout.md`; the selected ten malformed families and their bounded rejection review are recorded in Decision (Q3) and `Selected malformed corpus`. No unexecuted Track 005 trace, PCM result, completion position, or fixture bytes are claimed as M1 acceptance evidence.
- DRAFT created with user approval. No implementation, test, fixture, configuration, production-source, durable-memory, commit, or push change is authorized or recorded by this creation step.
- User approved the pattern-level finite-loop, envelope/`tempo[0] = 2`, and ten-case malformed-corpus behavior categories while this Track remains DRAFT. No implementation, test, fixture, configuration, production-source, durable-memory, commit, or push change occurred.
- User approved a finite-fixture-recognizer loader boundary while this Track remains DRAFT. No implementation, test, fixture, configuration, production-source, durable-memory, commit, or push change occurred.
- DRAFT recovery: exact fixture bytes, tick traces, PCM values, and completion positions are no longer DRAFT promises. The canonical backlog gate requires bounded scope and approved activation; ACTIVE TDD establishes exact behavior. The required `docs/ARCHITECTURE.md` implementation update remains in scope. No implementation, test, fixture, production-source, canonical-document, durable-memory, commit, or push change occurred.
- S8 strict documentation and final audit: complete. The documentation set reviewed and recorded above includes the approved S8 correction to `docs/TFMXLegacy/PROVENANCE.md`; that correction is part of this Track's documentation work, not an unrelated change. The independent fixture audit covered 14/14 fixtures (four valid and ten malformed). Explicit sizes, offsets, words, sample values, and mutations matched; every layout is self-authored with no external material; local/repository-limited claims and canonical provenance markers are compliant. The full untracked inventory was included in review: 39 fixture/layout artifacts plus this ACTIVE Track record, with no unrelated changes.
- Final validation: `cmake -S . -B build -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/cmocka` configured successfully; focused build and execution passed 32/32; CTest passed 3/3; the full parallel C23 build passed; `git diff --check` produced no output; and `otool -L` for the focused target listed only Homebrew CMocka and `libSystem`, with no SDL linkage.
- Final compatibility conclusion: public API/ABI/MCP, existing module and general-parser support, interpreter order, persistence, platform/audio adapter, component boundary, and the single-global/non-reentrant constraint remain as previously assessed. The only bounded additions to state are the private fixture-specific byte-layout recognizer and the private all-voice snapshot; neither establishes a general parser, public API, or multi-context boundary.
- Confirmed discrepancy corrections: invalid active-binding, macro-ordering, and pattern-contract layout records now match their binaries; S4 and S6 clean-reset observations match the documentation and tests. No false public compatibility claim is made.
- Deferred safeguards remain: the generic loader/domain contract; audio fingerprinting; reentrancy and multi-context playback; dynamic eClock assertions; direct sample-range predicate coverage; and finite-loop exact-layout tightening.
- S9 roadmap reconciliation: complete. The Phase 2 roadmap was revised to include Track 005's bounded delivery; the roadmap index is unchanged because phase and order remain Phase 2 in progress.
- Final validation evidence remains: focused 32/32, CTest 3/3, full C23 build, `git diff --check`, SDL-free focused linkage, and 14/14 byte-level fixture audit.
- Public API/ABI/MCP, generic parser/module compatibility, interpreter order except the private clean-start reset, persistence, platform/audio adapter, and the private non-reentrant boundary remain as recorded.
- Deferred safeguards remain retained as recorded.
- No commit or push exists yet, so no changelog memory was created.
