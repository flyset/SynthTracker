# TRACK TRACK_010 [COMPLETED]: private_header_colocation_and_include_retirement

Track
- ID: TRACK_010
- Repository: SynthTracker
- Branch: main
- Current path: .backlog/COMPLETED/2026/TRACK_010_COMPLETED_private_header_colocation_and_include_retirement.md
- Status: COMPLETED

Problems (PORE)
- P1: As a SynthTracker maintainer, I cannot see ownership of the remaining legacy project headers from their source folders, because `include/` still mixes transitional project-owned headers with the project's intended private, ownership-oriented layout.
- P2: As a Phase 4 contributor, I risk adding or retaining project-owned headers in a deprecated location, because the four legacy headers and their build/path references have not yet been migrated and retired as one bounded change.

Objective
- Migrate the four remaining project-owned headers into `src/`, update the necessary private CMake include paths and documentation path references, and retire the project-owned `include/` directory without changing behavior or any public contract.

Non-negotiables
- This Track is planning only while DRAFT; no implementation begins until the Track is moved to ACTIVE and its Move-to-ACTIVE plan step is checked.
- All implementation follows TDD where observable behavior changes: a focused failing test, the smallest passing implementation, then refactoring and validation. This Track intends no behavior change; the existing test suite and executable composition provide regression evidence, while layout inspection is supporting structural evidence only.
- All project-owned production and test headers remain private and co-located by owning folder, without requiring one-to-one source/header basename pairs. Third-party, generated, and platform SDK headers are excluded.
- No behavior, API, ABI, TFMX module, audio, format, or platform change is intended. The source/package layout change is private structural only.
- No test-owned headers currently exist; the migration must not invent a test-header area or add a project-owned header under `include/`.

Acceptance criteria
- [x] A1) [P1, P2] `include/audio.h`, `include/player.h`, `include/tfmx.h`, and `include/tfmxsong.h` are moved into appropriate owning folders under `src/`, with declarations and definitions unchanged except for required include-path spelling or guards.
- [x] A2) [P1, P2] Every project-owned production header is private and co-located in its owning source folder, no project-owned test headers exist or are added, and no project-owned header remains under `include/`; third-party, generated, and platform SDK headers remain out of scope.
- [x] A3) [P2] CMake include directories and target-specific private include paths resolve the relocated headers without retaining a project-owned `include/` dependency; path-reference documentation is updated only where it names the old project-header locations, while legacy-format meaning and citations remain accurate.
- [x] A4) [P1, P2] C23 configure/build, the existing test suite, executable composition, and bounded compatibility evidence pass after migration, with no observed behavior, API/ABI, TFMX, audio, format, or platform change.
- [x] A5) [P1, P2] The `include/` directory is retired after its project-owned contents are migrated, and the final inventory records that only excluded third-party/generated/platform SDK headers, if any, are not governed by this Track.

Why now / impact
- Phase 4 component extraction is in progress after Track 009. Removing the remaining transitional header-location exception now makes ownership visible before further component extraction expands private dependencies or adds new headers.

Scope
- In scope:
  - Move `include/audio.h`, `include/player.h`, `include/tfmx.h`, and `include/tfmxsong.h` into their owning `src/` folder locations.
  - Preserve folder co-location without imposing one-to-one basename pairing.
  - Update CMake include directories and target-specific private include paths required by the moves.
  - Update documentation path references that specifically point to the old project-header locations.
  - Retire the project-owned `include/` directory after migration.
  - Record structural inventory, compatibility impact, and validation evidence in this Track.
- Out of scope:
  - Any behavior, API, ABI, TFMX module or format, interpreter, timing, audio, SDL, platform, or persistent-format change.
  - Any new public header, library-header model, or public package contract.
  - Third-party, generated, or platform SDK headers.
  - Adding test-owned headers; none currently exist.
  - GUI, editor, playback-core redesign, component extraction beyond header ownership, memory, or Git history changes.

