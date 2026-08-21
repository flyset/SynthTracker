# TRACK TRACK_012 [COMPLETED]: audio_output_null_adapter

Track
- ID: TRACK_012
- Repository: SynthTracker
- Branch: stage/04-03-audio-output-extraction
- Current path: .backlog/COMPLETED/2026/TRACK_012_COMPLETED_audio_output_null_adapter.md
- Status: COMPLETED

Problems (PORE)
- P1: As a SynthTracker maintainer, I cannot test any `Audio Output` boundary behavior independently of a device, because `Audio Output` exists only as approved target vocabulary in ADR-005/ASR-006 and no component, port, or adapter is implemented.
- P2: As a Phase 4 contributor, I cannot prove the ADR-007 fixed first format or the ASR-009 Audio Frame Block validity and distinct-failure invariants, because no component owns Audio Frame Block validation and ASR-009's verification is recorded as "focused component tests when the Audio Output boundary is implemented".
- P3: As a maintainer extracting components, I risk binding the first `Audio Output` work to SDL-era device behavior in `src/audio.c`, because the only existing output path is the legacy SDL callback/ring-buffer code, which offers no device-independent seam to test against.

Objective
- Establish the first private, device-independent `Audio Output` component with fixed-format Audio Frame Block validation and a synchronous null adapter, covered by focused CMocka component tests, implemented only after this Track is ACTIVE.

Non-negotiables
- This Track is planning only while DRAFT. No implementation—including tests intended to drive a code change—begins until the Track is moved to ACTIVE and its S1 Move-to-ACTIVE plan step is checked.
- All implementation follows TDD: a focused failing component test for the intended observable behavior, the smallest passing implementation, then refactoring and validation.
- All new production and test source is C23 or later ISO C; C++ is not a project direction.
- All new project-owned production and test headers are private and co-located in their owning source or test folder (ASR-008); no project-owned header may be added under a retired `include/` location.
- The component is private. This Track creates no public C API, ABI, library-header model, or public package boundary.
- The fixed first format is exactly 44.1 kHz, stereo, interleaved, signed 16-bit, little-endian PCM (ADR-007). No format negotiation or conversion is introduced.
- The ASR-009 invariants are required: a zero-frame block is valid exactly when `payload_length == 0`, regardless of whether `payload` is NULL; a zero-frame block with nonzero payload length is an incorrect-length failure; a nonzero block's payload length must be exactly the non-overflowing `frame_count × 4` bytes; missing payload and incorrect payload length must produce distinct failures. A nonzero overflowing frame count is an incorrect-length failure before missing-payload classification.
- The null adapter accesses no audio device, opens no SDL/CoreAudio handle, performs no file or network I/O, retains no pointer to caller payload after the call returns, and does not spawn threads.
- Phase 4 compatibility policy applies: preserving current TFMX behavior here is a temporary development scaffold, not a SynthTracker v1 compatibility promise. Current TFMX emission, routing, and SDL-era playback behavior must remain observably unchanged, evidenced by the existing suite.
- Every behavior change requires automated coverage. Structural, layout, or source-text inspection is review support only and never behavioral evidence.

Acceptance criteria
- [x] A1) [P1, P2] A focused component test proves that zero-frame validity depends only on payload length: `{ frame_count = 0, payload_length = 0 }` is accepted as an observable no-op whether `payload` is NULL or non-NULL; `{ frame_count = 0, payload_length > 0 }` is rejected as `AUDIO_OUTPUT_SUBMIT_INCORRECT_PAYLOAD_LENGTH` regardless of payload nullness; all cases leave both counters unchanged.
- [x] A2) [P2] A focused component test proves that a valid nonzero, non-overflowing frame count whose payload length is exactly `frame_count × 4` bytes for the fixed format is accepted, at more than one distinct nonzero frame count.
- [x] A3) [P2] A focused component test proves that a nonzero, non-overflowing block with a missing payload is rejected and not accepted by the adapter.
- [x] A4) [P2] A focused component test proves that a nonzero block whose payload length differs from `frame_count × 4` is rejected, and that this failure is observably distinct from the missing-payload failure of A3 (two different reported outcomes, asserted as unequal).
- [x] A5) [P1, P3] A focused component test proves the null adapter's observable behavior: it synchronously accepts each valid block, discards it, retains no reference to caller-owned payload memory after return (verified by mutating or invalidating caller memory after the call and observing no adapter-visible effect), and links and runs with no SDL, CoreAudio, or other device dependency in its test target.
- [x] A6) [P1, P3] The component is private and co-located: its production sources and headers live in one owning `src/` component folder, its tests live in the matching `tests/` component folder, no project-owned header is added outside an owning folder, and no public API/ABI or public package boundary is created.
- [x] A7) [P1, P2, P3] After implementation, C23 configure/build succeeds and the full CTest suite passes, including the new component tests and the existing TFMX/playback compatibility evidence, with no observed change to current TFMX emission, routing, or SDL-era playback behavior.
- [x] A8) [P2] A focused component test proves every nonzero `frame_count` for which `frame_count × 4` overflows `size_t` is rejected as `AUDIO_OUTPUT_SUBMIT_INCORRECT_PAYLOAD_LENGTH`, with no counter changes, for both NULL and non-NULL payload variants.

