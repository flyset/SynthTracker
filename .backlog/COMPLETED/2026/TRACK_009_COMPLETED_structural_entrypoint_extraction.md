# TRACK TRACK_009 [COMPLETED]: structural_entrypoint_extraction

Track
- ID: TRACK_009
- Repository: SynthTracker
- Branch: main
- Current path: .backlog/COMPLETED/2026/TRACK_009_COMPLETED_structural_entrypoint_extraction.md
- Status: COMPLETED

Problems (PORE)
- P1: As a SynthTracker maintainer, I experience the process entrypoint and application orchestration as co-located with legacy TFMX loading code, because `src/tfmx.c` currently owns `main()`, command-line handling, loading, and top-level playback startup in one translation unit.
- P2: As a future DAW component implementer, I cannot establish the approved `Main` to `Application` structural boundary without risking legacy behavior, because no minimal application entrypoint exists while the current orchestration remains embedded in `src/tfmx.c`.

Objective
- Extract the process entrypoint structurally so `src/main.c` is minimal, current orchestration moves from `src/tfmx.c` to `src/application.c`, and the application calls the existing legacy loading functions in `tfmx.c` without changing legacy globals, interfaces, or playback behavior.

Non-negotiables
- Implementation was authorized after this Track moved to ACTIVE and the Move-to-ACTIVE plan step was checked; S1-S6 are complete and the Track is COMPLETED.
- The approved scope is structural-only application-entry extraction.
- `src/main.c` must remain minimal and own only process start, application invocation, and process-status handling appropriate to the existing CLI.
- Current orchestration moves from `src/tfmx.c` to `src/application.c`; legacy loading functions remain in `src/tfmx.c` and are called by the application.
- Legacy globals, interfaces, TFMX loading semantics, interpreter/timing/audio behavior, and playback behavior remain unchanged.
- No new public API, ABI contract, persistent DAW format, platform adapter, shell-execution capability, or unrestricted filesystem interface is introduced.
- The selected internal declaration boundary is private `src/application.h`, shared by `src/main.c`, `src/application.c`, and application-level tests; it is not a public C API or ABI and does not change legacy interfaces.
- The selected first Application contract, for a non-root process, is that with no module argument, `application_run` returns status 2 and writes a `Usage:` marker to stderr; it does not enter the legacy loader or audio setup, and `main` preserves that process status. The selected non-root invalid-option usage path likewise returns status 2 through Application to Main. Existing early `exit` behavior for debug/export paths is preserved; the existing CLI rejects root before argument handling.
- Future implementation must follow TDD: first select an agreed observable application-level contract, then add a focused failing test for it, make it pass with the smallest move, refactor, and validate. The source-text structural test approach was rejected under the project-wide testing strategy.
- Phase 4 compatibility preservation is a temporary development scaffold and not a SynthTracker v1 compatibility promise.

Acceptance criteria
- [x] A1) [P1, P2] The selected Application contract is covered by CMocka application tests and executable integration: in a non-root process, no module and invalid-option cases return status 2 and include a `Usage:` marker; `main` preserves the status, and the no-module path does not enter legacy loading or audio setup. The original no-module test showed the intended red compile failure before production because `src/application.h` was missing; the invalid-option case was added afterward as supplemental regression coverage. Existing debug/export early `exit` behavior and root rejection remain preserved.
- [x] A2) [P1, P2] The final CMake configuration/build compiled and linked `src/main.c`, `src/application.c`, and `src/tfmx.c` into the executable. Full compile/link evidence and an observed non-root `./build/tfmx` run showed status 2, startup text on stdout, and usage text on stderr.
- [x] A3) [P1] Review confirmed that `src/application.c` invokes the existing legacy loading functions in `src/tfmx.c` through unchanged interfaces; loader implementation and legacy global ownership remain in `tfmx.c`.
- [x] A4) [P1, P2] CMocka application coverage, final compile/link validation, 4/4 CTest passing, and a manual known-valid legacy module check provide automated and supplemental evidence for the extracted boundary and retained behavior. The direct module check is bounded to that known-valid module and is not a format-wide compatibility claim; the rejected private `step8` fixture is not regression evidence.
- [x] A5) [P2] Review confirmed a small, source-level-only `Main` to `Application` boundary with no public API/ABI, GUI, editor, reusable public playback API, or unrelated component extraction.

