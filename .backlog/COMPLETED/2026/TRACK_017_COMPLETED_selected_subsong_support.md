# TRACK 017 [COMPLETED]: selected subsong support

Track
- ID: TRACK_017
- Repository: SynthTracker
- Branch: stage/04-04-loader-player-extraction
- Current path: `.backlog/COMPLETED/2026/TRACK_017_COMPLETED_selected_subsong_support.md`

Problems (PORE)
- P1: As a legacy CLI listener, I cannot select a valid nonzero subsong with
  `-p <n>`, because the playback context rejects it and the bridge always
  starts subsong 0.
- P2: As a maintainer, I cannot safely support selected subsongs, because the
  temporary POC forwards an unbounded selector to legacy header indexing
  without validating the selected `start[n]`/inclusive `end[n]` range.
- P3: As a legacy CLI listener, I cannot start a selected subsong at an
  absolute trackstep with `-P`, because bridge reset discards the requested
  override and the current path does not validate it within the selection.

Objective
- Deliver bounded private selected-subsong playback through the existing CLI,
  context, and bridge path, with automated evidence and no broad TFMX
  compatibility claim.

Non-negotiables
- All implementation follows TDD: a focused failing test, the smallest passing
  implementation, then refactoring and validation.
- Preserve subsong-0 behavior and existing loader/bridge structural safeguards.
- Validate selection before the legacy interpreter starts; add no runtime
  subsong switching, callback-path work, public API, or reentrancy claim.
- Retain Phase 4's temporary compatibility-scaffold framing: evidence is
  bounded and not a SynthTracker v1 compatibility promise.

Acceptance criteria
- [x] A1) [P1, P3] A self-authored fixture proves that a structurally valid
  nonzero selected subsong starts, progresses, renders, and completes through
  the private playback context, both normally and from a valid absolute `-P`
  trackstep within that selected subsong. Evidence: `mdat.selected_01` and the
  S4 slot-1/absolute-position component contracts pass in the focused 50/50
  playback suite.
- [x] A2) [P2, P3] Malformed, negative, overflowed, and out-of-domain `-p`
  values fail with usage/status 2; parsed absolute `-P` positions outside the
  selected range and invalid selected-slot ranges fail safely with status 1,
  without leaving legacy playback state active. Evidence: S4 proves bridge
  reset/no-active-state rejection; S5 covers 11 invalid CLI cases and the
  malformed selected-slot application failure before audio lifecycle.
- [x] A3) [P1, P2, P3] The application forwards a valid selected subsong through
  its fake-CoreAudio lifecycle path, including a valid absolute `-P` override;
  invalid `-p`/`-P` input produces the status and diagnostic decided in Q3
  before ACTIVE implementation. Evidence: valid selected composition completes
  through the fake route, an out-of-range parsed absolute position fails before
  audio starts, and S4 directly proves the valid absolute-position playback
  effect.
- [x] A4) [P2] Existing subsong-0 behavior and loader/bridge structural
  protections remain covered. Evidence: slot 0 remains covered by the existing
  playback contracts and loader admission remains slot-0-only.
- [x] A5) [P1, P2] Focused component and application tests, the full CTest
  suite, build validation, and `git diff --check` pass. Evidence: focused
  playback 50/50; focused application 5/5; final build and full CTest 8/8;
  `git diff --check` clean.
- [x] A6) [P1, P2] The six-dimension compatibility-impact decision and required
  documentation assessment are recorded; any user-approved manual check is
  bounded supplemental evidence only. Evidence: README, Architecture, ASR, and
  Artifacts updated; ADR-002/003 and glossary assessed unchanged; the user
  provided bounded Turrican2-LVL1 confirmation recorded in S6.

Why now / impact
- The reversible POC showed that the extracted path can directly forward a
  selected index to the legacy start call once it stops blocking it. The POC
  retained no named module, selector, or successful manual outcome. The current
  CLI advertises `-p` but cannot fulfill nonzero selection, so the private
  scaffold has misleading observable behavior.

Scope
- In scope:
  - Strict CLI `-p` and `-P` parsing and bounded selector/position handling.
  - Context and bridge selected-subsong start behavior, including selected-slot
    structural validation before `StartSong`.
  - Absolute `-P` validation within the selected subsong range and preservation
    of that approved override across bridge reset/start.
  - Self-authored selected-subsong and malformed-selected-slot fixtures.
  - Playback-context and application-level automated tests.
  - Temporary POC inventory and disposal, compatibility assessment, and
    required documentation updates or explicit unchanged assessments.
- Out of scope:
  - Runtime switching, multi-context/reentrancy, and callback-path changes.
  - A loader redesign, general real-module admission, or selection-aware loader
    admission unless an explicit DRAFT decision expands the scope.
  - Timing, effects, loop, exact-audio, or audio-quality discrepancy work.
  - External module/sample content in the repository or corpus evidence as an
    automated acceptance criterion.