Why now / impact
- Phase 4 component extraction is the current execution boundary, and Tracks 009–011 completed the structural entrypoint, header co-location, and product identity work. ADR-007 and ASR-009 now fix the Audio Frame Block's first format and invariants, which makes a deterministic, device-free `Audio Output` component the smallest testable first extraction at that boundary. Doing it with a null adapter establishes the device-independent port before any SDL or CoreAudio adapter can bind the boundary to device semantics.

Scope
- In scope:
  - A private, co-located production/header structure for one `Audio Output` component under an owning `src/` folder.
  - Fixed-format Audio Frame Block representation sufficient to carry frame count and payload for the ADR-007 first format.
  - Audio Frame Block validation implementing the ASR-009 invariants, including distinct failures for missing payload and incorrect payload length.
  - A synchronous null adapter behind the device-independent port that accepts valid blocks and discards them.
  - Focused CMocka component tests in the matching `tests/` component folder.
  - CMake changes only as needed to build, link, and register the new component test target with CTest.
  - TDD execution and Track evidence recording once ACTIVE.
- Out of scope:
  - Changing current TFMX emission or routing.
  - Changing current `src/audio.c` (mixing, filtering, stereo blending, ring buffer, SDL callbacks, pthread synchronization).
  - Changing current SDL-era playback behavior.
  - Implementing SDL, CoreAudio, or any other device adapter.
  - Switching the CLI or any current code path to the new output component.
  - Format negotiation or conversion beyond the fixed first format.
  - Any public API, ABI, library-header model, or public package boundary.
  - Ownership, borrowing/copying, lifetime, timing, ordering, queueing, and backpressure design at the consumer boundary.
  - File I/O and rendered-audio export routing.
  - GUI, editor, or `Model` work.
  - Durable project memory changes and Git history changes.

Milestones
- [x] M1) Track is approved and moved to ACTIVE with S1 checked.
- [x] M2) Fixed-format Audio Frame Block validation satisfies the ASR-009 invariants under focused failing-test-first chunks.
- [x] M3) The synchronous null adapter satisfies its accept-and-discard, no-retention, and no-device behavior under focused failing-test-first chunks.
- [x] M4) Private boundary and co-located layout are in place, CMake registers the component test target with CTest, and the full validation set plus existing TFMX compatibility evidence passes.