Why now / impact
- Phase 3 is delivered and Phase 4 is the next execution boundary for component extraction. This is the smallest useful structural step toward the approved target architecture: it creates an application coordination seam while avoiding domain redesign. The expected impact is improved maintainability and a clearer foundation for future GUI-first DAW work, with no intended runtime, module, audio, or format change.

Scope
- In scope:
  - Add only the structural `src/main.c` process entrypoint.
  - Add only the structural `src/application.c` orchestration unit.
  - Move current top-level orchestration from `src/tfmx.c` into `src/application.c` with the smallest viable interface arrangement.
  - Use private `src/application.h` for the `application_run` declaration shared by `src/main.c`, `src/application.c`, and application-level tests; this is not a public C API or ABI and does not change legacy interfaces.
  - Keep legacy TFMX loading functions in `src/tfmx.c` and call them from the application.
  - Update the build target only as required to compile and link the extracted units.
   - Add the focused application-level test and validation required by the ACTIVE implementation plan after selecting an agreed observable contract.
- Out of scope:
  - Changing legacy globals, interfaces, loader logic, module formats, or playback behavior.
  - Refactoring the interpreter, timing, mixer, SDL audio path, or audio-output adapter.
  - Creating GUI, editor, model, persistent DAW format, or public reusable playback functionality.
  - General filesystem, shell-execution, dependency, configuration, documentation, ADR, ASR, or durable-memory changes outside the implementation evidence required by an ACTIVE Track.

Milestones
- [x] M1) Resolve the open structural and compatibility questions and move this Track to ACTIVE.
- [x] M2) Select an agreed observable application-level contract, then add and capture its failing focused test before changing production code. The source-text structural test approach is rejected under the project-wide testing strategy.
- [x] M3) Perform the smallest structural move and make the focused application-level test pass.
- [x] M4) Refactor only for clarity without changing legacy interfaces or behavior.
- [x] M5) Run build, application-level, playback, and compatibility validation and record evidence.

Risks / decisions
- Risk: Moving top-level control can accidentally change initialization order, early-exit behavior, or the ordering of loading, audio setup, song start, and teardown.
- Decision: Preserve the existing orchestration order and process-status behavior; only the selected non-root no-module and invalid-option usage paths return status 2 through Application to Main. Preserve existing early `exit` behavior for debug/export paths; the extraction is a move, not a redesign.
- Risk: A new application boundary could be mistaken for a public reusable playback API.
- Decision: Keep the boundary internal and legacy-compatible; no public API or ABI is defined by this Track.
- Risk: Build changes could omit a required legacy translation unit or introduce duplicate symbols.
- Decision: Use compile/link validation and inspect the final target source list before acceptance.
- Version impact: C API/ABI is unchanged because no public header, symbol contract, type, calling convention, or legacy interface is intentionally changed.
- Version impact: Module compatibility/extension is unchanged because TFMX bytes, loader functions, format detection, and module extension rules are out of scope; the application only calls the existing loader.
- Version impact: Interpreter/timing/audio behavior is unchanged because player, audio, sequencing, timing, mixing, filtering, stereo blending, and SDL callback code remain untouched and orchestration order is preserved.
- Version impact: Persistent DAW format/versioning is unchanged because no DAW format, serialized data, model, import/export contract, or version field is introduced.
- Version impact: Platform/audio-output adapter is unchanged because the existing SDL-era output path remains in place; CoreAudio and device-independent adapter work are deferred.
- Version impact: Component/package boundaries change only structurally at the process/application seam; no public package contract or domain component boundary is introduced, and legacy loader ownership remains in `tfmx.c`.