Milestones
- [x] M1) Inspect and confirm the complete header ownership/path-reference inventory and resolve destination folders without changing files outside the approved implementation scope.
- [x] M2) Move the four project-owned headers and update only the necessary private CMake include paths and documentation path references.
- [x] M3) Retire `include/` after confirming it contains no project-owned headers and review the exact structural diff.
- [x] M4) Run the compliant validation set and record compatibility evidence and all acceptance results.

Risks / decisions
- Risk: Header relocation can leave hidden include-directory dependencies or stale documentation paths.
- Decision: Use target-specific private include paths and repository-wide path-reference review; do not preserve `include/` as a compatibility location for project-owned headers.
- Risk: Moving declarations could accidentally alter legacy layout, linkage, or type interpretation.
- Decision: Preserve header contents and include relationships unless a path-only adjustment is required, then validate compile/link and existing behavior.
- Decision: No test-owned headers currently exist, so no test-header migration or new test-header placement is part of this Track.
- Version impact: C API/ABI is unchanged because all affected headers are private project-owned headers, no public header/library model is created, and declarations, symbols, calling conventions, and types are not intentionally changed.
- Version impact: Module compatibility/extension is unchanged because TFMX bytes, loader behavior, format detection, module extensions, and legacy data layouts are unchanged; only private header paths move.
- Version impact: Interpreter/timing/audio behavior is unchanged because player, sequencing, timing, mixing, filtering, stereo blending, SDL callbacks, and runtime orchestration are not modified.
- Version impact: Persistent DAW format/versioning is unchanged because no DAW model, serialized data, import/export contract, or version field is introduced.
- Version impact: Platform/audio-output adapter is unchanged because the existing macOS/SDL-era output path and platform scope remain unchanged; no adapter or SDK integration is added.
- Version impact: Component/package boundaries are unchanged as behavioral or public boundaries; only the private source/package layout is structurally co-located, and `include/` is retired.

Open questions
- [x] Q1) Confirmed that all four legacy headers belong in top-level `src/`, not `src/playback`, because top-level legacy executable sources consume them.
- [x] Q2) Confirmed and reconciled the current references in the `AGENTS.md` current-scope line, Architecture current layout, Glossary/MACRO_DESIGN citations, ASR completion status, and this Track; TFMXLegacy citations were reconciled, while completed Track/ADR historical evidence remains unchanged.

Decision log
- Decision (scope): This Track is limited to private header co-location, required CMake/path-reference updates, and `include/` retirement; implementation is authorized only while ACTIVE and executed under approved plan chunks.
- Decision (compatibility): All applicable Phase 4 compatibility dimensions are unchanged for the reasons recorded under Risks / decisions; the source/package layout change is private structural only.
- Decision (test ownership): No test-owned headers currently exist, and none will be introduced by this Track.
- Decision (S2 inventory): All four legacy headers resolve to top-level `src/`, not `src/playback`, because top-level legacy executable sources consume them. The CMake targets previously resolving `include/` now use private `src/` paths, and no target retains an `include/` dependency. All current documentation citations, including TFMXLegacy citations, were reconciled; completed Track/ADR historical evidence remains unchanged.

Plan (execution steps)
- [x] S1) Move Track TRACK_010 to ACTIVE (folder, filename, and title status) after resolving required planning questions and receiving implementation approval.
- [x] S2) Re-read this Track and declare the next unchecked implementation step; capture the final header ownership map, CMake targets, include-resolution references, and documentation path references.
- [x] S3) Move `include/audio.h`, `include/player.h`, `include/tfmx.h`, and `include/tfmxsong.h` into their selected private `src/` owning folders without changing declarations or runtime code.
- [x] S4) Update only the necessary CMake private include directories/target paths and documentation references, then remove the retired project-owned `include/` directory.
- [x] S5) Run compliant validation: C23 configure/build (`cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix cmocka)"` and `cmake --build build --parallel 2`), the existing test suite (`ctest --test-dir build --output-on-failure`), executable composition/compile-link checks, and bounded compatibility evidence for an existing known-valid TFMX module or equivalent current fixture evidence; record results and any mismatch here. On 2026-08-19, CMake configure with the Track command succeeded, `cmake --build build --parallel 2` succeeded, `tfmx`, `test_playback_context`, `test_application`, `tfmx_compile_probe`, and `player_compile_probe` compiled/linked, and CTest passed 4/4 (0 failures) in 0.18 seconds. `test_playback_context` is bounded automated playback compatibility evidence, not format-wide evidence.
- [x] S6) Review the exact changed-file inventory, include resolution, private ownership, documentation paths, compatibility dimensions, and acceptance evidence; complete only after all criteria pass and roadmap reconciliation is recorded.