Risks / decisions
- Risk (planning gate): Implementation cannot begin before this Track is ACTIVE and S1 is checked. Any test written to drive the new component is implementation and is blocked by the same gate.
- Decision: Treat ADR-005, ADR-007, ASR-006, ASR-009, and `docs/ARTIFACTS.md` as the normative design basis for this Track. Commit `46c971e` ("Document Audio Frame Block boundary and fixed first format") committed ADR-007, ASR-009, `docs/ARTIFACTS.md`, and the related `README.md`, `docs/ADR.md`, `docs/ARCHITECTURE.md`, and `docs/GLOSSARY.md` updates; this design basis is historical committed evidence, not uncommitted working-tree content.
- Risk (scope creep into deferred contracts): Implementing a consumer boundary invites resolving ownership, lifetime, timing, queueing, and backpressure, which ADR-007 and `ARTIFACTS.md` explicitly defer.
- Decision: Keep the adapter synchronous and the block borrow-only for the duration of the call, assert no retention behaviorally, and leave every deferred contract question open. If a deferred question must be resolved to proceed, update this Track or open a separate Track before proceeding.
- Risk (device coupling): Wiring the new component into the current SDL path would bind the first port design to legacy device behavior and could change audible behavior.
- Decision: The null adapter is the only implementation in this Track, and no current code path is switched to it; `src/audio.c` remains untouched.
- Risk (compatibility regression): Adding sources and CMake targets can disturb the existing build or test suite.
- Decision: Add new targets additively, leave existing target definitions intact except where a new target requires registration, and require the full existing suite to pass as part of A7.
- Version impact (C API/ABI): Unchanged. The component, its header, and its types are private and co-located; no public header, library-header model, exported symbol contract, or ABI surface is created, and no existing declaration is modified.
- Version impact (module compatibility/extension): Unchanged. No TFMX bytes, loader behavior, format detection, module extension, or legacy data layout is read, written, or modified by this Track.
- Version impact (interpreter/timing/audio behavior): Unchanged. `src/player.c`, `src/tfmx.c`, and `src/audio.c` are untouched; no current emission, routing, mixing, filtering, stereo blending, or SDL callback path is modified, and nothing is switched to the new component.
- Version impact (persistent DAW format/versioning): Unchanged. No serialized project data, import/export contract, or version field is introduced; the Audio Frame Block is an in-memory boundary artifact only.
- Version impact (platform/audio-output adapter): Unchanged for existing adapters. The current macOS SDL-era output path and platform scope remain the implemented output path. The added null adapter is device-free, is not registered as a platform adapter, and does not implement or preempt the intended CoreAudio adapter.
- Version impact (component/package boundaries): Changed only privately. A new private `Audio Output` component boundary is added with its own co-located tests. No public package boundary changes, no existing component boundary is redefined, and the private `src/playback/` compatibility seam is unaffected.

Open questions
- [x] Q1) Resolved: the owning folders are `src/audio_output/` and `tests/audio_output/`.
- [x] Q2) Resolved: one private block struct with `size_t frame_count`, `const uint8_t *payload`, `size_t payload_length`, and a private submit-result enum with `ACCEPTED`, `MISSING_PAYLOAD`, `INCORRECT_PAYLOAD_LENGTH`.
- [x] Q3) Resolved: private scalar accepted-block and accepted-byte counters, with no retained payload pointer.
- [x] Q4) Resolved: a standalone `test_audio_output` CMocka executable, registered with CTest, linked only to CMocka.

Decision log
- Decision (gate): No implementation, including new tests, occurs while this Track is DRAFT; S1 is the immediate next step and no implementation plan step is pre-checked.
- Decision (design basis): ADR-005 and ASR-006 authorize the device-independent `Audio Output` direction; ADR-007 and ASR-009 fix the first format and validity invariants; `docs/ARTIFACTS.md` catalogs the Audio Frame Block artifact and its deferred contracts. ADR-007, ASR-009, and `docs/ARTIFACTS.md` were committed in `46c971e` ("Document Audio Frame Block boundary and fixed first format") and are historical committed evidence, not uncommitted working-tree content.
- Decision (format): The fixed first format is 44.1 kHz, stereo, interleaved, signed 16-bit, little-endian PCM, giving 4 bytes per frame; nonzero payload length must equal `frame_count × 4`.
- Decision (adapter behavior): The null adapter synchronously accepts valid blocks and discards them, without retaining payload data, accessing a device, or performing I/O. Concrete implementation and tests are deferred until this Track is ACTIVE.
- Decision (compatibility): Every Phase 4 compatibility dimension is unchanged for the reasons recorded under Risks / decisions, except that component/package boundaries gain a private `Audio Output` component while no public package boundary changes.
- Decision (S2 — Q1–Q4): Owning folders are `src/audio_output/` and `tests/audio_output/`. The concrete representation is one private block struct (`size_t frame_count`, `const uint8_t *payload`, `size_t payload_length`) and a private submit-result enum with `ACCEPTED`, `MISSING_PAYLOAD`, `INCORRECT_PAYLOAD_LENGTH`. The null adapter exposes only private scalar accepted-block and accepted-byte counters, with no retained payload pointer. The test target is a standalone `test_audio_output` CMocka executable registered with CTest and linked only to CMocka.
- Decision (corrective validation semantics): Zero-frame blocks are accepted exactly when `payload_length == 0`, irrespective of the payload pointer; zero-frame blocks with nonzero payload length return `AUDIO_OUTPUT_SUBMIT_INCORRECT_PAYLOAD_LENGTH`. A nonzero count that would overflow `frame_count × 4` returns `AUDIO_OUTPUT_SUBMIT_INCORRECT_PAYLOAD_LENGTH` before missing-payload classification. This corrects private validation behavior only: C API/ABI, module compatibility, interpreter/timing/audio behavior, persistent DAW format/versioning, and existing platform adapters remain unchanged; the private component/package boundary remains unchanged.

