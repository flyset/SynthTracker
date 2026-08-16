# TRACK 002 [COMPLETED]: compatibility_safeguards

Track
- ID: TRACK_002
- Repository: TFMX.cpp
- Branch: main
- Current path: .backlog/COMPLETED/2026/TRACK_002_COMPLETED_compatibility_safeguards.md

Problems (PORE)
- P1: As a playback-core maintainer, I need a small observable boundary for loading and rendering a TFMX module, because the current SDL-bound legacy executable does not yet provide a stable engine-level surface for compatibility checks.
- P2: As a compatibility maintainer, I need checked-in, self-authored fixture inputs and an explicit audio layout, because compatibility regressions cannot be reproduced or reviewed reliably from user-local module files and device output alone.

Objective
- Establish the first internal playback-core boundary and reproducible compatibility evidence for one minimal end-to-end playback path, without claiming that Phase 2 safeguards are complete.

Non-negotiables
- Implementation, tests, fixtures, and production changes were permitted only while this Track was ACTIVE and after its Move-to-ACTIVE step was checked.
- All implementation follows TDD: a focused failing test, the smallest passing implementation, then refactoring and validation.
- Preserve the established compatibility floor: existing TFMX modules must load and play correctly; bit-identical playback is not required.
- Keep all TFMX-owned production and test source in C23; no C++ port is planned. Third-party dependency implementation languages remain separately evaluated.
- The first boundary exposes load, start, tick, snapshot, render, and completion operations. It is an internal playback-core boundary, not a public MCP contract.
- Canonical rendered output is 44.1 kHz, signed 16-bit, little-endian, stereo interleaved PCM, using default interpolation and stereo blend settings.
- The fixture is self-authored and checked in as a dual-file binary fixture, accompanied by a human-readable layout specification. Do not check in copied legacy module data or user-local media.
- CMake 3.13 is the minimum supported configuration version for this work.
- CMocka 2.0.2 is the chosen C-native automated test framework and is provided as a system-installed Homebrew package on macOS. CMake config-mode discovery via the Homebrew CMocka prefix, focused-target linking, and CTest registration are implemented. Do not download or vendor CMocka.
- Phase 2 expands compatibility safeguards later; this first milestone must not silently broaden into full format coverage, bit-perfect comparison, or TUI implementation.
- Phase 5 is C23 product readiness: a reusable C playback core and C-based TUI/DAW foundation; this Track remains limited to its Phase 2 compatibility milestone.

Acceptance criteria
- [x] A1) [P1] A focused automated test demonstrates the internal boundary can load the self-authored fixture, start playback, advance by tick, snapshot voice 0, render canonical PCM, and report completion.
- [x] A2) [P1] The voice 0 snapshot exposes and verifies active state, pitch, and volume at the agreed playback point.
- [x] A3) [P1] Rendered canonical PCM is non-silent and conforms to 44.1 kHz signed 16-bit little-endian stereo interleaved layout with default interpolation and blend settings.
- [x] A4) [P1] The engine-level stop/completion path is observable and prevents further playback from being reported as active.
- [x] A5) [P2] The checked-in self-authored dual-file binary fixture and human-readable layout specification reproduce the tested path without external module files.
- [x] A6) [P1, P2] The Track records implementation, TDD, and validation evidence, including any compatibility mismatch or deferred safeguard discovered during execution.

Why now / impact
- Phase 1 established a macOS/Clang C23 compilation baseline. The reusable playback-core seam, initial CMocka test wiring, and deterministic fixture-backed evidence for loading, start, tick, snapshot, render, and completion are now established; final fixture and first-milestone evidence are recorded within this bounded milestone. This milestone makes the next compatibility work executable and reviewable while keeping the broader Phase 2 safeguards bounded.

Scope
- In scope:
  - Define and implement the first internal playback-core boundary for load, start, tick, snapshot, render, and completion.
  - Define the canonical PCM output contract and default interpolation/blend behavior for this boundary.
  - Create a self-authored checked-in dual-file binary fixture and a human-readable layout specification.
  - Add focused automated TDD coverage for voice 0 state, non-silent output, and engine-level stop/completion.
  - Validate the milestone on the established macOS/Clang project boundary.
