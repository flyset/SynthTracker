# TRACK 004 [COMPLETED]: architecture_vision_reconciliation

Track
- ID: TRACK_004
- Repository: TFMX.cpp
- Branch: main
- Current path: .backlog/COMPLETED/2026/TRACK_004_COMPLETED_architecture_vision_reconciliation.md

Problems (PORE)
- P1: As a TFMX.cpp maintainer, I experience conflicting or incomplete descriptions of the product's system, application container, domains, and audio/UI boundaries, because the current README, vision, architecture, macro, glossary, and TUI documents describe different levels of current behavior and future direction.
- P2: As a contributor planning future work, I cannot reliably tell which names describe implemented behavior, which names are target architecture, and which contracts remain open, because terminology and current-state/target-vision boundaries are not normalized across the documentation set.

Objective
- Reconcile the approved GUI-first architecture and product vision into a terminology-consistent documentation set, with explicit current-state versus target-vision distinctions and unresolved contracts preserved as open.

Non-negotiables
- This Track is ACTIVE and documentation-only. Execute only the next declared plan step or coherent documentation-reconciliation chunk.
- Canonical documentation edits are permitted only after the Move-to-ACTIVE step is checked and the user approves the exact documentation-only scope.
- No source, CMake, test, configuration, Track other than this file, durable-memory, or dependency changes are in scope.
- All TFMX-owned production and test source remains C23; no C++ port is planned.
- Do not invent implemented behavior. Current behavior and target vision must be labeled separately.
- Treat the approved SDL GUI platform and intended macOS CoreAudio output adapter as target direction, while preserving the loader-to-domain-to-player contract and CoreAudio integration design as open.
- No public API or public contract changes are authorized by this Track.

Acceptance criteria
- [x] A1) [P1] The Track's ACTIVE work identifies reconciliation evidence for `AGENTS.md`, `README.md`, `docs/VISION.md`, `docs/ARCHITECTURE.md`, `docs/MACRO_DESIGN.md`, `docs/GLOSSARY.md`, and removal of `docs/TUI.md`, and records any implementation/documentation mismatch rather than silently changing implementation.
- [x] A2) [P1, P2] The documentation review uses one normalized product vocabulary: one TFMX.cpp system; one TFMX application container; current transitional application with legacy CLI and SDL-backed audio; target GUI-first application using SDL; Editor; Loader/Writer; shared editable/playable Module Domain Model; distinct Sequencing and Synthesis domains with shared control vocabulary; Playback Engine; Audio Mixer; Audio Output Port; and an intended macOS CoreAudio Adapter.
- [x] A3) [P1, P2] The documentation set clearly labels current transitional behavior, target vision, and unresolved design questions, including that no UI or editing functionality is currently implemented.
- [x] A4) [P1] The documentation preserves the unresolved loader-to-domain-to-player contract and does not select ownership, lifetime, validation, or raw-versus-decoded representation by wording alone.
- [x] A5) [P1, P2] The documentation review proves that the requested reconciliation changes only `AGENTS.md` and the five named canonical documentation files, removes `docs/TUI.md`, preserves internal links and canonical terminology, and makes no source/CMake/test/public-API/CoreAudio/GUI implementation change.
- [x] A6) [P1] This Track completion record includes review evidence for the implementation inventory, terminology decisions, links, examples, formatting, and any mismatch reported to the user.

Why now / impact
- The project needs one durable architectural vocabulary before future playback, DAW, and UI work expands the documentation surface. Reconciling the documents now reduces accidental contract invention while preserving the existing implementation boundary and the open questions carried by Track 003.

Scope
- In scope:
- Plan a documentation-only reconciliation of `AGENTS.md`, `README.md`, `docs/VISION.md`, `docs/ARCHITECTURE.md`, `docs/MACRO_DESIGN.md`, and `docs/GLOSSARY.md`, plus removal of `docs/TUI.md`.
  - Normalize the approved system/application/domain terminology listed in A2.
- Distinguish the current transitional application (legacy CLI and SDL-backed audio) from the target GUI-first application using SDL and an intended macOS CoreAudio output adapter.
  - Describe the shared editable/playable Module Domain Model as target architecture without pretending it is implemented.
  - Preserve distinct Sequencing and Synthesis domains with shared control vocabulary, and distinguish Playback Engine, Audio Mixer, Audio Output Port, and CoreAudio Adapter responsibilities.
  - Preserve and cross-reference the unresolved loader-to-domain-to-player contract.
- Review internal links, terminology, examples, formatting, and implementation/documentation mismatches after removing obsolete TUI references.
- Out of scope:
- Editing canonical documentation outside the next approved documentation-only reconciliation chunk.
  - Source, CMake, test, configuration, dependency, Track, or durable-memory changes.
  - Any implementation, activation, public API change, or public contract change.
  - Deciding the loader-to-domain-to-player contract or raw-versus-decoded model representation.