Plan (execution steps)
- [x] S1) Move Track TRACK_012 to ACTIVE (folder, filename, and title status) after planning approval; this is the immediate next step and gates every step below.
- [x] S2) Re-read this Track, state the next unchecked step, and resolve Q1–Q4: owning folder names, the private block/validation-result representation, minimal adapter observability, and the test-target shape. Record the decisions here before writing any code.
- [x] S3) TDD chunk — zero-frame validity: failing focused test for A1, smallest passing validation and adapter no-op, refactor, run validation, update this Track.
- [x] S4) TDD chunk — exact nonzero payload acceptance: failing focused test for A2 at multiple nonzero frame counts, smallest passing change, refactor, run validation, update this Track.
- [x] S5) TDD chunk — missing payload rejection: failing focused test for A3, smallest passing change, refactor, run validation, update this Track.
- [x] S6) TDD chunk — wrong payload length rejection distinct from missing payload: focused characterization test for A4 asserting the two outcomes are unequal, approved exception because the behavior pre-existed S6, run validation, update this Track.
- [x] S7) TDD chunk — null adapter accept/discard, no retention, and no device access: focused characterization test for A5 asserting the existing scalar-only/no-retention behavior, approved exception because the behavior pre-existed S7, run validation, update this Track.
- [x] S8) Confirm A6: private boundary, co-located production/header and test folders, no public API/ABI or public package boundary, and CMake registration of the component test target with CTest; record the structural review as supporting evidence only.
- [x] S9) Run the full compliant validation set for A7 (`cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix cmocka)"`, `cmake --build build --parallel 2`, `ctest --test-dir build --output-on-failure`), confirm existing TFMX/playback compatibility evidence still passes, and record results here. This is a completed pre-correction baseline; it does not satisfy final A7.
- [x] S10) Corrective TDD chunk — zero-frame validation/A1: add focused four-case coverage for NULL/non-NULL payloads at zero and nonzero payload lengths, observe red, minimally correct validation, run focused validation, and update this Track.
- [x] S11) Corrective TDD chunk — checked multiplication/A8: add focused `SIZE_MAX / 4 + 1` coverage for NULL and non-NULL payload variants, observe red, reject overflow before missing-payload classification, run focused validation, and update this Track.
- [x] S12) Rerun the full compliant validation set for final A7 (`cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix cmocka)"`, `cmake --build build --parallel 2`, `ctest --test-dir build --output-on-failure`), confirm existing TFMX/playback compatibility evidence still passes, and record fresh results here.
- [x] S13) Review the exact changed-file inventory and every Phase 4 compatibility dimension against the committed ADR-007/ASR-009/ARTIFACTS design basis (`46c971e`), then move Track TRACK_012 to COMPLETED with acceptance evidence.

S3 evidence
- Red: after adding `tests/audio_output/test_audio_output.c` and its standalone CMocka target, `cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix cmocka)" && cmake --build build --target test_audio_output --parallel 2` failed as intended because the not-yet-created private `src/audio_output/audio_output.h` was missing.
- Changed inventory: added `src/audio_output/audio_output.h`, `src/audio_output/audio_output.c`, and `tests/audio_output/test_audio_output.c`; modified root `CMakeLists.txt` only to add the standalone `test_audio_output` target and CTest registration. The target links only to `cmocka::cmocka`; no SDL, CoreAudio, device, I/O, allocation, threading, payload retention, legacy audio, or existing TFMX path changes.
- Green focused validation: `cmake --build build --target test_audio_output --parallel 2 && ctest --test-dir build --output-on-failure -R '^test_audio_output$'` — 1/1 test passed.
- Broader validation: `cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix cmocka)" && cmake --build build --parallel 2 && ctest --test-dir build --output-on-failure` — build passed and 6/6 CTest tests passed, including existing CLI, compile-probe, playback, and application compatibility evidence.

