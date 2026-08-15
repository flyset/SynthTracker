# TRACK 001 [ACTIVE]: c23_compilation_baseline

Track
- ID: TRACK_001
- Repository: TFMX.cpp
- Branch: main
- Current path: .backlog/ACTIVE/2026/TRACK_001_ACTIVE_c23_compilation_baseline.md

Problems (PORE)
- P1: As a maintainer, I need reproducible C23 compilation evidence for the legacy engine across the approved macOS/Clang and Linux/GCC matrix, because C23 probes exposed incompatible legacy function declarations and callback types and the source retains portability-sensitive integer and endian dependencies.

Objective
- User goal: Establish a documented, test-first C23 compilation baseline for the existing TFMX C engine without changing playback behavior or implementing the C++ migration.

Non-negotiables
- All implementation follows TDD: a focused failing test, the smallest passing implementation, then refactoring and validation.
- No implementation begins while this Track is DRAFT; move it to ACTIVE and check S1 before executing implementation steps.
- Preserve existing TFMX module compatibility and runtime semantics; this Track is a compilation-baseline migration, not an engine redesign or C++ conversion.
- Do not add an external dependency solely to hide a portability failure; any endian replacement must preserve the existing byte-order behavior on supported platforms.

Acceptance criteria
- [ ] A1) [P1] The CMake target configures and compiles all existing engine sources as C23 on macOS/Clang and Linux/GCC without C23 declaration or callback-type diagnostics.
- [ ] A2) [P1] Focused automated compile/type tests cover the observed `StartSong` declaration and `Audio.loop` callback blockers and the remaining `uint`/endian portability boundary, and pass under the C23 baseline.
- [ ] A3) [P1] Existing module inspection/playback validation remains available and passes without a documented behavior regression.
- [ ] A4) [P1] The Track records the final C23 configuration, blocker resolutions, test evidence, and any remaining portability limitations.

Why now / impact
- The project is porting a legacy C/SDL engine toward a maintainable modern core, but C23 source blockers and missing Linux/GCC evidence prevent an objective, cross-platform modern-language baseline. Establishing C23 first makes subsequent refactoring failures attributable and keeps compatibility work measurable.

Scope
- In scope:
  - Inventory the CMake, source, header, and existing validation surfaces involved in a C23 baseline.
  - Add focused failing compile probes for the observed C23 blockers before production changes.
  - Make the smallest source and build-definition changes needed for C23 compilation and the supported-platform portability boundary while preserving behavior.
  - Add minimal Linux/GCC CI that exercises the existing SDL 1.2-era discovery, full target build, and CTest probes.
  - Validate compilation, automated tests, and direct legacy-module checks after the Track is ACTIVE.
- Out of scope:
  - Implementing any migration while this Track remains DRAFT.
  - Converting the engine from C to C++, redesigning the loader/player/mixer boundaries, or changing TFMX semantics.
  - Adding TUI, editing, format-extension, or unrelated portability work.

Milestones
- [ ] M1) Complete the C23 blocker inventory and add focused failing compile/type coverage after activation.
- [ ] M2) Apply the smallest passing C23 fixes for the observed declaration and callback blockers, then address remaining supported-platform integer/endian findings.
- [ ] M3) Run the baseline and compatibility validations, then record evidence and remaining risks in this Track.

Risks / decisions
- Risk: Correcting the `StartSong` declaration must preserve its existing two-argument song/mode call contract.
- Risk: Correcting the `Audio.loop` callback contract must preserve `LoopOn`/`LoopOff` DMA-wait and loop behavior; indirect calls currently pass `struct Audio *`.
- Risk: Resolving `uint` portability may expose width or signedness assumptions in file-offset arithmetic; tests must cover those assumptions.
- Risk: Resolving endian-header portability must preserve big-endian and little-endian behavior on both supported platforms.
- Decision: The target baseline is C23 for the existing `.c` sources; C++ migration remains outside this Track.
- Decision: No public MCP contract or endpoint changes are in scope, so no public-contract version impact is introduced.
- Decision: The C23 baseline is validated on macOS with Clang and Linux with GCC.
- Decision: The compatibility target is the SDL 1.2-era API surface used by the legacy engine.
- Decision: User-local Turrican II module pairs are manual validation inputs only; original module files and rendered outputs are not committed.
- Decision: Focused C23 blocker coverage uses CTest/CMake compile probes that compile the affected production translation units; the normal `tfmx` target remains the integration build check.
- Decision: Correct the existing CMake SDL variable mismatch by using the already discovered `SDL_LIBS` value in the existing target link configuration; this is a prerequisite for configuring the C23 probe harness and does not change the SDL API target or add a dependency.
- Decision: AppleClang C23 probes establish that `export` is not a C23 keyword and that the current macOS SDK supplies both `uint` transitively and `machine/endian.h`; retain the latter two as cross-platform portability checks rather than treating them as observed macOS failures.
- Decision: Linux/GCC matrix evidence is provided by a minimal GitHub Actions job on `ubuntu-22.04`, installing `libsdl1.2-dev` so the existing `sdl-config` discovery is exercised; it has no cache or artifact policy.