Open questions
- [x] Q1) What is the smallest internal declaration/interface needed for `application.c` to call the existing `tfmx.c` loading and legacy orchestration functions without changing their signatures? Resolved: use private `src/application.h` for the `application_run` declaration, shared by `src/main.c`, `src/application.c`, and application-level tests; it is not a public C API or ABI and does not change legacy interfaces.
- [x] Q2) Which observable application-level contract and test mechanism best prove that the extracted entrypoint preserves the existing process-status behavior and orchestration? For a non-root process, the first contract is the no-module-argument path: `application_run` returns status 2, writes a `Usage:` marker to stderr, does not enter the legacy loader or audio setup, and `main` preserves that process status. The existing CLI rejects root before argument handling.
- [x] Q3) Which direct legacy CLI/module fixture checks are available to evidence unchanged loading and playback after the move? A manual check with one known-valid legacy TFMX module confirmed that the real CLI works after extraction. This is supplemental, module-specific evidence and not a format-wide compatibility claim.

Decision log
- Decision (scope approval): The approved scope is structural-only application-entry extraction: minimal `src/main.c`; current orchestration moves from `src/tfmx.c` to `src/application.c`; the application calls legacy loading functions in `tfmx.c`; legacy globals, interfaces, and playback behavior remain unchanged.
- Decision (compatibility): Treat all six Phase 4 compatibility dimensions as unchanged except for the explicitly structural process/application source boundary, with the reasons recorded under Risks / decisions.
- Decision (lifecycle): Implementation proceeded only after this Track moved to ACTIVE and the Move-to-ACTIVE step was checked; the Track is COMPLETED after acceptance review.
- Decision (first Application contract): For a non-root process, with no module argument, `application_run` returns status 2 and writes a `Usage:` marker to stderr; it does not enter the legacy loader or audio setup, and `main` preserves that process status. The selected non-root invalid-option usage path also returns status 2 through Application to Main. Existing early `exit` behavior for debug/export paths is preserved, and the existing CLI rejects root before argument handling. This is the first focused application-level behavior to drive the structural extraction; the focused failing test completed with the observed red compile failure before production code was added.
- Decision (internal declaration boundary): Use private `src/application.h` for the `application_run` declaration shared by `src/main.c`, `src/application.c`, and application-level tests. This is not a public C API or ABI and does not change legacy interfaces.
- Decision (S5 compatibility evidence): Retain the manual real-CLI check with one known-valid legacy TFMX module as supplemental, module-specific evidence only; it is not a format-wide compatibility claim. The self-authored `step8` fixture remains private playback-seam evidence and was unsuitable for the legacy CLI direct check because the legacy loader rejected it; this is not a regression claim.

Plan (execution steps)
- [x] S1) Move Track TRACK_009 to ACTIVE (folder, filename, and title status).
- [x] S2a) TDD chunk 1 contract selection: select the first observable Application contract for a non-root process: with no module argument, `application_run` returns status 2 and writes a `Usage:` marker to stderr; it does not enter the legacy loader or audio setup, and `main` preserves that process status. The selected non-root invalid-option usage path also returns status 2 through Application to Main. Existing early `exit` behavior for debug/export paths is preserved; the existing CLI rejects root before argument handling.
- [x] S2b) TDD chunk 1 failing test: add the focused automated test for the selected contract and observe it failing before changing production code. The source-text structural test approach is rejected under the project-wide testing strategy.
- [x] S3) TDD chunk 1: make the focused application-level test pass with the smallest structural move by adding minimal `src/main.c`, adding `src/application.c`, moving only current orchestration, and retaining legacy loading functions in `src/tfmx.c`.
- [x] S4) TDD chunk 1: refactor only duplicated or newly exposed internal declarations for clarity, preserving signatures, globals, ordering, and behavior.
- [x] S5) TDD chunk 1: validate the C23 compile/link baseline, focused application-level test, relevant existing automated playback tests, and direct legacy module/CLI checks; record evidence in this Track.
- [x] S6) Update current inventory, acceptance evidence, risks, and completion notes after each meaningful implementation chunk; complete only after all acceptance criteria pass.

