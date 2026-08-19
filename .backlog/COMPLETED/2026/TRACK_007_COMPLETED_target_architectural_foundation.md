# TRACK 007 [COMPLETED]: target_architectural_foundation

Track
- ID: TRACK_007
- Repository: TFMX.cpp
- Branch: main
- Current path: .backlog/COMPLETED/2026/TRACK_007_COMPLETED_target_architectural_foundation.md

Problems (PORE)
- P1: As a TFMX developer, I cannot safely evolve the legacy engine toward the GUI-first DAW vision because no approved target architectural foundation defines intended components and their relationships.

Objective
- Define and document the approved target architecture for the TFMX DAW: its components, responsibilities, ownership, dependencies, and principal data flows, without implementation or extraction planning.

Non-negotiables
- No implementation is authorized by this ACTIVE target-only documentation Track.
- Preserve documented TFMX semantics and compatibility requirements for existing modules.
- Playback remains UI-agnostic and usable independently of GUI concerns.
- Filesystem authority stays bounded to explicit file responsibilities; no shell execution or unrestricted filesystem access is introduced.
- Audio output remains device-independent at the target boundary.
- Target boundaries remain small, explicit, and independently testable.
- All future TFMX-owned implementation remains C23 or a later ISO C standard; C++ is not a project direction.

Acceptance criteria
- [x] A1) [P1] The target architecture identifies each component and defines its purpose clearly enough to distinguish it from every other component.
- [x] A2) [P1] The target architecture defines ownership, allowed dependencies, and principal control/data flows without implementation or extraction mechanics.
- [x] A3) [P1] The approved target documentation explicitly evidences alignment with the GUI-first DAW vision, TFMX compatibility requirements, UI-independent playback, bounded file access, and device-independent audio output.
- [x] A4) [P1] The approved foundation is recorded in applicable architecture documentation, ADRs, ASRs, and the glossary, and is clearly distinguished from the current implementation.

Why now / impact
- Phase 3 Architecture clarity is the next step in the living TFMX modernization roadmap. Establishing the target foundation before extraction reduces accidental contracts, god-object growth, and compatibility regressions while giving later component work an approved vocabulary and relationship model.

Scope
- In scope:
  - Define and document the target responsibilities and relationships for Main, Application, UI, Editor, Model, Filesystem, File I/O, Input, Tracker, Synthesizer, Mixer, and Audio Output.
  - Analyze and reconcile ownership, allowed dependencies, principal control flows, and principal data flows.
  - Reconcile target terminology with applicable architecture documentation, ADRs, ASRs, and glossary entries.
  - Record the approved target architecture and its explicit current-versus-target boundary.
- Out of scope:
  - Source code, tests, build changes, or implementation.
  - Extraction or migration sequencing and mechanics.
  - Concrete C API design or implementation.
  - Playback behavior changes or TFMX format changes.
  - Linux or other platform expansion.
  - GUI implementation.

Milestones
- [x] M1) Establish the approved target component vocabulary and current-versus-target inventory.
- [x] M2) Define component responsibilities, ownership, allowed dependencies, and principal control/data flows.
- [x] M3) Record the approved foundation in the applicable architecture docs, ADRs, and glossary without presenting it as implemented.
- [x] M4) Validate acceptance evidence, terminology, links, compatibility constraints, and roadmap reconciliation.