- Out of scope:
  - Beginning implementation outside the declared ACTIVE-Track plan and TDD gates.
  - Full legacy-module compatibility coverage, bit-identical playback, or a complete format specification.
  - TUI, editing, composition, persistence, public MCP behavior, or endpoint changes.
  - User-local or copied legacy module fixtures and rendered reference audio.
  - The remaining Phase 2 compatibility safeguards beyond this first internal boundary.

Milestones
- [x] M1) Resolve the boundary shape, fixture layout, and canonical PCM contract during planning; keep unresolved design choices in the Decision log.
- [x] M2) With activation complete, execute the first TDD chunk for private playback-context create/destroy lifecycle without fixture loading.
- [x] M3) Complete the coherent TDD chunk for loader separation and fixture loading; defer playback start to later playback-boundary work.
- [x] M4) Complete the render/completion TDD chunk, including non-silent canonical PCM and engine-level stop.
- [x] M5) Validate the dual-file fixture and layout specification, then record the first-milestone evidence and remaining Phase 2 safeguards.

Risks / decisions
- Risk: A minimal self-authored fixture may exercise only a narrow subset of TFMX semantics and must not be presented as broad compatibility evidence.
- Risk: The internal boundary may expose assumptions that require a separate design decision before broader module coverage.
- Decision: The first Phase 2 milestone is the internal playback-core boundary with load, start, tick, snapshot, render, and completion.
- Decision: The canonical audio contract is 44.1 kHz signed 16-bit little-endian stereo interleaved PCM with default interpolation and stereo blend.
- Decision: The first state assertion is voice 0 active/pitch/volume; additional observability is deferred until justified by later safeguards.
- Decision: Engine-level stop/completion is the required termination signal; device-level SDL behavior is not the boundary's acceptance mechanism.
- Decision: Test code may request the limited snapshot after every engine tick; normal playback does not automatically log or emit per-tick reports.
- Decision: No public MCP contract or endpoint changes are in scope, so no public-contract version impact is introduced.
- Decision: CMake 3.13 is the minimum configuration version for this Track.
- Decision: CMocka 2.0.2 is the chosen C-native automated test framework, provided as a system-installed Homebrew package on macOS and discovered in CMake config mode via the Homebrew CMocka prefix. Focused-target linking and CTest registration are implemented. CMocka is not downloaded or vendored.
- Decision: Phase 2 remains broader than this first milestone; later safeguards require explicit scope and evidence rather than being implied by this Track.
- Decision: Automated tests use a component-first layout: `tests/playback/`, `tests/loader/`, `tests/mixer/`, `tests/editor/`, `tests/tui/`, and `tests/integration/`; shared self-authored fixtures belong in `tests/fixtures/`. The first playback-context test is `tests/playback/test_playback_context.c`.
- Decision: The approved first TDD sequencing is to establish and test the private playback-context create/destroy lifecycle without fixture loading; loader separation and fixture load/start follow in a later coherent TDD chunk. The future fixture requirements remain unchanged.
- Decision: The next context-owned loader TDD chunk includes creating the approved self-authored `mdat.step8`/`smpl.step8` fixture pair and its human-readable layout specification, then testing successful separate-file loading with that real fixture data; fixture creation is not deferred to a later plan step.
- Decision: The bounded S6 render/completion chunk fixes the canonical output as 44.1 kHz, signed 16-bit, little-endian, stereo interleaved PCM with default interpolation and stereo blend settings. It extracts a private mixer path from `src/audio.c` that renders into a caller-owned buffer; SDL, an audio device, and `main()` are outside this boundary.
- Decision: For the self-authored fixture, the expected render trace is silent at ticks 1-2 and non-silent at tick 3. Engine completion is expected at tick 29, not tick 15, because prescale delays completion; this is an implementation target pending S6 validation.
- Decision: S6 retains the copied bridge's single-global, non-reentrant limitation; multi-context playback remains out of scope.

Open questions
- [x] Q2) Resolved: after starting subsong 0 and ticking once before each snapshot, tick 1 is `0/0/0`, tick 2 is inactive/`0x06AE`/`0`, ticks 3-4 are active/`0x06AE`/`15`, and tick 5 is inactive/`0x06AE`/`15` (active/pitch/volume).
- [x] Q3) Resolved for this milestone: the agreed remaining Phase 2 compatibility safeguards are recorded below as future explicit scope and do not expand this Track retroactively.