Current inventory
- `src/main.c`: minimal process entrypoint; invokes Application and returns its process status.
- `src/application.c`: owns argument parsing, option handling, path derivation, module information output, debug/export branching, audio setup, `TfmxInit`, `StartSong`, signal setup, `play_it`, and `TfmxTakedown`; preserves early `exit` behavior for debug/export paths and returns status 2 for the selected non-root no-module and invalid-option usage paths.
- `src/tfmx.c`: owns the legacy loader functions and legacy globals; Application calls them through unchanged interfaces. Loader/global ownership was unchanged by the extraction.
- `src/player.c`: owns the legacy interpreter and trackstep, pattern, macro, effects, and timing behavior; not in scope for this structural extraction.
- `src/audio.c`: owns legacy mixing, filtering, stereo blending, ring-buffer, SDL callback, and synchronization behavior; not in scope.
- `include/tfmx.h` and `include/tfmxsong.h`: existing legacy types and extern declarations; no public contract change is planned.
- `CMakeLists.txt`: the `tfmx` executable and application-level CMocka target compile/link the extracted entrypoint/application units with the existing legacy units; final CMake configure/build completed successfully.
- `src/playback/`: existing private SDL-free playback seam and legacy bridge tests provide related component evidence but are not part of this entrypoint move.
- `step8` fixture: self-authored fixture remains private playback-seam evidence; the legacy loader rejected it during the direct CLI check, so it is unsuitable for that check and does not establish a regression.
- `tests/application/test_application.c`: CMocka coverage verifies the selected non-root no-module status-2/`Usage` contract; the invalid-option status-2/`Usage` case was added afterward as supplemental regression coverage. The original no-module test observed a red compile failure before production because private `src/application.h` was absent.
- `CMakeLists.txt`: application-level `test_application` registration and final target wiring are present; final CMake configure/build succeeded.
- Validation: final C23 compile/link completed; the observed non-root `./build/tfmx` run returned status 2 with startup stdout and usage stderr; CTest passed 4/4.
- `tests/`: existing playback tests remain available automated behavior evidence; the deleted source-text structural test approach is not valid evidence under the project-wide testing strategy.
- Validation: manual verification after extraction confirmed that the real CLI works with one known-valid legacy TFMX module. This is bounded supplemental, module-specific evidence and not a format-wide compatibility claim. The private `step8` fixture was rejected by the legacy loader and is not a regression claim.
- Review: source inspection confirmed no public API/ABI was introduced, legacy loader/global ownership and interfaces remain unchanged, and the extraction is a source-level-only Main/Application boundary.
- Documentation baseline: `docs/ARCHITECTURE.md` records the implemented `main.c` process entrypoint, `application.c` application orchestration, and `tfmx.c` legacy loader/global ownership; `docs/VISION.md`, ASR-003, ASR-004, and ADR-005 provide the applicable direction and boundary context.

Artifacts
- `docs/ARCHITECTURE.md` — implemented entrypoint ownership and `Main`/`Application` relationship.
- `docs/VISION.md` — Phase 4 component-extraction direction and future GUI-first DAW boundary.
- `docs/AGENT_WORKFLOW.md` — compatibility-impact and TDD gates.
- `.backlog/README.md` and `.backlog/PORE.md` — Track format, lifecycle, PORE traceability, and implementation gates.
- `docs/ASR.md` — ASR-003 UI-agnostic playback core and ASR-004 explicit independently testable component boundaries.
- `docs/ADR.md` and `docs/adr/ADR-005-target-daw-component-foundation.md` — accepted target architectural foundation.
- `MEMORY.md` — Phase 4 project context and roadmap-memory governance; no durable-memory mutation is authorized by this Track creation request.

Completion notes
- This Track is COMPLETED at the path recorded above; S1–S6 are complete and acceptance evidence is already recorded.
- The rejected source-text structural test was removed.
- S2a selected the first Application contract and S2b captured the original focused CMocka no-module test failing at compile time because private `src/application.h` was absent. After production extraction, the no-module and invalid-option non-root cases return status 2 and include `Usage`; invalid-option coverage was added afterward as supplemental regression coverage. Existing debug/export early `exit` behavior and root rejection remain preserved.
- Full compile/link and final CMake configure/build completed successfully; the observed non-root `./build/tfmx` returned status 2 with startup stdout and usage stderr, and CTest passed 4/4. A known-valid legacy module check passed as bounded supplemental evidence. The private `step8` fixture was rejected and is not a regression claim.
- Review evidence confirms implemented `main.c`/`application.c`/`tfmx.c` ownership, no public API/ABI, unchanged loader/global ownership, and a source-level-only boundary. All acceptance criteria A1-A5, milestones M1-M5, and plan steps S3-S6 are complete. The Track is COMPLETED with acceptance evidence already recorded.
