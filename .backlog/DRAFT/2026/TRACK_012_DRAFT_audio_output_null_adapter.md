# TRACK TRACK_012 [DRAFT]: audio_output_null_adapter

Track
- ID: TRACK_012
- Repository: SynthTracker
- Branch: main
- Current path: .backlog/DRAFT/2026/TRACK_012_DRAFT_audio_output_null_adapter.md
- Status: DRAFT

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
- The ASR-009 invariants are required: zero-frame blocks are valid; a nonzero block's payload length must be exactly `frame_count × 4` bytes; missing payload and incorrect payload length must produce distinct failures.
- The null adapter accesses no audio device, opens no SDL/CoreAudio handle, performs no file or network I/O, retains no pointer to caller payload after the call returns, and does not spawn threads.
- Phase 4 compatibility policy applies: preserving current TFMX behavior here is a temporary development scaffold, not a SynthTracker v1 compatibility promise. Current TFMX emission, routing, and SDL-era playback behavior must remain observably unchanged, evidenced by the existing suite.
- Every behavior change requires automated coverage. Structural, layout, or source-text inspection is review support only and never behavioral evidence.

Acceptance criteria
- [ ] A1) [P1, P2] A focused component test proves that a valid Audio Frame Block with `frame_count == 0` is accepted as valid and its submission is an observable no-op: the adapter reports success and records no accepted payload bytes.
- [ ] A2) [P2] A focused component test proves that a valid nonzero block whose payload length is exactly `frame_count × 4` bytes for the fixed format is accepted, at more than one distinct nonzero frame count.
- [ ] A3) [P2] A focused component test proves that a nonzero block with a missing payload is rejected and not accepted by the adapter.
- [ ] A4) [P2] A focused component test proves that a nonzero block whose payload length differs from `frame_count × 4` is rejected, and that this failure is observably distinct from the missing-payload failure of A3 (two different reported outcomes, asserted as unequal).
- [ ] A5) [P1, P3] A focused component test proves the null adapter's observable behavior: it synchronously accepts each valid block, discards it, retains no reference to caller-owned payload memory after return (verified by mutating or invalidating caller memory after the call and observing no adapter-visible effect), and links and runs with no SDL, CoreAudio, or other device dependency in its test target.
- [ ] A6) [P1, P3] The component is private and co-located: its production sources and headers live in one owning `src/` component folder, its tests live in the matching `tests/` component folder, no project-owned header is added outside an owning folder, and no public API/ABI or public package boundary is created.
- [ ] A7) [P1, P2, P3] After implementation, C23 configure/build succeeds and the full CTest suite passes, including the new component tests and the existing TFMX/playback compatibility evidence, with no observed change to current TFMX emission, routing, or SDL-era playback behavior.

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
- [ ] M1) Track is approved and moved to ACTIVE with S1 checked.
- [ ] M2) Fixed-format Audio Frame Block validation satisfies the ASR-009 invariants under focused failing-test-first chunks.
- [ ] M3) The synchronous null adapter satisfies its accept-and-discard, no-retention, and no-device behavior under focused failing-test-first chunks.
- [ ] M4) Private boundary and co-located layout are in place, CMake registers the component test target with CTest, and the full validation set plus existing TFMX compatibility evidence passes.

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
- [ ] Q1) The exact owning folder names for the component and its tests (for example `src/audio_output/` and `tests/audio_output/`), decided at S2 within the co-location rule.
- [ ] Q2) The concrete private C type and validation-result representation carrying frame count, payload pointer, and payload length, decided at S2 as the smallest form satisfying ASR-009's distinct-failure requirement without resolving deferred ownership or error-model contracts.
- [ ] Q3) Whether the null adapter needs any observable counter or last-outcome state for tests beyond the submit result, decided at S2 and kept to the minimum required by A1 and A5.
- [ ] Q4) Whether the new test target is a separate CMocka executable or is added to an existing target; expected to be a separate executable registered with CTest, confirmed at S2.

Decision log
- Decision (gate): No implementation, including new tests, occurs while this Track is DRAFT; S1 is the immediate next step and no implementation plan step is pre-checked.
- Decision (design basis): ADR-005 and ASR-006 authorize the device-independent `Audio Output` direction; ADR-007 and ASR-009 fix the first format and validity invariants; `docs/ARTIFACTS.md` catalogs the Audio Frame Block artifact and its deferred contracts. ADR-007, ASR-009, and `docs/ARTIFACTS.md` were committed in `46c971e` ("Document Audio Frame Block boundary and fixed first format") and are historical committed evidence, not uncommitted working-tree content.
- Decision (format): The fixed first format is 44.1 kHz, stereo, interleaved, signed 16-bit, little-endian PCM, giving 4 bytes per frame; nonzero payload length must equal `frame_count × 4`.
- Decision (adapter behavior): The null adapter synchronously accepts valid blocks and discards them, without retaining payload data, accessing a device, or performing I/O. Concrete implementation and tests are deferred until this Track is ACTIVE.
- Decision (compatibility): Every Phase 4 compatibility dimension is unchanged for the reasons recorded under Risks / decisions, except that component/package boundaries gain a private `Audio Output` component while no public package boundary changes.

