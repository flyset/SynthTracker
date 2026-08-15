# TRACK 001 [DRAFT]: c23_compilation_baseline

Track
- ID: TRACK_001
- Repository: TFMX.cpp
- Branch: main
- Current path: .backlog/DRAFT/2026/TRACK_001_DRAFT_c23_compilation_baseline.md

Problems (PORE)
- P1: As a maintainer, I experience no reliable C23 compilation baseline for the legacy engine, because the build is pinned to C99 and the source contains a C23 keyword conflict, a non-standard integer type, and a platform-specific endian include.

Objective
- User goal: Establish a documented, test-first C23 compilation baseline for the existing TFMX C engine without changing playback behavior or implementing the migration in this DRAFT.

Non-negotiables
- All implementation follows TDD: a focused failing test, the smallest passing implementation, then refactoring and validation.
- No implementation begins while this Track is DRAFT; move it to ACTIVE and check S1 before executing implementation steps.
- Preserve existing TFMX module compatibility and runtime semantics; this Track is a compilation-baseline migration, not an engine redesign or C++ conversion.
- Do not add an external dependency solely to hide a portability failure; any endian replacement must preserve the existing byte-order behavior on supported platforms.

Acceptance criteria
- [ ] A1) [P1] The CMake target configures and compiles all existing engine sources as C23 without errors from `export`, `uint`, or `machine/endian.h`.
- [ ] A2) [P1] Focused automated compile/type tests cover the three known blockers and pass under the C23 baseline.
- [ ] A3) [P1] Existing module inspection/playback validation remains available and passes without a documented behavior regression.
- [ ] A4) [P1] The Track records the final C23 configuration, blocker resolutions, test evidence, and any remaining portability limitations.

Why now / impact
- The project is porting a legacy C99/SDL engine toward a maintainable modern core, but the current standard and known source blockers prevent an objective modern-language baseline. Establishing C23 first makes subsequent refactoring failures attributable and keeps compatibility work measurable.

Scope
- In scope:
  - Inventory the CMake, source, header, and existing validation surfaces involved in a C23 baseline.
  - Add focused failing tests or compile probes for the three known blockers before production changes.
  - Make the smallest source and build-definition changes needed for C23 compilation while preserving behavior.
  - Validate compilation, automated tests, and direct legacy-module checks after the Track is ACTIVE.
- Out of scope:
  - Implementing any migration while this Track remains DRAFT.
  - Converting the engine from C to C++, redesigning the loader/player/mixer boundaries, or changing TFMX semantics.
  - Adding TUI, editing, format-extension, or unrelated portability work.

Milestones
- [ ] M1) Complete the C23 blocker inventory and add focused failing compile/type coverage after activation.
- [ ] M2) Apply the smallest passing C23 fixes for the keyword, integer type, and endian-header blockers.
- [ ] M3) Run the baseline and compatibility validations, then record evidence and remaining risks in this Track.

Risks / decisions
- Risk: Renaming the `export` variable must preserve the existing `-x` command-line behavior and export path.
- Risk: Replacing `uint` with an explicit type may expose width or signedness assumptions in file-offset arithmetic; tests must cover those assumptions.
- Risk: Replacing `machine/endian.h` must preserve big-endian and little-endian behavior rather than only fixing macOS compilation.
- Decision: The target baseline is C23 for the existing `.c` sources; C++ migration remains outside this Track.
- Decision: No public MCP contract or endpoint changes are in scope, so no public-contract version impact is introduced.
- Decision: The C23 baseline is validated on macOS with Clang and Linux with GCC.
- Decision: The compatibility target is the SDL 1.2-era API surface used by the legacy engine.
- Decision: User-local Turrican II module pairs are manual validation inputs only; original module files and rendered outputs are not committed.

Open questions
- [x] Q1) Which supported compiler/platform matrix is required for the C23 baseline?
  Resolved: macOS/Clang and Linux/GCC.
- [x] Q2) Which representative module fixtures are available for direct compatibility validation?
  Resolved: user-local Turrican II pairs, used for manual validation only; no originals or renders are committed.

Decision log
- Decision (Q1): The required C23 baseline is macOS/Clang and Linux/GCC.
- Decision (Q2): User-local Turrican II module pairs provide manual compatibility validation only; the repository contains no original module files or rendered outputs.

Plan (execution steps)
- [ ] S1) Move Track TRACK_001 to ACTIVE (folder, filename, and title status).
- [ ] S2) Read the ACTIVE Track, declare the next TDD chunk, and add focused failing compile/type coverage for `export`, `uint`, and `machine/endian.h`.
- [ ] S3) Change the CMake baseline to C23 and implement the smallest passing fixes for the three blockers.
- [ ] S4) Refactor only as needed to clarify the portable types/endian boundary, then rerun the focused tests.
- [ ] S5) Run the validation commands, inspect compatibility evidence, update the inventory and acceptance evidence, and decide whether any remaining issue requires a separate Track.

Validation commands (after ACTIVE)
- `cmake -S . -B build-c23`
- `cmake --build build-c23 --target tfmx`
- `ctest --test-dir build-c23 --output-on-failure`
- `./test.sh` with user-local representative Turrican II module pairs

Validation boundary
- The compilation baseline is checked on macOS/Clang and Linux/GCC.
- SDL compatibility is evaluated against the SDL 1.2-era API surface used by the legacy engine.
- Manual compatibility validation uses user-local Turrican II pairs.
- Original module files and rendered audio outputs remain outside the repository and are not committed.

Current inventory
- `CMakeLists.txt:4-5` sets `CMAKE_C_STANDARD` to `99`; `CMakeLists.txt:31-35` builds `src/audio.c`, `src/player.c`, and `src/tfmx.c` as one executable.
- Known blocker: `src/tfmx.c:78` declares `int export`; `src/tfmx.c:668` and `src/tfmx.c:802` use it. `export` is a C23 keyword.
- Known blocker: `src/tfmx.c:50-52` and `src/tfmx.c:355` use `uint`, which is not a standard C23 type guaranteed by the current includes.
- Known blocker: `src/player.c:5` includes `machine/endian.h`, a non-portable header unavailable on the target macOS baseline.
- `include/tfmx.h` provides project aliases such as `U32`, while `include/tfmxsong.h` also uses native `unsigned short` and `unsigned int`; width assumptions must be reviewed during TDD.
- No CTest test target is declared in `CMakeLists.txt`; `test.sh` is an existing direct check but currently depends on an environment-specific external module path.

Artifacts
- `README.md` — current legacy C/SDL build and compatibility context.
- `docs/VISION.md` — legacy-compatibility-first product boundary.
- `docs/ARCHITECTURE.md` — current three-file C engine inventory and preserved semantic layers.
- Roadmap: `TFMX.cpp modernization roadmap`, Phase 1 — C23 compilation baseline.
- No implementation artifact exists; this DRAFT is the planning record.

Completion notes
- Track created as DRAFT. No implementation, build, test, or activation was performed in this planning step.