Milestones
- [x] M1) Record the POC inventory, dispose of the temporary two-file patch,
  and establish a clean baseline build. Evidence (2026-08-25): restored
  `src/playback/playback_context.c` and
  `src/playback/playback_legacy_bridge.c`; `cmake --build build` passed and
  `git diff --check` was clean. Path-scoped working-tree and staged diffs for
  both POC files against `HEAD` were empty, and their path-scoped status was
  empty; the only remaining worktree change is this DRAFT Track.
- [x] M2) Resolve open questions and record the six-dimension compatibility
  impact decision while DRAFT. Evidence (2026-08-25): Q1-Q5 are resolved in
  the Decision log; selected start remains private and bounded; and the
  six-dimension decision is recorded below.
- [x] M3) Deliver the bounded playback-context and bridge contract through TDD.
  Evidence (2026-08-25): S4 delivered selected slot-1 start, selected-range
  validation, absolute-position preservation and range validation, and
  out-of-domain selector rejection through focused red/green component tests.
  The focused suite passed 50/50; `git diff --check` passed; independent review
  found no blocker.
- [x] M4) Deliver strict CLI selection behavior and application composition
  through TDD. Evidence (2026-08-25): S5 delivered strict nonnegative CLI
  parsing, selector-domain rejection, post-load absolute-position preservation,
  and application lifecycle/error contracts. The focused application suite
  passed 5/5 and final full CTest passed 8/8.
- [x] M5) Complete validation, documentation assessment, and bounded
  supplemental manual evidence. Evidence: S6 recorded the manual confirmation,
  documentation updates, unchanged ADR/glossary assessment, and final
  automated validation.

Risks / decisions
- Risk: `StartSong` directly indexes legacy header arrays and trusts the range;
  forwarding a selector without a bounded pre-start check can read invalid data.
- Risk: The TFMX header has selector slots but no explicit subsong count; the
  presence rule for an unused slot requires an explicit decision.
- Risk: The current loader proves only subsong-0 admission; accepting another
  selector may make start behavior depend on a different structural range.
- Decision: The 2026-08-25 POC is temporary, uncommitted evidence only. It must
  be restored after this DRAFT exists and before Track implementation begins.
- Version impact: Decided during DRAFT. C API/ABI is unchanged because no public
  header, export, library, or target is introduced. Module compatibility is a
  bounded private start-admission change: loader admission remains slot-0-only,
  while the bridge validates selected slot `n` in `0..31` and its inclusive
  trackstep range; no module bytes, extension, or general admission claim
  changes. Interpreter/timing/audio behavior intentionally changes only in
  reachability: the unchanged interpreter receives `StartSong(n, 0)` and an
  absolute validated `-P` position within that selected range; macro, pattern,
  timing, mixing, device, and exact-audio behavior otherwise remain unchanged.
  Persistent DAW format/versioning is unchanged because no persistence or
  schema is involved. Platform/audio-output adapter behavior is unchanged:
  selection completes before the existing CoreAudio route starts. Component and
  package boundaries change only privately: application owns strict CLI
  parsing, context/bridge own selected-start and absolute-position validation,
  and loader remains slot-0 admission only; no public architecture,
  reentrancy, or callback-path boundary is introduced.

Open questions
- [x] Q1) The selector domain is exactly header slots `0..31`. There is no
  documented unused-slot sentinel or count; a selected slot is startable only
  when its decoded `start[n] <= end[n]` and complete inclusive range fit the
  validated trackstep span.
- [x] Q2) Retain slot-0 loader admission. Validate only the selected slot at
  private bridge start; selection-aware loader admission is out of scope.
- [x] Q3) Malformed, negative, or overflowed `-p`/`-P` values, and `-p` outside
  header slots `0..31`, print usage and return status 2. A parsed `-P` position
  outside the selected subsong range, or a structurally invalid selected slot,
  returns status 1 with no new CLI diagnostic.
- [x] Q4) Support `-P` as an absolute trackstep only within the selected
  subsong's validated inclusive range; preserve it across bridge reset/start.
- [x] Q5) Add one self-authored fixture retaining valid slot 0 and giving slot
  1 tracksteps 1..3, with distinct start and absolute-override signatures.
  Bounded supplemental manual evidence is `Turrican2-LVL1 -p 1 -P 2` rejection
  and `Turrican2-LVL1 -p 1 -P 0x45` selected-position playback confirmation.

Decision log
- Decision (POC sequencing): The user approved a temporary, two-source-file
  POC on 2026-08-25, then directed that this DRAFT be created before the POC is
  disposed. The POC removes the context's nonzero rejection and forwards the
  requested selector to `StartSong`; it is not production behavior.