Decision log
- Decision (Q1): The fixture blueprint is self-authored `mdat.step8` and `smpl.step8` files containing one short subsong. Its first trackstep selects a one-note/wait pattern on voice 0, with the other voices inactive. The macro selects the sample, sets nonzero volume, enables sound briefly, and then disables it. The following trackstep stops the engine. The sample file contains two nonzero signed bytes, and `mdat.step8` includes the padding and structural data required for current loader acceptance. Exact tick timing is derived and fixed during the declared TDD work, not guessed during planning.
- Decision (Q2): Test code may request the limited snapshot after every engine tick; normal playback does not automatically log or emit per-tick reports. Resolved by the S5 trace: tick 1 is `0/0/0`, tick 2 is inactive/`0x06AE`/`0`, ticks 3-4 are active/`0x06AE`/`15`, and tick 5 is inactive/`0x06AE`/`15` (active/pitch/volume).
- Decision (Q3): The agreed remaining Phase 2 compatibility safeguards are future explicit scope and do not expand this Track retroactively.

Plan (execution steps)
- [x] S1) Move Track TRACK_002 to ACTIVE (folder, filename, and title status); do not implement before this gate.
- [x] S2) Read the ACTIVE Track, declare the next TDD chunk, inspect the current playback-context seam, and add focused failing coverage at `tests/playback/test_playback_context.c` for private create/destroy lifecycle without fixture loading.
- [x] S3) Implement the smallest passing private playback-context create/destroy lifecycle and validate its focused test.
- [x] S4) In the context-owned loader TDD chunk, create the approved self-authored `mdat.step8`/`smpl.step8` fixture pair and human-readable layout specification, separate the loader seam, and add successful separate-file load coverage using that real fixture data. Playback start remains deferred to the later playback-boundary work.
- [x] S5) Implement the smallest passing tick/snapshot boundary and verify voice 0 active/pitch/volume.
- [x] S5 prerequisite: Add a bounded copied legacy-state start bridge that starts only loaded subsong 0; tick, snapshots, render, completion, SDL/device/main execution, and multi-context playback were deferred at that prerequisite stage.
- [x] S6) Implement the smallest passing SDL-free canonical render and completion/stop behavior: extract a private mixer path from `src/audio.c`, render into a caller-owned buffer using the fixed canonical PCM format, and verify silent ticks 1-2, non-silent tick 3, and engine completion at tick 29 (not tick 15) due to prescale. Do not add SDL/device/`main()` execution; retain the single-global bridge limitation.
- [x] S7) Rerun focused and full CTest validation for the self-authored dual-file binary fixture and human-readable layout specification.
- [x] S8) Record inventory, validation evidence, mismatches, and deferred Phase 2 safeguards; complete only after acceptance is evidenced.

Validation commands / evidence placeholders
- First lifecycle TDD chunk: complete. Changed files:
  - `src/playback_context.h`
  - `src/playback_context.c`
  - `tests/playback/test_playback_context.c`
  - `CMakeLists.txt` — CMocka discovery and SDL-free test-target/CTest wiring.
- Red evidence:
  - `cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix cmocka)"`
  - Result: configure failed because `src/playback_context.c` was absent.
- Green focused test:
  - `cmake -S . -B build -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/cmocka`
  - `cmake --build build --target test_playback_context`
  - `./build/test_playback_context`
  - Result: PASS; the CMocka create/destroy test passed, including `destroy(NULL)`.
- Full CTest:
  - `ctest --test-dir build --output-on-failure`
  - Result: PASS, 3/3 tests.
- Full build using the Homebrew CMocka prefix:
  - `cmake -S . -B build -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/cmocka`
  - `cmake --build build --parallel 2`
  - Result: PASS; all targets built.