Risks / decisions
- Risk: Existing terms such as `Model`, `Filesystem`, `File I/O`, `Tracker`, `Synthesizer`, and `Audio Output` are ambiguous and may conflate domain responsibilities, protocols, or platform adapters. `Model` retains persistent in-memory `File Information` metadata/reference, including at least path and filename, alongside DAW project data, but has no filesystem authority: it does not browse, list, delete, read, or write filesystem resources. `Filesystem` browses/lists directories, deletes files, and produces `File Information`, but never reads or writes file content. `File I/O` uses `File Information` retrieved by `Application` from `Model` to directly read/write content and translate it to/from `Model` data; it owns bounded format import/loading, save/export, and rendered-audio export.
- Risk: `Application` could become a god-object that owns UI, editing, playback, persistence, and platform behavior instead of coordinating bounded responsibilities.
- Risk: Target names could be mistaken for implemented components, public APIs, or extraction instructions unless every target statement is clearly labeled.
- Risk: Defining flows too concretely could accidentally settle unresolved ownership, lifetime, representation, or compatibility contracts.
- Decision: The approved-for-now component list is a starting architecture, not immutable. Names and boundaries may change if Track analysis justifies the change.
- Decision: This documentation Track does not change the implemented C API/ABI, module format or compatibility, interpreter/timing/audio behavior, persistent DAW format, platform adapter behavior, or current package boundaries.
- Decision: The Track may define approved target boundaries, but those boundaries must remain clearly labeled as target and must not be presented as current implementation.
- Version impact: No public C API/ABI, module-format, interpreter/timing/audio, persistent-DAW-format, platform-adapter, or current package-boundary change is authorized by this ACTIVE target-only documentation Track; each remains unchanged because the Track records target architecture only.

Open questions
- [x] Q1) What precise responsibilities and terminology distinguish `Model`, `Filesystem`, and `File I/O`, including `Model` retaining persistent in-memory `File Information` metadata/reference without filesystem authority; `Filesystem` browsing/listing directories, deleting files, and producing `File Information` without reading or writing content; and `File I/O` directly reading/writing content, translating it to/from `Model` data, and owning bounded format import/loading, save/export, and rendered-audio export?
- [x] Q2) What precise responsibilities distinguish `Tracker` from `Editor`, `Model`, and `Synthesizer` without implying a general-purpose tracker or sample editor?
- [x] Q3) What precise target boundary distinguishes device-independent `Audio Output` from the future platform-specific adapter?
- [x] Q4) What coordination responsibilities belong to `Application` without making it a god-object?
- [x] Q5) Which ownership and lifetime decisions must remain open until a later contract or implementation Track?

