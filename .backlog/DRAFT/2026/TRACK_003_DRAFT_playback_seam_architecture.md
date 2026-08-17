# TRACK 003 [DRAFT]: playback_seam_architecture

Track
- ID: TRACK_003
- Repository: TFMX.cpp
- Branch: main
- Current path: .backlog/DRAFT/2026/TRACK_003_DRAFT_playback_seam_architecture.md

Problems (PORE)
- P1: As a playback-core maintainer, I experience an internal playback seam with unclear ownership and placement, because Track 002 added test-only playback files to the flat `src/` directory without aligning source, build, and test layout to the target component architecture.

Objective
- Characterize and organize the existing test-only playback seam into internal component-aligned source, build, and test placement while preserving all observed behavior and leaving unresolved contracts explicitly unresolved.

Non-negotiables
- This Track is DRAFT: planning and characterization are allowed; implementation is not. No implementation begins until the Track is moved to ACTIVE and its Move-to-ACTIVE step is checked.
- Any later structural change follows TDD: add a focused failing test or structural verification, make the smallest passing change, refactor, and run validation before updating this Track.
- Scope is behavior-preserving organization of the existing test-only seam. No public API, public MCP contract, endpoint, or runtime feature contract is introduced or changed.
- Keep all TFMX-owned production and test source in C23; no C++ port is planned.
- Do not implement GUI UI, editor, synthesis, sequencing, CoreAudio, or any other future application component in this Track.
- The loader-to-domain-to-player contract is an unresolved open question. This Track must not assume or finalize its typed model shape, ownership, or decoding depth.
- Do not expand the fixture corpus or claim broad compatibility from the existing narrow fixture.

Acceptance criteria
- [ ] A1) [P1] The Track records the current seam inventory and maps each existing file and test responsibility to a proposed internal component-aligned placement, including source, build, and test concerns.
- [ ] A2) [P1] A later ACTIVE implementation, if approved, places or organizes the existing seam without changing its observed load, start, tick, snapshot, render, completion, validation, or fixture behavior; focused and full automated evidence proves equivalence.
- [ ] A3) [P1] The resulting internal ownership notes distinguish Loader/Writer, shared Module Domain Model, Sequencing, Synthesis, Playback Engine, Audio Mixer, and Audio Output Port responsibilities without resolving the loader-to-domain-to-player contract.
- [ ] A4) [P1] The Track explicitly records that the target application architecture is future direction only: an SDL-based GUI application, Application Control, Editor, shared Module Domain Model, Loader/Writer, distinct Sequencing and Synthesis domains with shared control vocabulary, Playback Engine, Audio Mixer, Audio Output Port, and a macOS CoreAudio Adapter.
- [ ] A5) [P1] No implementation, GUI/editor/synthesis/sequencing feature, CoreAudio integration, public API change, behavior change, or fixture-corpus expansion is included in the approved scope.

Why now / impact
- Track 002 delivered the first internal playback-core evidence, but its test-only seam is still mixed into the flat legacy source layout. Making ownership and placement explicit now supports Phase 3 architecture clarity and later Phase 4 component extraction without silently turning a test seam into an assumed public or finalized domain contract.

Scope
- In scope:
  - Characterize the existing Track 002 playback seam and its current CMake/test wiring.
  - Define internal component-aligned placement and ownership for the existing seam, with the smallest behavior-preserving structural change reserved for a later ACTIVE implementation.
  - Preserve the current focused fixture-backed behavior and validation surface.
- Record the approved target application architecture as future/proposed context: an SDL-based GUI with a macOS CoreAudio output adapter.
- Out of scope:
  - Implementing any change while this Track is DRAFT.
  - Resolving the loader-to-domain-to-player contract or choosing a raw versus decoded domain representation.
  - Public APIs, MCP behavior, GUI UI, editor, sequencing, synthesis, CoreAudio, audio-device integration, or feature behavior changes.
  - Expanding the fixture corpus, adding compatibility claims, or implementing the broader Phase 2 safeguards.
  - Moving or refactoring the legacy `tfmx.c`, `player.c`, or `audio.c` behavior beyond the approved seam organization.