- Whitespace validation: `git diff --check` passed with no errors.
- Loader/fixture TDD chunk: complete. Changed files:
  - `CMakeLists.txt` — adds the loader source to the focused CMocka target and passes the source root for fixture lookup.
  - `src/playback_context.h` and `src/playback_context.c` — adds the load-status API, loaded-state query, and transactional context commit.
  - `src/tfmx_loader.h` and `src/tfmx_loader.c` — adds the separate MDAT/SMPL file loader, bounds/structure validation, and candidate disposal.
  - `tests/playback/test_playback_context.c` — adds five focused CMocka cases covering valid, missing/invalid, malformed, semantic-mutation, and transactional loading behavior.
  - `tests/fixtures/mdat.step8` and `tests/fixtures/smpl.step8` — adds the self-authored dual-file binary fixture.
  - `tests/fixtures/step8_layout.md` — documents the fixture offsets, voice-0 binding, note/wait pattern, macro sequence, and sample bytes.
- Loader red evidence:
  - Focused configure/build failed while `src/tfmx_loader.c` was absent from the target.
  - The loaded-state API test failed before `tfmx_playback_context_is_loaded` was declared and implemented.
  - The semantic mutation assertions failed before the loader checked the documented voice-0 trackstep/pattern structure and macro semantics.
- Loader/fixture green focused validation using Homebrew CMocka at `/opt/homebrew/opt/cmocka`:
  - `cmake -S . -B build -DCMAKE_PREFIX_PATH=/opt/homebrew/opt/cmocka` — PASS.
  - `cmake --build build --target test_playback_context` — PASS.
  - `./build/test_playback_context` — PASS, 5/5 tests.
  - `ctest --test-dir build --output-on-failure` — PASS, 3/3 tests.
  - `cmake --build build --parallel 2` — PASS, full build.
  - `git diff --check` — PASS, no errors.
- Loader/fixture coverage: valid separate-file loading; invalid arguments and missing files; invalid format; malformed structure; semantic mutations; transactional preservation of an already loaded pair; voice-0 binding; and the note/wait pattern. The layout review confirms the checked-in dual-file fixture and its documented offsets and byte contents.
- Explicit deferral for the loader/fixture chunk: playback start, tick advancement, voice-0 snapshot, canonical PCM render, SDL output, and engine completion/stop were not implemented or validated by that chunk. Start and tick/snapshot work are now covered by later completed chunks; canonical PCM render, SDL output, and engine completion/stop remain in the unchecked later plan steps.
- Start-bridge TDD prerequisite: complete. Changed files:
  - `CMakeLists.txt` — adds the bridge source to the SDL-free focused CMocka target.
  - `src/playback_context.h` and `src/playback_context.c` — adds start status/API handling for valid, null, unloaded, and unsupported nonzero-subsong requests, with bridge reset on context replacement and destruction.
  - `src/playback_legacy_bridge.h` and `src/playback_legacy_bridge.c` — adds the bounded copied legacy-state bridge for loaded subsong 0 and owns the copied legacy globals/sample buffer.
  - `tests/playback/test_playback_context.c` — adds start success and invalid-state/nonzero-subsong coverage, bringing the focused suite to seven tests.
- Start-bridge red evidence: the focused configure/build failed because `src/playback_legacy_bridge.c` was absent from the target/source set before the bridge implementation was added.
- Start-bridge green and linkage evidence:
  - Focused `test_playback_context` validation passed 7/7.
  - `ctest --test-dir build --output-on-failure` passed 3/3.
  - The full build passed with the Homebrew CMocka prefix `/opt/homebrew/opt/cmocka`.
  - `git diff --check` passed with no errors.
  - The focused test target links CMocka and the copied bridge/player sources without SDL libraries, so this evidence does not execute SDL runtime, an SDL device, or `main()`.
