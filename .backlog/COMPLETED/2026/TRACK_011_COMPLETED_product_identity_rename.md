# TRACK TRACK_011 [COMPLETED]: product_identity_rename

Track
- ID: TRACK_011
- Repository: SynthTracker
- Branch: main
- Current path: .backlog/COMPLETED/2026/TRACK_011_COMPLETED_product_identity_rename.md
- Status: COMPLETED

Problems (PORE)
- P1: As a SynthTracker user, I cannot consistently identify the product I built or launched, because CMake, the executable, and CLI messages still expose the legacy `tfmx` or `tfmxplay` names.
- P2: As a SynthTracker maintainer, I risk documenting or debugging the wrong executable, because build, cleanup, ignore, and IDE references do not agree on the product identity.

Objective
- Make `SynthTracker` the consistent CMake project, executable, and user-facing CLI identity while preserving all legacy TFMX format and playback semantics.

Non-negotiables
- This Track is planning only while DRAFT; no implementation begins until the Track is moved to ACTIVE and its Move-to-ACTIVE plan step is checked.
- All implementation follows TDD: a focused failing automated executable-composition or CLI-identity test, the smallest passing implementation, then refactoring and validation.
- `SynthTracker` is the product identity; TFMX remains the legacy format, module, semantic, and temporary compatibility lineage.
- Preserve existing CLI options, TFMX loading, trackstep, pattern, macro, timing, interpreter, mixing, filtering, and SDL-era audio behavior.
- Do not introduce public C APIs, a library model, persistent DAW formats, new platform support, or an audio-output adapter.

Acceptance criteria
- [x] A1) [P1] A clean CMake configure and build produces an executable named `SynthTracker`; CMake project and executable target identities use `SynthTracker`, not `tfmxplay` or `tfmx`.
- [x] A2) [P1] The executable's help/usage banner and root-user guard identify the product as `SynthTracker`, while preserving all existing options and TFMX module terminology.
- [x] A3) [P2] Current tracked build/run and developer-support references (`README.md`, `.gitignore`, and `clean.sh`) align with the `SynthTracker` executable.
- [x] A4) [P1, P2] A focused automated executable-composition or CLI-identity check fails before the rename and passes after it; the C23 configure/build and existing CTest suite pass.
- [x] A5) [P1] Bounded existing playback compatibility evidence passes with no intended change to TFMX modules, format handling, trackstep, pattern, macro, timing, interpreter, mixing, filtering, or SDL-era audio behavior.

Why now / impact
- Phase 4 component extraction is in progress. Aligning the remaining executable-facing legacy identity with the established SynthTracker product boundary prevents new component work and its validation artifacts from extending obsolete product names.

Scope
- In scope:
  - Rename the CMake project and executable target/output from `tfmxplay`/`tfmx` to `SynthTracker`.
  - Update target-specific CMake references required by the executable-target rename.
  - Update the current CLI usage banner and root-user guard to use the SynthTracker product identity.
  - Update the current executable references in `README.md`, `.gitignore`, and `clean.sh`.
  - Add focused automated evidence for the observable executable identity and record validation and compatibility evidence in this Track.
- Out of scope:
  - Renaming TFMX files, source modules, functions, data structures, test fixtures, compile-probe target names, or the Mnemosyne `synthtracker` namespace.
  - Changing CLI options, exit behavior, TFMX module/file-format behavior, playback semantics, audio behavior, or compatibility fixtures.
  - Rewriting historical legacy documentation or completed Track/ADR evidence.
  - Local ignored `.vscode/launch.json` configuration, which contains a machine-specific module path and is not a tracked project artifact.
  - Public C APIs or ABI, package/component extraction, persistent DAW formats, GUI work, CoreAudio integration, platform changes, memory mutation, or Git history changes.

Milestones
- [x] M1) Confirm the complete executable-identity inventory and select a focused automated observable-identity test at the appropriate integration boundary.
- [x] M2) Rename the CMake project/executable target and align all approved current user-facing and developer-support references.
- [x] M3) Run the focused test, C23 configure/build, CTest suite, executable composition validation, and bounded playback compatibility evidence.

Risks / decisions
- Risk: A target rename can leave CMake target references, generated-output assumptions, cleanup rules, or IDE launch paths stale.
- Decision: Rename every current product executable reference in scope, but retain TFMX identifiers where they denote legacy format, module, semantics, source, or compatibility lineage.
- Risk: Product-identity text can unintentionally alter CLI options or legacy semantic terminology.
- Decision: Change only product/executable identity text; preserve option spelling, exit behavior, and TFMX terms.
- Version impact: C API/ABI is unchanged because no headers, declarations, symbols, types, calling conventions, or public library model are changed.
- Version impact: Module compatibility/extension is unchanged because TFMX bytes, format detection, loader behavior, module extensions, and legacy data layouts are unchanged.
- Version impact: Interpreter/timing/audio behavior is unchanged because sequencing, macros, timing, mixing, filtering, stereo blending, SDL callbacks, and runtime playback behavior are not modified.
- Version impact: Persistent DAW format/versioning is unchanged because no DAW model, serialized data, import/export contract, or version field is introduced.
- Version impact: Platform/audio-output adapter is unchanged because macOS scope and the SDL-era output path remain unchanged; no adapter or SDK integration is added.
- Version impact: Component/package boundaries are unchanged because the executable name and CMake project identity change without extracting, exposing, or reassigning components.
- Compatibility impact: The executable invocation name intentionally changes from `tfmx` to `SynthTracker`; no TFMX compatibility promise is created, and existing TFMX behavior remains temporary Phase 4 scaffold evidence.