S4/A2 evidence
- Red: `cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix cmocka)" && cmake --build build --target test_audio_output --parallel 2 && ctest --test-dir build --output-on-failure -R '^test_audio_output$'` — focused CTest failed as intended: the new test submitted 1-frame/4-byte and 3-frame/12-byte blocks, but the first result was `AUDIO_OUTPUT_SUBMIT_INCORRECT_PAYLOAD_LENGTH` instead of `AUDIO_OUTPUT_SUBMIT_ACCEPTED`.
- Green focused validation: `cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix cmocka)" && cmake --build build --target test_audio_output --parallel 2 && ctest --test-dir build --output-on-failure -R '^test_audio_output$'` — configure/build passed and 1/1 focused CTest test passed; both nonzero submissions were accepted and counters totaled 2 blocks/16 bytes.
- Changed inventory for S4: extended `tests/audio_output/test_audio_output.c` with the focused A2 test and changed only `src/audio_output/audio_output.c` to accept non-NULL nonzero payloads of exactly `frame_count × 4` bytes and increment the two scalar counters. No missing-payload or wrong-length distinction, payload retention, overflow policy, device behavior, allocation, threading, headers, CMake, SDL/CoreAudio, or legacy-path work was added.

S5/A3 evidence
- Red: `cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix cmocka)" && cmake --build build --target test_audio_output --parallel 2 && ctest --test-dir build --output-on-failure -R '^test_audio_output$'` — focused CTest failed as intended: the 1-frame `{ .frame_count = 1, .payload = NULL, .payload_length = 4 }` submission returned `AUDIO_OUTPUT_SUBMIT_INCORRECT_PAYLOAD_LENGTH` instead of `AUDIO_OUTPUT_SUBMIT_MISSING_PAYLOAD`; the two existing tests passed.
- Green focused validation: `cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix cmocka)" && cmake --build build --target test_audio_output --parallel 2 && ctest --test-dir build --output-on-failure -R '^test_audio_output$'` — configure/build passed and 1/1 focused CTest test passed; the missing-payload test returned `AUDIO_OUTPUT_SUBMIT_MISSING_PAYLOAD` and both counters remained zero.
- Broader validation: `cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix cmocka)" && cmake --build build --parallel 2 && ctest --test-dir build --output-on-failure` — build passed and 6/6 CTest tests passed, including existing CLI, compile-probe, playback, application, and Audio Output evidence.
- Changed inventory for S5: added one focused missing-payload test in `tests/audio_output/test_audio_output.c` and added only the `frame_count > 0 && payload == NULL` `AUDIO_OUTPUT_SUBMIT_MISSING_PAYLOAD` branch in `src/audio_output/audio_output.c`, after the zero-frame no-op and before exact nonzero acceptance. No S6 wrong-length distinction, retention/device behavior, overflow, headers, CMake, legacy paths, SDL/CoreAudio, allocation, or threading work was added.

S6/A4 evidence (approved characterization exception)
- Red phase exception: the wrong-length distinction behavior pre-existed S6, contrary to the prior Track evidence, so no artificial red phase was manufactured. S6 characterized the existing behavior with one CMocka test using a non-NULL wrong-length `{ frame_count = 1, payload = <non-null>, payload_length = 3 }` block and a missing-payload `{ frame_count = 1, payload = NULL, payload_length = 4 }` block. The test asserts `AUDIO_OUTPUT_SUBMIT_INCORRECT_PAYLOAD_LENGTH` and `AUDIO_OUTPUT_SUBMIT_MISSING_PAYLOAD` respectively, asserts the results unequal, and asserts both counters remain zero.
- Focused validation: `cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix cmocka)" && cmake --build build --target test_audio_output --parallel 2 && ctest --test-dir build --output-on-failure -R '^test_audio_output$'` — configure/build passed and 1/1 focused CTest test passed.
- Changed inventory for S6: modified only `tests/audio_output/test_audio_output.c` with the characterization test and this Track file's S6/A4 evidence and inventory. No production code was modified; S7–S10 remain unchecked.