- Start coverage: successful start of loaded subsong 0; invalid null context; valid but unloaded context; and rejection of nonzero subsong. The bridge copies loaded state into the legacy global ownership required by `player.c`, starts with `TfmxInit()` and `StartSong(0, 0)`, and resets that copied ownership between context operations.
- Explicit limitation: the copied global bridge is intentionally non-reentrant and does not support multiple simultaneously active playback contexts; this is deferred. Tick advancement and voice-0 snapshots are implemented for the bounded boundary. Rendering, audio mixing, SDL device behavior, `main()` execution, and engine completion/stop remain unimplemented and unchecked.
- Accepted fixture semantic revision: complete. The self-authored fixture now fixes the ordered pitch/sample/length/volume/DMA macro sequence, a seven-unit pattern wait, and a two-byte nonzero sample payload. The loader tests strengthen semantic and transactional coverage, including a direct sample-length mutation that exceeds the fixture payload bounds.
- Fixture semantic revision red evidence: the focused semantic and transactional assertions failed before the revised fixture contract and corresponding loader checks were in place.
- Fixture semantic revision green evidence: the focused `test_playback_context` suite passed 8/8; `ctest --test-dir build --output-on-failure` passed 3/3; the full build passed; and `git diff --check` passed with no errors.
- Explicit deferral retained: canonical PCM rendering, audio mixing, SDL device behavior, `main()` execution, and engine completion/stop remain unverified or deferred. The five-tick boundary trace is verified, but wall-clock and audio-output timing remain outside this boundary. At that point, the Track remained ACTIVE and its plan was unchanged.
- Tick/snapshot TDD chunk: complete. Changed files:
  - `CMakeLists.txt` — keeps the focused target SDL-free while compiling the bridge and legacy player sources.
  - `src/playback_context.h` and `src/playback_context.c` — add tick status, voice-0 snapshot status/data, and the context tick/snapshot boundary.
  - `src/playback_legacy_bridge.h` and `src/playback_legacy_bridge.c` — add the bounded legacy tick bridge, normalize macro index 0, preserve pattern-before-macro legacy ordering, copy only loader-counted pattern/macro table entries, and convert endianness only for trackstep words.
  - `src/tfmx_loader.h` and `src/tfmx_loader.c` — own normalized metadata and pattern/macro counts and validate the distinct inactive `FE01` through `FE07` bindings.
  - `tests/playback/test_playback_context.c` — adds the five-tick trace and invalid tick/snapshot-state coverage, bringing the focused suite to 11 tests.
  - `tests/fixtures/step8_layout.md` — records the verified five-tick snapshot trace and the legacy-compatible ordering and inactive bindings.
- Tick/snapshot red compatibility evidence: prior direct legacy trace attempts produced snapshot mismatches and segfaults. These were recorded as compatibility mismatches, not suppressed as test failures. Root causes were incorrect macro index normalization, failure to preserve pattern-before-macro legacy ordering, bridge-side assumptions about table extent and endian conversion, and incomplete `FE01` through `FE07` inactive bindings.
- Tick/snapshot corrections: the loader now owns normalized metadata and exact table counts; the bridge copies only those counted table entries, normalizes macro index 0, preserves pattern-before-macro execution ordering, converts only trackstep words from big-endian, and validates all inactive bindings `FE01` through `FE07`.
- Tick/snapshot green validation using Homebrew CMocka at `/opt/homebrew/opt/cmocka`:
  - Focused `test_playback_context` validation — PASS, 11/11 tests.
  - `ctest --test-dir build --output-on-failure` — PASS, 3/3 tests.
  - `cmake --build build --parallel 2` — PASS, full build.
  - `git diff --check` — PASS, no errors.
- Linkage check — PASS, the focused target has no SDL linkage; it does not execute SDL runtime, an SDL device, or `main()`.

- Render/completion TDD chunk: complete. Changed files:
  - `CMakeLists.txt` — adds `src/playback_legacy_mixer.c` to the private SDL-free `test_playback_context` target and retains focused CMocka/CTest wiring.
  - `src/playback_context.h` and `src/playback_context.c` — add caller-owned canonical render, pending-frame handling, and observable engine completion.
  - `src/playback_legacy_mixer.h` and `src/playback_legacy_mixer.c` — add the private fixed-point mixer path, default linear interpolation, stereo blend, frame accounting, and signed-16 little-endian stereo interleaving.
  - `tests/playback/test_playback_context.c` — adds render, invalid/capacity, and completion coverage, bringing the focused suite to 15 tests.
- Render/completion red evidence: the focused configure/build failed because `src/playback_legacy_mixer.c` was absent from the focused target/source set.
- Render/completion green validation using Homebrew CMocka at `/opt/homebrew/opt/cmocka`:
  - Focused `test_playback_context` validation — PASS, 15/15.
  - `ctest --test-dir build --output-on-failure` — PASS, 3/3.
  - `cmake --build build --parallel 2` — PASS, full build.
  - `git diff --check` — PASS, no errors.
  - Linkage check — PASS, the focused target has no SDL runtime linkage and does not execute SDL runtime, an SDL device, or `main()`.