Milestones
- [ ] M1) Complete the current seam and build/test placement characterization while DRAFT.
- [ ] M2) Define and review the minimal component-aligned organization, ownership notes, and behavior-preservation checks.
- [ ] M3) If approved, activate the Track and execute the structural change through declared TDD steps.
- [ ] M4) Validate unchanged seam behavior and record evidence, risks, and any mismatch without broadening scope.

Risks / decisions
- Risk: Moving files or changing target composition can accidentally alter include visibility, linkage, global-state reset behavior, or the non-reentrant limitation.
- Risk: Names such as “domain,” “player,” or “mixer” could be mistaken for finalized contracts; this Track records responsibilities and placement only.
- Risk: The existing test-only seam validates a narrow self-authored fixture and must not be presented as complete TFMX compatibility coverage.
- Decision: The current seam remains internal and test-only; no public API or public-contract version impact is introduced.
- Decision: Structural organization must preserve the existing behavior and validation boundary, including the single-global/non-reentrant bridge limitation unless a separately approved Track changes it.
- Decision: The target application architecture is future direction, not implementation authorization: an SDL-based GUI → Application Control and Editor surfaces over a shared Module Domain Model; Loader/Writer; distinct Sequencing and Synthesis domains sharing control vocabulary; Playback Engine; Audio Mixer; Audio Output Port; macOS CoreAudio Adapter.
- Decision: Phase 2 compatibility safeguards remain separate from this architecture-clarity Track; this Track does not expand the fixture corpus.

Open questions
- [ ] Q1) What exact contract connects Loader/Writer output through the shared Module Domain Model to the Playback Engine/player, including ownership, lifetime, validation, and raw-versus-decoded representation? This remains unresolved and must not be assumed by this Track.
- [ ] Q2) Which existing seam responsibilities belong to the shared Module Domain Model versus Loader/Writer, Sequencing, Synthesis, Playback Engine, and Audio Mixer after characterization?
- [x] Q3) Resolved: SDL is the future GUI platform and CoreAudio is the intended macOS audio-output adapter. Neither is implemented by this Track.

Decision log
- Decision (Q1): No loader-to-domain-to-player contract is selected. The current `tfmx_loader_candidate` and playback-context coupling are inventory evidence, not a finalized architecture contract.
- Decision (Q2): Placement proposals must be judged by current ownership and behavior, not by introducing new domain behavior or API surface.
- Decision (Q3): The future application architecture is an SDL-based GUI with a macOS CoreAudio output adapter. This remains target direction only; current product behavior remains the existing C23/macOS playback boundary documented in `docs/ARCHITECTURE.md`.

Plan (execution steps)
- [ ] S1) Move Track TRACK_003 to ACTIVE (folder, filename, and title status); do not implement before this gate.
- [ ] S2) Read the ACTIVE Track, declare the next structural TDD chunk, and verify the current seam inventory against `src/`, `tests/playback/`, `tests/fixtures/`, and `CMakeLists.txt`.
- [ ] S3) Add focused failing structural/ownership coverage or an equivalent reviewable check for the approved component-aligned placement, without changing behavior or expanding fixtures.
- [ ] S4) Make the smallest passing source/build/test organization change, preserving the existing internal seam and its non-reentrant limitation.
- [ ] S5) Run focused and full automated validation, inspect include/linkage and fixture behavior, and record any mismatch or deferral.
- [ ] S6) Update acceptance evidence and complete only after the approved structural outcome and roadmap reconciliation are recorded.