- Decision (POC evidence limit): The POC build succeeded, but no named module,
  selector, or successful audible result is retained as Track evidence. It does
  not establish a selected-subsong compatibility claim.
- Decision (scope): This Track addresses selected-subsong playback only. The
  separate `Turrican-TITLE` missing-tempo observation remains deferred.
- Decision (selector and admission): Support exactly slots `0..31`; no
  undocumented unused-slot sentinel is inferred. Keep Track 016's slot-0
  loader admission and validate only the selected slot's decoded inclusive
  trackstep range at private bridge start.
- Decision (CLI failure behavior): Malformed, negative, or overflowed `-p`/`-P`
  values, and `-p` outside `0..31`, print usage and return status 2. A parsed
  `-P` outside the selected range, or a structurally invalid selected slot,
  returns status 1 without a new CLI diagnostic.
- Decision (absolute position): `-P` is supported as an absolute trackstep
  within the selected subsong's inclusive range. It is neither a relative
  offset nor a general unchecked legacy override.
- Decision (evidence): One self-authored fixture keeps slot 0 valid and gives
  slot 1 the inclusive range 1..3 with distinct signatures for normal selected
  start and absolute position 2. The user confirmed on 2026-08-25 that
  `Turrican2-LVL1 -p 1 -P 2` correctly rejects an absolute position outside
  subsong 1's range, while `Turrican2-LVL1 -p 1 -P 0x45` starts the expected
  selected position. This is bounded supplemental evidence only; no
  corpus-wide claim follows.
- Decision (six-dimension impact): C API/ABI, persistent DAW
  format/versioning, and platform/audio-output adapter are unchanged for the
  reasons recorded in Version impact. Module compatibility is a bounded private
  selected-start change; interpreter/timing/audio reachability intentionally
  changes only for selected start and validated absolute position; and only the
  private application/context/bridge responsibility allocation changes.

Plan (execution steps)
- [x] S1) Record the exact POC inventory in this Track, restore the two POC
  source files, rebuild the existing `build/` tree, and confirm no POC residue.
  Evidence (2026-08-25): the source files were restored; the existing build
  completed successfully; `git diff --check` was clean; path-scoped
  working-tree and staged diffs for both files against `HEAD` were empty; and
  the worktree retains only this DRAFT Track. The POC had no automated-test
  obligation.
- [x] S2) Resolve Q1-Q5 and record the complete six-dimension compatibility
  impact decision while DRAFT. Evidence (2026-08-25): see resolved questions,
  Decision log, and Version impact.
- [x] S3) Move Track 017 to ACTIVE (folder, filename, and title status).
  Evidence (2026-08-25): user approved activation after all DRAFT decisions
  and the complete six-dimension impact decision were recorded.
- [x] S4) TDD chunk: add failing playback-context contracts and self-authored
  fixtures for valid nonzero selection, invalid index, and invalid selected
  ranges, plus valid and out-of-range absolute positions; implement the
  smallest bounded context/bridge behavior; preserve reset and subsong-0
  behavior; refactor and validate.
  Red evidence (2026-08-25): self-authored `mdat.selected_01` and
  `mdat.malformed_selected_slot_end_span` fixtures plus their layout notes and
  four focused playback-context contracts were added. The normal slot-1 start,
  absolute position 2, malformed selected range, and absolute position 4 tests
  each fail only because current start returns `TFMX_START_UNSUPPORTED_SUBSONG`;
  their load assertions pass first. The selector-32 rejection remains covered.
  `cmake --build build --target test_playback_context` passed; focused CTest
  ran 50 tests with 46 passing and the 4 intended failures; `git diff --check`
  passed and independent review found no blocker.
  Green evidence (2026-08-25): `playback_context.c` now accepts slots `0..31`
  and retains out-of-domain rejection; `playback_legacy_bridge.c` defensively
  validates the selected decoded range and a captured absolute position before
  selected `StartSong`. The four contracts pass, including no-active-state
  rejection behavior; the focused suite passed 50/50 and `git diff --check`
  passed. The malformed selected-range fixture was reconciled to differ from
  the valid fixture only at `end[1]`; final independent review found no blocker.