Decision log
- Decision (starting set): Main, Application, UI, Editor, Model, Filesystem, File I/O, Input, Tracker, Synthesizer, Mixer, and Audio Output are the approved starting architecture for analysis; the list is not immutable and may be revised with recorded justification.
- Decision (principal relationships): Documented principal permitted relationships are target-only and non-exhaustive; detailed contracts and additional relationships remain deferred to later Tracks.
- Decision (File split): `Model` retains persistent in-memory `File Information` metadata/reference, including at least path and filename, alongside DAW project data, but has no filesystem authority and does not browse, list, delete, read, or write filesystem resources. `Filesystem` owns bounded directory browsing/listing and file deletion and produces `File Information`; it never reads or writes file content. `File I/O` uses `File Information` retrieved by `Application` from `Model` to directly read/write content and translate it to/from `Model` data. It owns bounded format import/loading, save/export, and rendered-audio export. S4 refined ASR-007 to record the target `Filesystem`/`File I/O`/`Model` least-authority split, while remaining target-only/not implemented and preserving the no-shell/no-unrestricted-filesystem guardrail.
- Decision (load flow): At the target level, `UI` requests a load through `Application`; `Filesystem` returns `File Information` to `Application`; `Application` stores it in `Model`, later retrieves it from `Model`, and invokes `File I/O`; and `File I/O` loads content into `Model`. This records the intended flow without inventing APIs or concrete data types.
- Decision (Model): `Model` is the authoritative DAW-wide project data that `Editor` changes, distinct from a possible future `Module Domain Model`.
- Decision (Editor/UI): `Editor` owns edit commands and undo/redo history. `UI` reads and observes `Model` state for presentation; UI mutations flow `UI` → `Application` → `Editor` → `Model`.
- Decision (Tracker): `Tracker` reads song data from `Model`, produces timed musical events, and routes recorded input into `Editor`. It is an internal TFMX song sequencing/recording component, not a general-purpose tracker product.
- Decision (Synthesizer): `Synthesizer` reads model configuration records that describe which Synthesizer engine instances are active, contains multiple independent engine instances that may be rendered simultaneously, and has each instance produce an audio stream for `Mixer`. `Tracker` and `Input` events identify their target configured engine instance. These instances are not threads or channels; scheduling strategy, including one real-time thread, a pool, or dedicated threads, remains explicitly deferred under the existing S3 boundary. `Synthesizer` is not the DAW product category.
- Decision (Mixer): `Mixer` receives `Synthesizer` audio streams, reads level, pan, and mix settings from `Model`, and produces audio frames.
- Decision (audio routing): For audio export, `Mixer` audio frames route to `File I/O` for writing; `Audio Output` receives frames for audible playback. The approved `File I/O`/`Filesystem` boundary remains unchanged.
- Decision (Audio Output): `Audio Output` means a device-independent output port. CoreAudio and future platform adapters remain implementation details, not top-level components.
- Decision (Input): `Input` means musical performance input only: MIDI, computer-keyboard notes, and controllers. Normal UI input remains the UI responsibility.
- Decision (Main): `Main` owns one `Application` instance and performs only process lifecycle: start, run, stop, and return process status.
- Decision (Application): `Application` owns top-level components and coordinates their lifecycle. UI actions flow through `Application` to relevant components; `Application` delegates all domain work and does not implement interpretation, synthesis, mixing, persistence, or filesystem operations.
- Decision (S3 boundary): This Track decides high-level component ownership/lifecycle and the roles, dependency, and data-flow relationships already recorded: `Application` owns top-level component lifecycles, `Model` owns authoritative in-memory project data and `File Information`, and each component owns its stated domain responsibility. Concrete object-lifetime mechanics, raw-versus-decoded representation, validation rules, the exact `File I/O → Model → Playback Engine` contract, including a possible relationship to a future `Module Domain Model`, C interfaces/APIs, threading/real-time scheduling mechanics, and implementation/package extraction remain deferred to later contract or implementation Tracks.
- Decision (real-time routes): `Application` configures real-time routes, but musical performance events do not travel through it: `Input` routes directly to `Synthesizer` for audition and, when recording, to `Tracker`, then `Editor`, then `Model`.
- Decision (Playback Engine): `Playback Engine` is a subsystem/emergent grouping of `Tracker`, `Synthesizer`, and `Mixer`, not an additional top-level component.
- Decision (documentation boundary): This Track defines approved target boundaries only. It does not change the implemented C API/ABI, module format/compatibility, interpreter/timing/audio behavior, persistent DAW format, platform adapter behavior, or current package boundaries. No public or implemented contract changes were made.
- Decision (current-versus-target): Current monolithic CLI behavior and the private playback seam remain inventory evidence; target components and flows must not be described as implemented.
- Decision (roadmap): This Track is derived from the living `TFMX.cpp modernization roadmap`, Phase 3 Architecture clarity. No roadmap mutation was authorized or approved by this completed target-only Track.
- S4 evidence: Approved target-only architecture is recorded in `docs/ARCHITECTURE.md`; ADR-005 is accepted and indexed in `docs/ADR.md`; ASR-007 records the `Filesystem`/`File I/O`/`Model` least-authority split without weakening the no-shell or unrestricted-filesystem guardrail; target terminology is recorded in `docs/GLOSSARY.md` while preserving legacy `Channel` meaning and target-only distinctions; and `docs/MACRO_DESIGN.md` uses the open `File I/O → Model → Playback Engine` wording instead of obsolete provisional contract wording. `README.md` and `docs/VISION.md` intentionally remain unchanged. Internal links, target terminology coverage, and `git diff --check` passed; S5 final acceptance validation passed A1-A4. All changes are target-only documentation evidence and do not authorize implementation or extraction or change implemented behavior or contracts.
- S5 evidence: Final read-only review passed A1-A4. All internal Markdown links resolve; ADR-005 indexing/status and ASR cross-references are coherent; current-versus-target distinctions and terminology are consistent; C23, existing-module compatibility, UI-agnostic playback, one playback context, Filesystem/File I/O least authority, and device-independent Audio Output are preserved. `README.md` and `docs/VISION.md` remain unchanged and aligned. All changed paths are approved scope, and tracked/untracked whitespace checks passed. No implementation or public contract changed.