- Choosing a GUI renderer other than the approved SDL platform, or designing the detailed CoreAudio integration boundary.
- Implementing or integrating CoreAudio, an Audio Output Port, an SDL GUI, an Editor, Sequencing, Synthesis, or the shared Module Domain Model.
- Claiming that the target architecture exists in the current code or that any GUI/editor functionality is implemented.

Milestones
- [x] M1) Complete the current-state and target-vision inventory and terminology comparison while DRAFT/ACTIVE S2.
- [x] M2) Review the proposed normalized vocabulary and unresolved-question wording against Track 003 and the current implementation.
- [x] M3) Obtain approval, move this Track to ACTIVE, and check the Move-to-ACTIVE gate before editing canonical documentation or removing `docs/TUI.md`.
- [x] M4) Apply only the approved documentation reconciliation and validate links, terminology, examples, formatting, and scope.
- [x] M5) Record evidence, mismatches, and roadmap reconciliation status before completion.

Risks / decisions
- Risk: Naming target domains as current components could cause contributors to infer nonexistent APIs or implementation boundaries.
- Risk: Rewording the loader, player, or audio seams could accidentally settle the unresolved loader-to-domain-to-player contract.
- Risk: Conflating SDL's approved GUI-platform role with the intended CoreAudio output-adapter role could obscure their separate future boundaries.
- Risk: The approved document set may contain legacy statements that conflict with current implementation evidence; mismatches must be reported and separately scoped if they require code changes.
- Decision: TFMX.cpp is one system, containing one TFMX application container; “application” is not a second product or separate system.
- Decision: The current application remains transitional: it has a legacy CLI and SDL-backed audio. This is current-state language, not target architecture.
- Decision: The target is a GUI-first application using SDL. The former TUI direction is removed from product planning.
- Decision: Editor, Loader/Writer, shared editable/playable Module Domain Model, distinct Sequencing and Synthesis domains with shared control vocabulary, Playback Engine, Audio Mixer, Audio Output Port, and an intended macOS CoreAudio Adapter are target responsibilities unless explicitly evidenced as current implementation.
- Decision: The loader-to-domain-to-player contract remains unresolved and must be stated as open wherever relevant.
- Decision: No public-contract version impact is introduced because this Track authorizes no public API or runtime change; canonical documentation edits remain approval-gated.

Open questions
- [ ] Q1) What exact contract connects Loader/Writer output through the shared Module Domain Model to the Playback Engine, including ownership, lifetime, validation, and representation? This remains unresolved.
- [x] Q2) Resolved: the legacy CLI, SDL-backed audio, and narrow internal playback seam are current; the GUI, Editor, Module Domain Model, distinct Sequencing/Synthesis boundaries, Audio Output Port, and CoreAudio Adapter remain future/proposed.
- [x] Q3) Resolved: the target GUI-first application uses SDL. Detailed GUI design remains outside this Track.
- [ ] Q4) What is the eventual Audio Output Port/CoreAudio Adapter boundary? CoreAudio is the intended macOS adapter, but its integration design is not part of this Track.
- [x] Q5) Resolved: no identified implementation/documentation mismatch requires a separate implementation Track; the reconciled documents accurately distinguish current and target state.

Decision log
- Decision (Q1): Track 004 records the loader-to-domain-to-player contract as open; it does not choose a model, ownership, lifetime, validation, or decoding contract.
- Decision (Q2): Current-state claims require implementation evidence; target responsibilities are labeled future/proposed and are not presented as implemented.
- Decision (Q3): The target is GUI-first using SDL; the former TUI product direction is removed.
- Decision (Q4): CoreAudio is the intended macOS Audio Output Adapter, but the detailed Audio Output Port/CoreAudio integration boundary remains open and no implementation is authorized.
- Decision (Q5): Any behavior or implementation mismatch discovered during review is reported and does not expand this documentation-only Track.

Plan (execution steps)
- [x] S1) Move Track TRACK_004 to ACTIVE (folder, filename, and title status); obtain approval and do not edit canonical documentation before this gate.
- [x] S2) Re-read the ACTIVE Track, inspect `AGENTS.md`, the five named canonical documents, `docs/TUI.md`, and relevant implementation evidence, and produce a current-state versus target-vision reconciliation matrix.
- [x] S3) Apply only the approved terminology and boundary edits to `AGENTS.md` and the five named canonical documentation files; remove `docs/TUI.md`; preserve open contracts and mark future behavior clearly.
- [x] S4) Review internal links, terminology, examples, formatting, and cross-document consistency; report implementation/documentation mismatches without changing code.
- [x] S5) Run documentation validation, inspect the exact diff, and record changed files and evidence in this Track.
- [x] S6) Complete only after acceptance evidence and roadmap reconciliation status are recorded; do not commit or push as part of this Track.