Current inventory
- `include/audio.h` moved intact to `src/audio.h`; it matches its original `HEAD` include blob and remains a project-owned legacy declaration for signal-stop handling.
- `include/player.h` moved intact to `src/player.h`; it matches its original `HEAD` include blob and remains the project-owned legacy player structures and dependency on `tfmx.h`.
- `include/tfmx.h` moved intact to `src/tfmx.h`; it matches its original `HEAD` include blob and remains the project-owned legacy aliases, union, constants, and global declarations.
- `include/tfmxsong.h` moved intact to `src/tfmxsong.h`; it matches its original `HEAD` include blob and remains the project-owned legacy song header structure and dependency on `tfmx.h`.
- `include/`: removed after all four project-owned headers were moved; no project-owned header remains there.
- `src/application.h`, `src/playback/*.h`: existing private production headers already co-located with owning source folders.
- `tests/**/*.h`: no test-owned headers currently exist.
- `CMakeLists.txt`: all targets now use private `src/` paths; no target retains an `include/` dependency.
- Documentation path references: all current documentation citations, including TFMXLegacy citations, were reconciled; completed Track/ADR historical evidence remains unchanged.
- Runtime/behavior surfaces: `src/player.c`, `src/audio.c`, `src/tfmx.c`, `src/application.c`, and private playback evidence include the affected headers; their behavior is out of scope.
- Validation status: On 2026-08-19, CMake configure/build, executable composition, CTest, and bounded automated playback compatibility validation passed; all acceptance criteria are complete.

Artifacts
- Living roadmap: `SynthTracker modernization roadmap`, Phase 4 — Component extraction; current phase is in progress after Track 009.
- [`docs/adr/ADR-006-private-header-colocation-and-include-retirement.md`](../../../docs/adr/ADR-006-private-header-colocation-and-include-retirement.md) — accepted private-header co-location decision and `include/` retirement.
- [`docs/ASR.md#asr-008--co-located-project-owned-headers-and-include-retirement`](../../../docs/ASR.md#asr-008--co-located-project-owned-headers-and-include-retirement) — co-located-header requirement and verification evidence.
- [`docs/ARCHITECTURE.md`](../../../docs/ARCHITECTURE.md) — current private header layout and approved co-location state.
- [`.backlog/README.md`](../../README.md) and [`.backlog/PORE.md`](../../PORE.md) — Track lifecycle, template, PORE, and implementation gates.
- [`docs/AGENT_WORKFLOW.md`](../../../docs/AGENT_WORKFLOW.md) — state-change approval, compatibility-impact, TDD, and validation gates.

Completion notes
- TRACK_010 is complete. The four headers moved intact to top-level `src/` and match their original `HEAD` include blobs; `include/` is retired; all CMake targets use private `src/` paths; and all current documentation citations, including TFMXLegacy, were reconciled while completed Track/ADR historical evidence remains unchanged.
- S3/S4 path-only implementation evidence is complete. No source behavior, test, memory, or Git history change is part of this Track.
- S5 validation evidence (2026-08-19): CMake configure with the Track command and `cmake --build build --parallel 2` succeeded; `tfmx`, `test_playback_context`, `test_application`, `tfmx_compile_probe`, and `player_compile_probe` compiled/linked; CTest passed 4/4 with 0 failures in 0.18 seconds. The bounded playback fixture evidence passed; `test_playback_context` is not format-wide evidence. `git diff --check` passes.
- All Phase 4 compatibility dimensions remained unchanged for the recorded reasons under Risks / decisions. The Phase 4 living roadmap remains current; no revision is proposed or required.