Open questions
- [x] Q1) Which supported compiler/platform matrix is required for the C23 baseline?
  Resolved: macOS/Clang and Linux/GCC.
- [x] Q2) Which representative module fixtures are available for direct compatibility validation?
  Resolved: user-local Turrican II pairs, used for manual validation only; no originals or renders are committed.

Decision log
- Decision (Q1): The required C23 baseline is macOS/Clang and Linux/GCC.
- Decision (Q2): User-local Turrican II module pairs provide manual compatibility validation only; the repository contains no original module files or rendered outputs.

Plan (execution steps)
- [x] S1) Move Track TRACK_001 to ACTIVE (folder, filename, and title status).
- [x] S2) Read the ACTIVE Track, declare the next TDD chunk, and add focused failing CTest/CMake C23 compile probes for the affected production translation units: `src/tfmx.c` for the `StartSong` declaration and `src/player.c` for the `Audio.loop` callback contract. Correct only the CMake probe wiring and the existing discovered SDL library-variable reference required to configure the harness; do not change production sources in this chunk.
- [ ] S3) Change the CMake baseline to C23 and implement the smallest passing fixes for the observed declaration and callback blockers, then add minimal Linux/GCC CI and resolve any remaining `uint` or endian portability failure exposed by the supported-platform matrix.
- [ ] S4) Refactor only as needed to clarify the portable types/endian boundary, then rerun the focused tests.
- [ ] S5) Run the validation commands, inspect compatibility evidence, update the inventory and acceptance evidence, and decide whether any remaining issue requires a separate Track.

Validation commands (after ACTIVE)
- `cmake -S . -B build-c23`
- `cmake --build build-c23 --target tfmx`
- `ctest --test-dir build-c23 --output-on-failure`
- `./test.sh` with user-local representative Turrican II module pairs
- Linux/GCC CI: configure `build-ci` with GCC, C23, and extensions disabled; build all targets; run CTest.

Validation boundary
- The compilation baseline is checked on macOS/Clang and Linux/GCC.
- SDL compatibility is evaluated against the SDL 1.2-era API surface used by the legacy engine.
- Manual compatibility validation uses user-local Turrican II pairs.
- Original module files and rendered audio outputs remain outside the repository and are not committed.
- Focused automated coverage uses CTest/CMake compile probes for the actual affected production translation units; the normal `tfmx` target remains the integration build check.

Current inventory
- `CMakeLists.txt` sets `CMAKE_C_STANDARD` to `23`, builds `src/audio.c`, `src/player.c`, and `src/tfmx.c` as one executable, and defines two CTest C23 compile probes for `src/tfmx.c` and `src/player.c`.
- Resolved C23 blocker: `StartSong` now consistently has the `(int song, int mode)` contract.
- Resolved C23 blocker: `Audio.loop`, `LoopOn`, and `LoopOff` now consistently use the `struct Audio *` callback contract; `LoopOff` intentionally ignores the parameter.
- Portability check: `src/tfmx.c:50-52,355` use `uint`, which is not a standard C23 type guaranteed by the project includes; it is supplied transitively by the current macOS SDK.
- Portability check: `src/player.c:5` includes `machine/endian.h`; the current macOS SDK supplies it, but its Linux availability remains unverified.
- `export` at `src/tfmx.c:78,668,802` is not a C23 keyword and is not an observed C23 blocker.
- `include/tfmx.h` provides project aliases such as `U32`, while `include/tfmxsong.h` also uses native `unsigned short` and `unsigned int`; width assumptions must be reviewed during TDD.
- CTest registers compile probes for the affected production translation units; `test.sh` remains a direct check that depends on a user-local external module path.
- The CMake SDL discovery populates and links `SDL_LIBS`; the prior undefined `SDL2_LIBRARIES` reference was corrected as a probe-harness prerequisite.
- S2 evidence (macOS/AppleClang 17, C23): `cmake -S . -B build-c23` configures successfully; `tfmx_compile_probe` fails because `StartSong()` at `src/tfmx.c:16` is called with two arguments at line 820; `player_compile_probe` fails because `LoopOn(struct Audio *)` is incompatible with `Audio.loop` at `src/player.c:252`; CTest reports both expected failing probes. The original presumed `export`, `uint`, and `machine/endian.h` failures are not reached on this platform.
- S3 macOS evidence (AppleClang 17, C23): configuration succeeds; both compile probes and CTest pass (2/2); the normal `tfmx` target builds successfully, with a non-blocking duplicate SDL library linker warning.

Artifacts
- `README.md` — current legacy C/SDL build and compatibility context.
- `docs/VISION.md` — legacy-compatibility-first product boundary.
- `docs/ARCHITECTURE.md` — current three-file C engine inventory and preserved semantic layers.
- Roadmap: `TFMX.cpp modernization roadmap`, Phase 1 — C23 compilation baseline.
- `.github/workflows/linux-gcc.yml` — Linux/GCC C23 build and CTest evidence path.

Completion notes
- Track created as DRAFT. No implementation, build, test, or activation was performed in this planning step.