- Canonical render evidence: output is 44.1 kHz, signed 16-bit little-endian stereo interleaved caller-owned PCM with default linear interpolation and stereo blend. Fixture ticks 1-2 render silent output; tick 3 renders 882 frames / 3528 bytes and is non-silent. Invalid context/buffer/byte-count arguments and insufficient output capacity are rejected.
- Completion evidence: after 29 engine ticks the context reports completion and voice 0 is inactive; tick 30 succeeds without reactivating voice 0. This confirms engine completion rather than SDL/device termination.
- Public-contract boundary: unchanged. The mixer and completion behavior remain private internal playback-core functionality; no public MCP contract, endpoint, or runtime API change is introduced.

- S7 validation evidence: Homebrew CMocka resolved at `/opt/homebrew/opt/cmocka`; focused build PASS; focused test 15/15; full CTest 3/3; full build PASS; `git diff --check` PASS; MDAT 640 bytes; SMPL 2 bytes; header/table/trackstep/pattern/macro/sample layout match; focused test no SDL linkage/no SDL undefined symbols while production target links SDL separately.

- First-milestone final evidence: complete. The dual-file fixture and human-readable layout specification were validated by the S7 evidence above, and the implementation, TDD, mismatch, and validation evidence is recorded in this Track. The first milestone is complete, and this Track is now COMPLETED after separate user approval.
- Deferred Phase 2 safeguards (future explicit scope): additional self-authored fixtures for loops, effects, multiple voices, tempo, and malformed modules; broader loader compatibility beyond the narrow dual-file fixture; audio regression references/fingerprints across a fixture corpus; multi-context and reentrant playback; and broader platform validation. These items are not claimed as complete by this milestone and require explicit future scope.
- Public-contract boundary: unchanged. No public MCP contract, endpoint, or runtime API change is introduced.

Validation boundary
- Validation is limited to the macOS/Clang boundary established by Phase 1 unless a new roadmap decision expands platform scope.
- Automated tests are required for the behavior; direct compatibility checks supplement them.
- Implementation and fixture creation follow the ACTIVE-Track TDD gates and declared plan steps.

Current inventory
- Phase 1 completed a macOS/Clang C23 baseline for the legacy C engine and recorded the current SDL 1.2-era API boundary in TRACK_001.
- `src/tfmx.c` currently contains executable entry, argument parsing, loading, and format detection; `src/player.c` contains interpretation and timing; `src/audio.c` contains mixing and SDL-bound output.
- The first lifecycle chunk adds a private opaque playback-context declaration and create/destroy implementation, with focused CMocka coverage in `tests/playback/test_playback_context.c`.
- `CMakeLists.txt` requires CMake 3.13, discovers CMocka in config mode via the Homebrew CMocka prefix, and defines the SDL-free `test_playback_context` CTest target with the loader, private mixer source, and fixture-root definition.
- `src/playback_context.h`/`.c` now expose separate-file load status, loaded-state inspection, and transactional commit; `src/tfmx_loader.h`/`.c` provide the independent MDAT/SMPL candidate loader and structural validation.
- `src/playback_context.h`/`.c` also expose the bounded start status/API; `src/playback_legacy_bridge.h`/`.c` copy loaded data into the legacy global state, start only subsong 0, and reset copied ownership. The bridge is non-reentrant and does not provide multi-context playback.
- `tests/playback/test_playback_context.c` now contains 15 focused tests for valid, missing/invalid, malformed, semantic-mutation, direct length-bound, and transactional loading; valid, null/unloaded, and nonzero-subsong start behavior; tick/snapshot trace and invalid-state behavior; silent/non-silent canonical rendering; invalid/capacity render behavior; and engine completion, including voice-0 binding and note/wait validation through the fixture loader.
- The self-authored dual-file fixture is checked in at `tests/fixtures/mdat.step8` and `tests/fixtures/smpl.step8`, with its human-readable specification at `tests/fixtures/step8_layout.md`; the specification fixes the pitch/sample/length/volume/DMA macro sequence, seven-unit wait, and two-byte sample payload.
- The agreed component-first test layout remains `tests/playback/`, `tests/loader/`, `tests/mixer/`, `tests/editor/`, `tests/tui/`, and `tests/integration/`, with shared self-authored fixtures in `tests/fixtures/`; the initial implemented paths are `tests/playback/test_playback_context.c` and the self-authored fixture files under `tests/fixtures/`.
- `docs/ARCHITECTURE.md` records the intended loader, player-core, and mixer/output decomposition, while the exact loader-to-player seam remains open.
- The bounded loaded-subsong-0 start, five-tick voice-0 tick/snapshot, private canonical mixer, caller-owned PCM render, and engine completion boundary are implemented and validated without SDL runtime linkage. SDL output/device behavior and `main()` execution remain outside this boundary; the copied global bridge is non-reentrant and multi-context playback remains deferred.
- Pre-existing unrelated worktree changes to `AGENTS.md`, `docs/VISION.md`, and `docs/MACRO_DESIGN.md` are separately approved C23 baseline work and are not Track 002 implementation evidence.