Open questions
- [x] Q1) Confirm the target product spelling and broader identity scope. Resolved: the CMake project, executable, and user-facing identity use exact spelling `SynthTracker`.

Decision log
- Decision (scope): Rename the CMake project, executable, and current user-facing/developer-support identity to `SynthTracker`; do not rename legacy TFMX concepts or historical evidence.
- Decision (compatibility): The command name changes intentionally; all TFMX format and playback compatibility dimensions remain unchanged for the recorded reasons.
- Decision (test boundary): Extend the existing application test for usage-banner identity and add a CTest executable-identity check. The CTest check invokes the current executable with `-z` and uses `PASS_REGULAR_EXPRESSION` for the expected `SynthTracker` identity, which permits the expected nonzero exit status; structural source-text inspection alone is insufficient.

Plan (execution steps)
- [x] S1) Move Track TRACK_011 to ACTIVE (folder, filename, and title status) after reviewing the final implementation plan and receiving implementation approval.
- [x] S2) Re-read this Track, state the next unchecked step, and capture the complete target/output, user-facing text, support-reference, and current test inventory.
- [x] S3) Add a focused failing automated executable-composition or CLI-identity test for the selected observable contract. On 2026-08-19, `cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix cmocka)"` and `cmake --build build --parallel 2 --target tfmx test_application` succeeded; the focused CTest selection failed as intended because the application banner lacks `SynthTracker v1.1.7/SDL` and the executable output retains `tfmxplay`/`tfmx-play`.
- [x] S4) Make the smallest CMake, CLI-text, and approved support-reference changes to pass the focused test. On 2026-08-19, CMake configured, `cmake --build build --parallel 2 --target SynthTracker test_application` succeeded, and `ctest --test-dir build -R 'test_application|test_synthtracker_cli_identity' --output-on-failure` passed 2/2.
- [x] S5) Refactor only if needed, then run the focused test, C23 configure/build, CTest suite, executable composition validation, and bounded playback compatibility evidence; record results. On 2026-08-19, a fresh temporary CMake build configured and built successfully with C23; `SynthTracker` exists and the configured project identity is `SynthTracker` version `1.1.7`; CTest passed 5/5; focused `test_synthtracker_cli_identity` and `test_playback_context` passed 2/2; and `git diff --check` passed.
- [x] S6) Review the changed-file inventory, all compatibility dimensions, and acceptance evidence; reconcile the linked roadmap before completion. All acceptance criteria pass; seven tracked implementation/test files are in scope; the six recorded compatibility/version-impact dimensions remain unchanged; and the linked Phase 4 roadmap remains current with no revision required.
- [x] S7) Move TRACK_011 to COMPLETED, synchronizing folder, filename, heading, Current path, and Status; check this step only after acceptance evidence and roadmap reconciliation are recorded.

Current inventory
- `CMakeLists.txt`: `project(SynthTracker VERSION 1.1.7)` and `add_executable(SynthTracker ...)` define the product CMake identity and executable target; its private include path, SDL link declaration, and `test_synthtracker_cli_identity` all reference `SynthTracker`. `tfmx_compile_probe` remains an internal legacy-source compile-probe target and is out of scope.
- `src/application.c` and `src/main.c`: the usage banner and root-user guard now identify `SynthTracker`; CLI options and TFMX module terminology are unchanged.
- `README.md`, `.gitignore`, and `clean.sh`: current build/run, ignored-output, and cleanup references now name `SynthTracker`.
- `.vscode/launch.json`: excluded as ignored local configuration containing a machine-specific module path; it is not a tracked project artifact or part of this completed change.
- `tests/application/test_application.c`: the missing-argument application test now requires `SynthTracker v1.1.7/SDL` and rejects `tfmxplay`, while retaining its existing observable status and usage assertions.
- `test_synthtracker_cli_identity`: the executable-composition CTest invokes `$<TARGET_FILE:SynthTracker> -z`, accepts the expected SynthTracker banner or root guard, and rejects `tfmxplay`/`tfmx-play` output.
- S3 red evidence (2026-08-19): focused CTest failed 2/2 as intended before implementation. S4 focused evidence passed 2/2 after implementation. S5 fresh C23 build and CTest evidence passed 5/5, including `test_synthtracker_cli_identity` and bounded `test_playback_context` compatibility evidence.
- Changed-file review: tracked changes are `CMakeLists.txt`, `src/application.c`, `src/main.c`, `README.md`, `.gitignore`, `clean.sh`, and `tests/application/test_application.c`. Generated build trees and local ignored VS Code configuration remain out of scope.

Artifacts
- Living roadmap: `SynthTracker modernization roadmap`, Phase 4 — Component extraction; it is in progress and this Track aligns the executable-facing product boundary before further extraction.
- [`docs/VISION.md`](../../../docs/VISION.md), [`docs/ARCHITECTURE.md`](../../../docs/ARCHITECTURE.md), and [`docs/GLOSSARY.md`](../../../docs/GLOSSARY.md) — current product/legacy terminology boundary.
- [`docs/AGENT_WORKFLOW.md`](../../../docs/AGENT_WORKFLOW.md) — state-change, compatibility-impact, TDD, and validation gates.
- [`.backlog/README.md`](../../README.md) and [`.backlog/PORE.md`](../../PORE.md) — Track lifecycle, template, PORE, and implementation gates.

Completion notes
- S6 reconciliation: the linked Phase 4 roadmap remains current. This Track changes executable-facing product identity only; it does not alter Phase 4 component-extraction sequencing, dependencies, intended outcomes, or the next major step. No roadmap mutation is required.
- TRACK_011 is complete. The CMake project and executable target, CLI identity, current support references, and focused automated coverage now use `SynthTracker`; TFMX legacy semantics and compatibility behavior remain unchanged.