Current inventory
- `src/playback_context.h` and `src/playback_context.c` provide the opaque internal context and the load, start, tick, snapshot, render, and completion boundary. The context owns loaded MDAT/SMPL buffers, normalized loader metadata, mixer state, and voice-0 snapshot state.
- `src/tfmx_loader.h` and `src/tfmx_loader.c` read separate MDAT and SMPL files, normalize metadata, validate the narrow self-authored fixture structure, and dispose candidate data. The exact output contract to a domain/player remains open.
- `src/playback_legacy_bridge.h` and `src/playback_legacy_bridge.c` copy the loaded seam into legacy global state, start subsong 0, advance the legacy player, expose a bounded voice-0 trace, and reset global ownership. The bridge is intentionally non-reentrant.
- `src/playback_legacy_mixer.h` and `src/playback_legacy_mixer.c` provide the private SDL-free mixer path, frame accounting, canonical PCM rendering, default interpolation, and stereo blending used by the test seam.
- Legacy references remain `src/tfmx.c` (entry/loading/format detection), `src/player.c` (interpretation/timing), and `src/audio.c` (mixing/SDL output). This Track does not reorganize their behavior.
- `tests/playback/test_playback_context.c` contains 15 focused CMocka tests covering loading, transactional rejection, start, tick/snapshot trace, render, invalid capacity/arguments, and completion.
- `tests/fixtures/mdat.step8`, `tests/fixtures/smpl.step8`, and `tests/fixtures/step8_layout.md` are the existing self-authored dual-file fixture and layout specification. No fixture expansion is planned.
- `CMakeLists.txt` builds `test_playback_context` from the seam sources plus `src/player.c`, registers it with CTest, and keeps this focused target SDL-free; the normal `tfmx` target remains built from the legacy `src/audio.c`, `src/player.c`, and `src/tfmx.c` sources.
- Existing evidence is recorded in `TRACK_002_COMPLETED_compatibility_safeguards.md`: focused test 15/15, CTest 3/3, full build pass, and no SDL runtime linkage for the focused target.

Proposed placement map (planning only)
- `src/playback_context.*` → proposed internal Playback Engine seam/orchestration placement; retain the existing context behavior, not a public API.
- `src/tfmx_loader.*` → proposed Loader/Writer-side placement for the current read/validation helper; do not treat `tfmx_loader_candidate` as the final shared Module Domain Model contract.
- `src/playback_legacy_bridge.*` → proposed internal legacy-player adapter beneath the Playback Engine; preserve its global, non-reentrant limitation until separately addressed.
- `src/playback_legacy_mixer.*` → proposed internal Audio Mixer placement for the SDL-free test mixer; it is not an Audio Output Port or CoreAudio Adapter.
- `src/player.c` → current legacy interpreter reference; Sequencing and Synthesis ownership remains to be characterized rather than split or implemented here.
- `src/audio.c` → current legacy mixer/SDL-output reference; Audio Output Port and CoreAudio Adapter boundaries remain future work.
- `tests/playback/test_playback_context.c` → retained component-aligned playback test placement, with fixture-backed behavior checks unchanged.
- `tests/fixtures/*` → retained shared self-authored fixture placement; no new corpus entries.
- `CMakeLists.txt` → proposed build ownership remains a focused internal playback test target plus the existing legacy executable; any later source-list/path edits require structural TDD and linkage validation.
- No current files implement the shared Module Domain Model, Application Control, Editor, distinct Sequencing/Synthesis domains, SDL GUI, Audio Output Port, or CoreAudio Adapter; these are target architecture responsibilities, not hidden current components.

Artifacts
- [`TRACK_002_COMPLETED_compatibility_safeguards.md`](../../COMPLETED/2026/TRACK_002_COMPLETED_compatibility_safeguards.md) — delivered first internal playback seam, fixture, behavior, and validation evidence.
- [`docs/ARCHITECTURE.md`](../../../docs/ARCHITECTURE.md) — current legacy inventory, agreed loader/player/mixer decomposition, component-first test layout, and open loader-to-player seam.
- Living roadmap: `TFMX.cpp modernization roadmap`, Phase 2 — compatibility safeguards (first milestone delivered by Track 002; deferred safeguards remain explicit).
- Living roadmap: `TFMX.cpp modernization roadmap`, Phase 3 — architecture clarity (clarify responsibilities, data flow, and dependencies to guide safe component boundaries).

Completion notes
- This DRAFT records planning only. No implementation, source/test/build change, activation, roadmap mutation, commit, or push has occurred.
- Validation evidence for any later ACTIVE structural change is intentionally pending. The roadmap remains current unless an approved completed outcome materially changes its phase sequencing, dependencies, intended outcomes, or next major step.