Artifacts
- `TRACK_001_COMPLETED_c23_compilation_baseline.md` — Phase 1 compilation and compatibility baseline.
- `docs/ARCHITECTURE.md` — current legacy inventory and agreed loader/player/mixer decomposition.
- Living roadmap: `TFMX.cpp modernization roadmap`, Phase 2 — first playback-core boundary and compatibility safeguards.

Completion notes
- The first lifecycle TDD chunk is complete: the private playback context can be created and destroyed safely, including a null destroy call. Red configure evidence showed the expected missing-`src/playback_context.c` failure; the focused test passed, the full CTest suite passed 3/3, and the full build passed with the Homebrew CMocka prefix `/opt/homebrew/opt/cmocka`. No loader, fixture, playback, rendering, SDL, or completion behavior was added. At that point, this Track remained ACTIVE; the remaining plan covered render/completion, fixture/layout validation, and final first-milestone evidence.
- The loader/fixture TDD chunk is complete: the separate self-authored `mdat.step8`/`smpl.step8` files load through the context-owned loader, invalid and malformed inputs are rejected, semantic mutations are rejected, and a failed replacement preserves the previously loaded pair. The focused executable passed 5/5, CTest passed 3/3, the full build passed, and `git diff --check` passed using the Homebrew CMocka prefix `/opt/homebrew/opt/cmocka`. Voice-0 binding and the note/wait pattern are covered by the loader's fixture validation and layout specification. Start, tick, snapshot, render, and completion were deferred at that point; this Track remained ACTIVE and the remaining plan was unchanged apart from S4 being complete for this bounded loader/fixture chunk.
- The bounded S5 start prerequisite is complete: the copied global bridge starts only loaded subsong 0 and rejects null, unloaded, and nonzero-subsong requests. The focused executable passed 7/7, CTest passed 3/3, the full build passed, and the target links without SDL runtime libraries; `git diff --check` passed. The bridge's non-reentrant, single-global ownership and multi-context limitation are explicit deferrals. Tick/snapshot is now complete under S5; render, audio/SDL device, `main()`, and completion behavior remained deferred, and this Track remained ACTIVE at that point.
- The accepted fixture semantic revision is recorded: the pitch/sample/length/volume/DMA macro sequence, seven-unit wait, and two-byte sample payload are fixed and documented; semantic and transactional loader coverage includes direct length-bound mutation. Focused validation passed 8/8, CTest passed 3/3, the full build passed, and `git diff --check` passed. The later S5 trace resolved tick/snapshot timing at the boundary; canonical render, completion, and PCM output remained deferred. At that point, this Track remained ACTIVE and the plan was unchanged.
- The bounded S6 render/completion chunk is complete: the private mixer renders the canonical caller-owned PCM format, ticks 1-2 are silent, tick 3 produces 882 frames / 3528 bytes of non-silent PCM, invalid and insufficient-capacity requests are rejected, and completion is observable at tick 29 with tick 30 inactive. The focused suite passed 15/15, CTest passed 3/3, the full build passed, `git diff --check` passed, and the focused target has no SDL runtime linkage. S7 fixture/final validation and S8 final evidence are complete; the first milestone is complete, and this Track is now COMPLETED after separate user approval, with no public contract changed.