S7/A5 evidence (approved characterization exception)
- Red phase exception: the null adapter behavior was already satisfied before S7 because the implementation retained only scalar accepted-block and accepted-byte counters and no payload pointer or payload data. No artificial red phase was manufactured.
- Focused green validation: `cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix cmocka)" && cmake --build build --target test_audio_output --parallel 2 && ctest --test-dir build --output-on-failure -R '^test_audio_output$'` — configure and build passed; 1/1 focused CTest test passed. The new test synchronously accepted 4-byte and 8-byte payloads, observed counters 1/4 then 2/12, and observed unchanged counters after mutating every byte of each payload.
- Isolation/no-device evidence: `test_audio_output` is a standalone CMocka target whose CMake link list contains only `cmocka::cmocka`; the focused configure/build/CTest run completed without SDL, CoreAudio, device, file, network, or thread setup.
- Changed inventory for S7: modified only `tests/audio_output/test_audio_output.c` with one registered focused A5 characterization test and this Track file's S7/A5 evidence. No production code, header, CMake, SDL/CoreAudio, device, I/O, allocation, threading, or legacy-path code was modified; S8–S10 remain unchecked.

S8/A6 evidence (structural review, supporting evidence only)
- No code changed for S8; this step is a structural review of the existing tree and CMake configuration and supplements, but does not replace, the automated CTest evidence recorded in S3–S7.
- Co-location: `src/audio_output/` contains only `audio_output.c` and `audio_output.h`, and `tests/audio_output/` contains only `test_audio_output.c`; both are single, matching, owning folders with no files outside them.
- No public header exposure: an `include/` directory does not exist in the repository (retired per ASR-008/ADR-006), and the `SynthTracker` target's `add_executable` source list (`src/main.c`, `src/application.c`, `src/audio.c`, `src/player.c`, `src/tfmx.c`) does not reference `src/audio_output/`; the private component is not wired into the product target and exposes no public API/ABI or public package boundary.
- Standalone CMocka-only test target: in `CMakeLists.txt`, `add_executable(test_audio_output ...)` lists only `tests/audio_output/test_audio_output.c` and `src/audio_output/audio_output.c`, `target_link_libraries(test_audio_output PRIVATE cmocka::cmocka)` is the sole link dependency, and `tests/audio_output/test_audio_output.c` includes only `<stddef.h>`, `<setjmp.h>`, `<stdarg.h>`, `<stdint.h>`, `<cmocka.h>`, and the local private `"audio_output.h"` — no SDL, CoreAudio, or device headers.
- CTest registration: `add_test(NAME test_audio_output COMMAND test_audio_output)` registers the target with CTest, consistent with the S3–S7 focused CTest runs already recorded (`ctest --test-dir build --output-on-failure -R '^test_audio_output$'`).

S9/A7 evidence
- Full validation: `cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix cmocka)"`, `cmake --build build --parallel 2`, and `ctest --test-dir build --output-on-failure` all passed.
- CTest: 6/6 passed — `test_synthtracker_cli_identity`, `tfmx_compile_probe`, `player_compile_probe`, `test_playback_context`, `test_application`, and `test_audio_output`.
- Compatibility evidence: the existing TFMX and player compile probes and the playback-context test passed. With no changes to the legacy emission, routing, or SDL-era playback paths, this run observed no change to those behaviors.
- Status: this is pre-correction baseline evidence only. A1 and A7 were reopened after S10 review found incomplete zero-frame validation and unchecked multiplication; S12 must supply final A7 evidence.

S10/A1 evidence
- Red: `cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix cmocka)" && cmake --build build --target test_audio_output --parallel 2 && ctest --test-dir build --output-on-failure -R '^test_audio_output$'` — focused CTest failed as intended: `zero_frame_validity_depends_only_on_payload_length` failed on the existing non-NULL/zero-length case (`2 != 0`), while the other 4 tests passed.
- Green focused validation: `cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix cmocka)" && cmake --build build --target test_audio_output --parallel 2 && ctest --test-dir build --output-on-failure -R '^test_audio_output$'` — configure/build passed and 1/1 focused CTest test passed; all four zero-frame cases returned the required results and both counters remained zero after each case.
- Changed inventory for S10: replaced the prior zero-frame test in `tests/audio_output/test_audio_output.c` with the focused four-case A1 test; changed only `src/audio_output/audio_output.c` to classify `frame_count == 0` by `payload_length` before existing nonzero handling. No headers, CMake, legacy, SDL/CoreAudio, multiplication/overflow, or S11+ work changed.
- Compatibility: unchanged. The legacy TFMX emission, routing, interpreter/timing, mixing/filtering/stereo, SDL-era playback, and existing platform output paths were not modified; this chunk used focused-only validation and did not run the broader suite.