Plan (execution steps)
- [x] S1) Move Track 007 to ACTIVE (folder, filename, and title status).
- [x] S2) Validate the current starting component set and reconcile terminology against the current implementation, `docs/VISION.md`, `docs/ARCHITECTURE.md`, `docs/ASR.md`, applicable ADRs, and `docs/GLOSSARY.md`.
- [x] S3) Define and finalize target roles, ownership, allowed dependencies, and principal control/data flows, preserving explicit current-versus-target distinctions and unresolved contracts.
- [x] S4) Record and approve the architecture decisions and target documentation in the applicable architecture docs, ADRs, and glossary; do not add extraction work.
- [x] S5) Validate documentation links, terminology, acceptance traceability, compatibility constraints, and the distinction from current implementation.
- [x] S6) Reconcile the Phase 3 Architecture clarity roadmap outcome and complete Track 007 only after all acceptance evidence and completion notes are recorded.

Current inventory
- The current product is a monolithic legacy CLI with SDL-backed audio in one executable.
- `src/tfmx.c` owns `main()`, CLI argument parsing, file loading, and byte-sniffing format detection.
- `src/player.c` owns legacy interpretation, including trackstep to pattern to macro sequencing, macro execution, timing, and effects.
- `src/audio.c` owns mixing, filtering, stereo blending, ring-buffer handling, SDL audio callbacks, and synchronization.
- Runtime state is shared global state; loaded module data is mutated in place, and per-song behavior hacks remain global.
- The current private playback seam is a narrow SDL-free, fixed-eight-voice, single-global, non-reentrant playback seam; it is not a public API or finalized target contract.
- Existing governance and reference material includes `docs/VISION.md`, `docs/ARCHITECTURE.md`, `docs/ASR.md`, `docs/ADR.md` and applicable ADRs, `docs/GLOSSARY.md`, `docs/AGENT_WORKFLOW.md`, `MEMORY.md`, and `.backlog/` Track governance.
- S2 validation evidence (read-only): Reviewed `README.md`, `docs/VISION.md`, `docs/ARCHITECTURE.md`, `docs/ASR.md`, the `docs/ADR.md` index and ADR records, `docs/GLOSSARY.md`, this active Track, current legacy sources/headers, CMake test wiring, and the private playback seam. Current `src/tfmx.c`, `src/player.c`, and `src/audio.c` are monolithic legacy CLI/interpreter/audio behavior; `src/playback/` is private, single-global, non-reentrant, SDL-free test evidence only. No target components are implemented. No public or implemented contract changes were made.

Artifacts
- [`docs/VISION.md`](../../../docs/VISION.md) — GUI-first DAW vision, TFMX compatibility, UI-agnostic playback, and target boundaries.
- [`docs/ARCHITECTURE.md`](../../../docs/ARCHITECTURE.md) — approved target architecture and current implementation inventory.
- [`docs/ASR.md`](../../../docs/ASR.md) — compatibility, UI-agnostic playback, explicit component, audio-output, and bounded-filesystem requirements.
- [`docs/ADR.md`](../../../docs/ADR.md) and applicable records — architectural decision governance and existing private playback-seam decisions.
- [`docs/GLOSSARY.md`](../../../docs/GLOSSARY.md) — canonical terminology and explicitly open target terms.
- Living roadmap: `TFMX.cpp modernization roadmap`, Phase 3 — Architecture clarity; define and document responsibilities, boundaries, dependencies, and data flow before extraction.
- Living roadmap: `TFMX.cpp modernization roadmap`, Phase 5 — Modern C product readiness; future reusable C playback core and GUI/DAW foundation remain downstream context.

Completion notes
- Delivered approved target-only DAW component foundation and documentation; no implementation/extraction/API/behavior/format change. A1-A4 and M1-M4 passed; internal link/terminology/constraint and whitespace validations passed; roadmap index and Phase 3 were inspected; roadmap remains current because the user identifies one or two further Phase 3 details outside this Track, and no roadmap revision is approved. No changelog memory record is due because no commit/push occurred.