Current inventory
- `AGENTS.md`, `README.md`, `docs/VISION.md`, `docs/ARCHITECTURE.md`, `docs/MACRO_DESIGN.md`, and `docs/GLOSSARY.md` now distinguish the current legacy CLI with SDL-backed audio from the future C23 GUI-first SDL DAW and intended macOS CoreAudio Adapter.
- `docs/TUI.md` was removed as superseded product-direction documentation. The reserved future test path is `tests/gui/`; no such directory was created.
- `docs/VISION.md` identifies the current playback work as a narrow internal/emerging seam, not a completed reusable playback core or implemented DAW.
- `docs/ARCHITECTURE.md` records target-only responsibilities—Editor, Loader/Writer, Module Domain Model, distinct Sequencing and Synthesis, Playback Engine, Audio Mixer, Audio Output Port, and CoreAudio Adapter—without asserting they are implemented.
- The Loader/Writer → Module Domain Model → Playback Engine contract remains open, including ownership, lifetime, validation, and raw-versus-decoded representation. Detailed SDL GUI and CoreAudio integration remain open.
- `src/tfmx.c` supplies the executable entry point, CLI parsing, loading, and format detection; `src/player.c` supplies legacy sequencing/interpreter behavior; `src/audio.c` supplies SDL-backed mixing/output behavior. No GUI, Editor, Module Domain Model, Audio Output Port, or CoreAudio Adapter is implemented.
- Track 003 remains a DRAFT planning record for playback-seam architecture and is aligned with the approved future SDL GUI/CoreAudio direction.

Artifacts
- [`TRACK_003_DRAFT_playback_seam_architecture.md`](../../DRAFT/2026/TRACK_003_DRAFT_playback_seam_architecture.md) — related DRAFT architecture/seam planning and unresolved-contract record.
- [`README.md`](../../../README.md) — current user-facing status and orientation.
- [`docs/VISION.md`](../../../docs/VISION.md) — current product intent, boundaries, and future direction.
- [`docs/ARCHITECTURE.md`](../../../docs/ARCHITECTURE.md) — current implementation inventory and architecture decisions.
- [`docs/MACRO_DESIGN.md`](../../../docs/MACRO_DESIGN.md) — pre-design macro reference and open representation questions.
- [`docs/GLOSSARY.md`](../../../docs/GLOSSARY.md) — canonical terminology.
- Living roadmap: `TFMX.cpp modernization roadmap`, Phase 3 — architecture clarity; this Track is intended to reconcile durable documentation before later architecture work.
- Living roadmap: `TFMX.cpp modernization roadmap`, Phase 5 — C23 product readiness; the current C23 playback boundary and future C-based DAW direction remain applicable.

Completion notes
- Track activation is complete with user approval. No canonical documentation edit, source change, public API change, roadmap mutation, commit, or push has occurred in this activation step.
- S2 evidence: `src/tfmx.c` remains a CLI entry point with module loading; `src/player.c` remains the legacy interpreter; `src/audio.c` remains SDL-backed audio output. No GUI, Editor, Module Domain Model, Audio Output Port, or CoreAudio Adapter implementation exists. The narrow internal playback seam does not establish the final Loader/Writer → Module Domain Model → Playback Engine contract.
- S3 reconciliation: edited `AGENTS.md`, `README.md`, `docs/VISION.md`, `docs/ARCHITECTURE.md`, `docs/MACRO_DESIGN.md`, and `docs/GLOSSARY.md`; removed `docs/TUI.md`. No source, CMake, test, configuration, dependency, public API, GUI, or CoreAudio implementation changed.
- S4 review corrected four wording defects in `AGENTS.md`, `docs/VISION.md`, `docs/MACRO_DESIGN.md`, and `docs/GLOSSARY.md`: the SDL 1.1.7 historical/current SDL-era distinction; one-system/one-application-container and current-versus-future wording; the open Loader/Writer → Module Domain Model → Playback Engine contract; and residual TUI wording. No implementation/documentation mismatch requires a separate implementation Track.
- S5 validation: `git diff --check` passed. The documentation reconciliation diff contains only the six approved documentation files and the `docs/TUI.md` deletion; separate backlog Track files are present as untracked records. A documentation link check resolved 52 repository references with zero missing paths; Markdown fences were balanced (eight in `README.md`, none in the other changed documents). `markdownlint`, `mdl`, and `vale` are not installed. Terminology, examples, formatting, current-versus-target claims, and open seams were reviewed; no blocker remains.
- Roadmap reconciliation: the user-approved GUI-first SDL/CoreAudio decision had already revised the living roadmap index and Phase 5 before this Track was activated. This documentation outcome introduces no further roadmap change; the roadmap remains current.
- Completion: Track 004 is complete. The documentation-only reconciliation changed `AGENTS.md`, `README.md`, `docs/VISION.md`, `docs/ARCHITECTURE.md`, `docs/MACRO_DESIGN.md`, and `docs/GLOSSARY.md`, and removed `docs/TUI.md`. The established GUI-first SDL direction and intended macOS CoreAudio Adapter are documented as future target architecture; no implementation was added. The roadmap remains current. No commit or push was performed.