S11/A8 evidence
- Red: `cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix cmocka)" && cmake --build build --target test_audio_output --parallel 2 && ctest --test-dir build --output-on-failure -R '^test_audio_output$'` — configure/build succeeded and focused CTest failed as intended: the new overflow test's NULL-payload case returned `AUDIO_OUTPUT_SUBMIT_MISSING_PAYLOAD` instead of `AUDIO_OUTPUT_SUBMIT_INCORRECT_PAYLOAD_LENGTH`; the other 5 tests passed.
- Green focused validation: `cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix cmocka)" && cmake --build build --target test_audio_output --parallel 2 && ctest --test-dir build --output-on-failure -R '^test_audio_output$'` — configure/build succeeded and 1/1 focused CTest test passed; both `frame_count = SIZE_MAX / 4 + 1` submissions, with NULL and non-NULL payloads, returned `AUDIO_OUTPUT_SUBMIT_INCORRECT_PAYLOAD_LENGTH`, and both counters remained zero.
- Changed inventory for S11: modified only `tests/audio_output/test_audio_output.c` to register one focused CMocka test that submits both variants before assertions, and `src/audio_output/audio_output.c` to guard nonzero overflowing counts with `frame_count > SIZE_MAX / 4` after zero-frame handling and before payload-nullness classification. The payload multiplication remains after this guard. No headers, CMake, legacy, SDL/CoreAudio, or other files changed for S11.
- Compatibility: unchanged. This private validation correction does not touch TFMX emission, routing, interpreter/timing, mixing/filtering/stereo, SDL-era playback, or existing platform output paths; only the authorized focused validation ran, and S12/full-suite validation remains pending.

S12/A7 evidence (final)
- Full validation: `cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix cmocka)"`, `cmake --build build --parallel 2`, and `ctest --test-dir build --output-on-failure` all succeeded.
- CTest: 6/6 passed — `test_synthtracker_cli_identity`, `tfmx_compile_probe`, `player_compile_probe`, `test_playback_context`, `test_application`, and `test_audio_output`, this time including the S10/A1 four-case zero-frame coverage and the S11/A8 overflow coverage.
- Compile-probe/playback compatibility evidence: `tfmx_compile_probe` and `player_compile_probe` compiled cleanly and `test_playback_context` passed against its fixture-backed bounded playback evidence, alongside `test_application`'s non-root usage/status evidence.
- Compatibility: no observed change to current TFMX emission, routing, or SDL-era playback behavior. `src/audio.c`, `src/player.c`, `src/tfmx.c`, and `src/playback/` remain unmodified by this Track, and no current code path was switched to the new `src/audio_output/` component.
- Status: A7 and S12 are satisfied with this final post-correction run, which supersedes the S9 pre-correction baseline.

S13 evidence
- Final in-scope inventory: this Track, `CMakeLists.txt`, `src/audio_output/audio_output.c`, `src/audio_output/audio_output.h`, and `tests/audio_output/test_audio_output.c`. The component files and test exist in their private co-located folders; CMake defines and registers the standalone CMocka-only `test_audio_output` target. No other file is part of this completion transition.
- Final S12 validation: CTest passed 6/6 — `test_synthtracker_cli_identity`, `tfmx_compile_probe`, `player_compile_probe`, `test_playback_context`, `test_application`, and `test_audio_output` — including the S10/A1 four-case zero-frame and S11/A8 overflow coverage. This is the final A7 evidence and supersedes S9.
- ADR-007, ASR-009, and `docs/ARTIFACTS.md` alignment: the fixed 4-bytes-per-frame format, zero-frame `payload_length == 0` rule, exact representable nonzero length, distinct missing-payload/incorrect-length outcomes, and overflow-before-missing-payload ordering align with the design basis. Deferred ownership, lifetime, timing, ordering, queueing, backpressure, and format-conversion contracts remain unresolved.
- Phase 4 compatibility: C API/ABI, module compatibility/extension, interpreter/timing/audio behavior, persistent DAW format/versioning, and existing platform/audio-output adapters are unchanged. Only the private component/package boundary changes: `src/audio_output/` is added without a public package boundary or legacy-route change.
- The Phase 4 record and canonical index were revised under user approval: Track 012 is delivered, Track 013 is next, and Stage 3 remains in progress.