Plan (execution steps)
- [ ] S1) Move Track TRACK_012 to ACTIVE (folder, filename, and title status) after planning approval; this is the immediate next step and gates every step below.
- [ ] S2) Re-read this Track, state the next unchecked step, and resolve Q1–Q4: owning folder names, the private block/validation-result representation, minimal adapter observability, and the test-target shape. Record the decisions here before writing any code.
- [ ] S3) TDD chunk — zero-frame validity: failing focused test for A1, smallest passing validation and adapter no-op, refactor, run validation, update this Track.
- [ ] S4) TDD chunk — exact nonzero payload acceptance: failing focused test for A2 at multiple nonzero frame counts, smallest passing change, refactor, run validation, update this Track.
- [ ] S5) TDD chunk — missing payload rejection: failing focused test for A3, smallest passing change, refactor, run validation, update this Track.
- [ ] S6) TDD chunk — wrong payload length rejection distinct from missing payload: failing focused test for A4 asserting the two outcomes are unequal, smallest passing change, refactor, run validation, update this Track.
- [ ] S7) TDD chunk — null adapter accept/discard, no retention, and no device access: failing focused tests for A5, smallest passing change, refactor, run validation, update this Track.
- [ ] S8) Confirm A6: private boundary, co-located production/header and test folders, no public API/ABI or public package boundary, and CMake registration of the component test target with CTest; record the structural review as supporting evidence only.
- [ ] S9) Run the full compliant validation set for A7 (`cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix cmocka)"`, `cmake --build build --parallel 2`, `ctest --test-dir build --output-on-failure`), confirm existing TFMX/playback compatibility evidence still passes, and record results here.
- [ ] S10) Review the exact changed-file inventory and every Phase 4 compatibility dimension against the committed ADR-007/ASR-009/ARTIFACTS design basis (`46c971e`), then move Track TRACK_012 to COMPLETED with acceptance evidence.

Current inventory
- Target `Audio Output` component: none exists. `Audio Output` appears only as approved target vocabulary in `docs/ARCHITECTURE.md` (target architecture and `Mixer` → `Audio Output` relationship), `docs/GLOSSARY.md` (target-only terminology and `Audio Output Port`), ADR-005, and ASR-006. No port, adapter, block type, or validation code exists anywhere in `src/`.
- Audio Frame Block: no implementation. It is defined as target vocabulary in `docs/ARTIFACTS.md`, with its fixed first format in ADR-007 and its validity invariants in ASR-009. ASR-009 status is `Target` with verification recorded as focused component tests when the Audio Output boundary is implemented.
- `src/audio.c` and `src/audio.h`: the only current output path. `src/audio.c` owns mixing, filtering, stereo blending, ring-buffer handling, SDL audio callbacks, and pthread synchronization; `src/audio.h` is a project-owned legacy declaration for signal-stop handling. These responsibilities are relevant legacy context for where output currently lives, and are explicitly out of scope: this Track does not modify, wrap, replace, or route through them.
- Other current production sources: `src/main.c` (minimal entrypoint), `src/application.c` (CLI/lifecycle coordination), `src/tfmx.c` (legacy format/loading), `src/player.c` (interpreter), and the private `src/playback/` seam (`playback_context.c`, `playback_legacy_bridge.c`, `playback_legacy_mixer.c`, `tfmx_loader.c`). All are unmodified by this Track; `src/playback/` remains temporary compatibility evidence, not target architecture.
- Header layout: every project-owned production and test header is private and co-located per ASR-008/ADR-006; `include/` is retired. New component headers must follow the same folder co-location rule.
- Existing test setup: CMake 3.13+ with `include(CTest)`/`enable_testing()` and config-mode CMocka 2.0.2 from Homebrew. Registered tests are `test_synthtracker_cli_identity` (executable CLI identity), `tfmx_compile_probe` and `player_compile_probe` (compile probes), `test_playback_context` (CMocka, `tests/playback/`, fixture-backed bounded playback compatibility evidence with `TFMX_SOURCE_ROOT`), and `test_application` (CMocka, `tests/application/`, bounded non-root usage/status behavior, links `${SDL_LIBS}`). Test folders are `tests/application/`, `tests/playback/`, and `tests/fixtures/`; there is no `tests/audio*` area. Validation commands are recorded in `docs/TESTING.md`.
- Design-basis provenance: `docs/ARTIFACTS.md`, `docs/adr/ADR-007-audio-frame-block-boundary-and-fixed-first-format.md`, and the related updates to `README.md`, `docs/ADR.md`, `docs/ARCHITECTURE.md`, `docs/ASR.md`, and `docs/GLOSSARY.md` were committed in `46c971e` ("Document Audio Frame Block boundary and fixed first format"). This Track's normative basis is therefore committed evidence; S10 reviews the changed-file inventory and compatibility dimensions against that basis before completion.

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
- Not started. This Track is DRAFT and planning only; no implementation, test, CMake, source, documentation, memory, or Git history change has been made under it.
- On completion, record: acceptance evidence per criterion, the resolved Q1–Q4 decisions, the exact changed-file inventory, the validation run output for S9, confirmation that current TFMX emission/routing and SDL-era playback behavior are observably unchanged, the S10 review of the changed-file inventory and compatibility dimensions against the committed ADR-007/ASR-009/ARTIFACTS design basis (`46c971e`), and whether the Phase 4 living roadmap was revised under user approval or remains current.