- [x] S5) TDD chunk: add failing application contracts for strict `-p`/`-P`
  parsing and selected-subsong composition through the fake route; implement
  the decided CLI behavior; refactor and validate focused tests, full CTest,
  build, and diff checks.
  Red evidence (2026-08-25): application lifecycle contracts cover successful
  `-p 1 -P 2` composition, `-p 1 -P 4` rejection before audio start, and
  malformed, negative, overflowed, and selector-domain CLI arguments. The
  focused application suite built successfully with 2/4 tests passing and 2
  intended failures: bridge reset loses `-P 4`, so audio starts and status is
  0 instead of 1; permissive parsing treats `-p bad` as 0 instead of producing
  usage/status 2. Per-test bridge reset isolates the legacy global state;
  `git diff --check` passed and independent review found no blocker.
  Green evidence (2026-08-25): `application.c` now rejects malformed, partial,
  negative, overflowed, and out-of-domain selectors with usage/status 2; it
  retains parsed absolute position locally across load and restores it before
  start. Parsed absolute positions outside the selected range and malformed
  selected slots remain silent status-1 failures before audio lifecycle. The
  expanded focused application suite passed 5/5, including 11 invalid-argument
  cases and malformed-slot rejection; the full build and CTest passed 8/8; and
  `git diff --check` passed. S4 remains the direct component-level evidence that
  valid absolute position 2 changes selected-slot playback; S5 provides the
  bounded application composition and failure-before-audio evidence.
- [x] S6) Perform the user-approved bounded manual check; update the applicable
  README, architecture, ASR, ADR, and glossary records for the implemented
  selection and compatibility behavior; record any defensible unchanged
  assessment; update Track evidence and complete only after acceptance.
  Manual evidence (2026-08-25): the user confirmed that `Turrican2-LVL1 -p 1
  -P 2` correctly rejects an out-of-range absolute position for subsong 1, and
  that `Turrican2-LVL1 -p 1 -P 0x45` starts the expected selected position.
  This is a narrow user judgment, provides no timing/effects/exact-audio or
  corpus-wide claim, and records no external file path or module data.
  Documentation evidence (2026-08-25): README, Architecture, ASR, and Artifacts
  document the bounded private selected-start behavior, strict CLI failure
  partition, retained slot-0 loader admission, and no-claim framing. ADR-002
  and ADR-003 remain unchanged because no seam placement, public API, or
  architectural decision changes; the glossary remains unchanged because its
  existing Trackstep and Playback context terms remain sufficient. Documentation
  review found and corrected the `-P` parse-failure wording. Final automated
  evidence remains focused playback 50/50, focused application 5/5, full CTest
  8/8, and `git diff --check` clean.

Current inventory
- The temporary POC changed only `src/playback/playback_context.c` and
  `src/playback/playback_legacy_bridge.c`: it removes the nonzero rejection and
  forwards `subsong` to `StartSong`, with no selector bound or selected-slot
  validation. Both files were restored on 2026-08-25; no POC source change
  remains.
- `src/application.c` strictly parses nonnegative `-p`/`-P` values, rejects
  malformed, partial, negative, overflowed, and out-of-domain selectors with
  usage/status 2, and restores a parsed absolute position after load before
  playback start.
- `tests/application/test_application_audio_lifecycle.c` contains S5 red
  then green application contracts for selected composition, absolute-position
  rejection before audio start, strict CLI argument rejection, and malformed
  selected-slot failure.
- `src/playback/tfmx_loader.c` admits only against subsong-0 bounds.
- `src/player.c` `StartSong` indexes legacy header arrays directly.
- `tests/playback/test_playback_context.c` currently asserts rejection of a
  nonzero selector outside the supported domain; S4 contracts prove slot-1 selected start,
  absolute position 2, rejection of a selected end outside the trackstep span,
  and rejection of absolute position 4 outside slot 1's range; all 50 focused
  component tests pass.
- `tests/fixtures/mdat.selected_01` and
  `tests/fixtures/mdat.malformed_selected_slot_end_span` are self-authored S4
  fixtures; `selected_01_layout.md` and `selected_slot_end_span_layout.md`
  record their bounded structural and observable evidence.
- Track 016 records unavailable `-p 1` as a deferred playback observation.

Artifacts
- Active critique: project memory `synthtracker/critiques`, “Subsong selection
  via -p is unavailable.”
- Roadmap-derived work: project memory `synthtracker/roadmaps`, “Phase 4 —
  Component extraction,” Stage 4 Loader and Player extraction.
- Historical context: `.backlog/COMPLETED/2026/TRACK_016_COMPLETED_bounded_structural_loader_admission.md`.
- Documentation assessment: ADR-002, ADR-003, `docs/ADR.md`, and
  `docs/GLOSSARY.md` assessed unchanged; no new ADR is required.

Completion notes
- Completed on 2026-08-25 after user acceptance. All acceptance criteria and
  S1-S6 are complete. The living roadmap index (revision 24) and Phase 4
  section (revision 17) were inspected before completion; Phase 4 was revised
  to revision 18 and the canonical index to revision 25, recording Track 017's
  bounded delivery while Stage 4 remains in progress. The resolved
  `Subsong selection via -p is unavailable` critique was archived at revision
  2. No commit or push was performed. The unrelated working-tree change
  `docs/TDD_GOVERNANCE_ANALYSIS.md` is excluded from this Track by user
  approval and was not assessed or modified.