Current inventory
- Final Track inventory: `CMakeLists.txt`; `src/audio_output/audio_output.c` and `audio_output.h`; and `tests/audio_output/test_audio_output.c`, plus this Track record.
- The private Audio Output component validates the fixed-format Audio Frame Block and provides the synchronous null adapter; its test target links only `cmocka::cmocka` and is registered with CTest.
- The current SDL-era output path and all legacy TFMX/playback sources remain outside this Track's implementation and routing scope.
- Final S12 evidence is 6/6 CTest passing. All A1–A8, M1–M4, and S1–S13 are checked.

Artifacts
- Living roadmap: `SynthTracker modernization roadmap`, Phase 4 — Component extraction; Phase 3 is delivered and Phase 4 is the current execution boundary.
- [`docs/adr/ADR-005-target-daw-component-foundation.md`](../../../docs/adr/ADR-005-target-daw-component-foundation.md) — approved target component foundation and device-independent `Audio Output` direction.
- [`docs/adr/ADR-007-audio-frame-block-boundary-and-fixed-first-format.md`](../../../docs/adr/ADR-007-audio-frame-block-boundary-and-fixed-first-format.md) — Audio Frame Block boundary artifact and fixed first format (committed in `46c971e`).
- [`docs/ASR.md#asr-006--isolated-future-audio-output`](../../../docs/ASR.md) — device-independent Audio Output Port requirement.
- [`docs/ASR.md#asr-009--audio-frame-block-boundary-invariants`](../../../docs/ASR.md) — Audio Frame Block validity and distinct-failure invariants (committed in `46c971e`).
- [`docs/ARTIFACTS.md`](../../../docs/ARTIFACTS.md) — Audio Frame Block artifact catalog, responsibilities, and deferred contract questions (committed in `46c971e`).
- [`docs/ARCHITECTURE.md`](../../../docs/ARCHITECTURE.md) — current system, target architecture, and Phase 4 compatibility policy.
- [`docs/TESTING.md`](../../../docs/TESTING.md) and [`tests/AGENTS.md`](../../../tests/AGENTS.md) — evidence levels, ownership-based test placement, and validation commands.
- [`docs/AGENT_WORKFLOW.md`](../../../docs/AGENT_WORKFLOW.md) — approval, compatibility-impact, TDD, and validation gates.
- [`.backlog/README.md`](../../README.md) and [`.backlog/PORE.md`](../../PORE.md) — Track lifecycle, template, PORE, and implementation gates.

Completion notes
- All acceptance criteria A1–A8 are satisfied with focused CMocka evidence recorded in S3–S7 and S10–S11; A6 is confirmed by the S8 structural review; A7 is satisfied by the final S12 full-suite run (6/6 CTest, no observed compatibility regression), re-confirmed at S13.
- Resolved Q1–Q4 (S2): owning folders `src/audio_output/` and `tests/audio_output/`; private `audio_frame_block` struct and `audio_output_submit_result` enum with `ACCEPTED`/`MISSING_PAYLOAD`/`INCORRECT_PAYLOAD_LENGTH`; private scalar `accepted_block_count`/`accepted_payload_bytes` counters with no retained payload pointer; standalone `test_audio_output` CMocka executable registered with CTest and linked only to `cmocka::cmocka`.
- S13 records the final in-scope inventory, S12 6/6 validation, ADR-007/ASR-009/ARTIFACTS alignment, and compatibility review. All compatibility dimensions are unchanged except the intended private component boundary.
- The Phase 4 record and canonical index were revised under user approval: Track 012 is delivered, Track 013 is next, and Stage 3 remains in progress.
- This Track is COMPLETED. All A1–A8, M1–M4, and S1–S13 are checked.
