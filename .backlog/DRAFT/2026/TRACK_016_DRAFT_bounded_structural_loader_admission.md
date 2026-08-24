# TRACK 016 [DRAFT]: bounded_structural_loader_admission

Track
- ID: TRACK_016
- Repository: SynthTracker
- Branch: `stage/04-04-loader-player-extraction`
- Current path: `.backlog/DRAFT/2026/TRACK_016_DRAFT_bounded_structural_loader_admission.md`

Problems (PORE)
- P1: As a SynthTracker listener, I cannot play real legacy TFMX modules, because
  the private playback loader is a finite-fixture content recognizer: it admits
  only the exact self-authored fixture layouts (`step8`, `loop_f1`,
  `envelope_tempo`, `voices_01`) with their exact trackstep, pattern, macro, and
  sample content, so the intended legacy corpus is rejected before the legacy
  playback path runs.
- P2: As a SynthTracker maintainer, I cannot evaluate loader behavior against a
  representative legacy corpus, because no bounded corpus inventory and
  structural-family evidence exist; the removed historical loader
  (`b12f77a^:src/tfmx.c` `load_tfmx`) is shallow and unsafe as an oracle, and
  the user-local corpus lives outside the repository with no committed evidence
  record.
- P3: As a SynthTracker maintainer, I cannot trust multi-pattern/multi-macro
  playback, because the legacy bridge points the interpreter's `patterns` and
  `macros` globals directly into the copied on-disk table region inside
  `editbuf`, coupling the legacy playback path to loader-derived table
  placement instead of validated, bridge-owned normalized tables.

Objective
- Replace fixture-specific loader content admission with bounded structural
  admission for the selected legacy corpus: raw SMPL treated as opaque bytes,
  zero header pointers resolved to documented defaults, pointer tables scanned
  and normalized up to 128 entries per table, and bridge-owned validated
  pattern/macro tables.
- The promised compatibility outcome is bounded structural load/start only.
  Timing, effects, loop behavior, PCM/exact audio, and other playback-detail
  discrepancies are explicitly deferred to later playback-focused work: this
  Track makes no intentional interpreter/timing/audio semantic change beyond
  admissibility and reachability, and observed differences are recorded, not
  resolved.

Non-negotiables
- This Track is DRAFT: planning only, no implementation, no tests intended to
  drive a code change.
- Implementation begins only after the Track is ACTIVE and its Move-to-ACTIVE
  plan step (S2) is checked, and follows TDD for every chunk: a focused failing
  automated test, the smallest passing change, refactor, then relevant
  validation, with the Track updated after each meaningful chunk.
- All project-owned production and test source stays in C23 or a later ISO C
  standard; project-owned headers remain private and co-located; `include/`
  remains retired. No public C API, header, export, library, or public target
  is added.
- Compatibility is non-promissory: bounded structural load/start admission for
  the selected corpus is temporary Phase 4 scaffold evidence, not a
  SynthTracker v1 compatibility promise. This Track asserts no format-wide
  validity, no exact audio behavior, and no resolution of deferred playback
  details (timing, effects, loop behavior, PCM/exact audio); its
  interpreter/timing/audio impact is stated as no intentional semantic change
  beyond admissibility/reachability, with observed differences deferred.
- The external legacy corpus remains uncommitted and outside the repository
  and is supplemental manual evidence only, not a repository automated
  acceptance criterion. This Track records corpus evidence by directory and
  module identifiers and permitted aggregate metadata only (module counts,
  structural families, default-pointer layouts), never absolute local paths,
  never literal module or sample byte values or derived per-file raw-content
  observations, and never copies module or sample content into the repository;
  the inventory covers loader outcome records for all 21 paired modules, and
  the two user-confirmed smoke cases are retained as supplemental load/start
  evidence.
- Fixture-content rejection tests are rebased into self-authored structural-
  invalid contract tests; truncated, alignment, and out-of-range rejection
  coverage is retained and strengthened. TDD remains required after ACTIVE.
- Paired SMPL is raw sample data with no header; no loader rule may invent a
  header or reject SMPL from its leading bytes. The current leading-`00 00`
  rejection is removed.
- Retain the four self-authored valid fixture pairs and their layout notes.
  Malformed-fixture expectations may be rebased only under explicit user
  approval, with the rationale recorded in this Track.
- Governed documentation updates require README.md, docs/ARCHITECTURE.md,
  docs/ASR.md, and docs/ARTIFACTS.md; ADR and glossary are explicitly assessed
  unchanged unless the actual implementation introduces an architectural or
  terminology change.
- During Phase 4, assess and retain bounded evidence for the intended
  compatibility impact on legacy TFMX module admission, trackstep, pattern,
  macro, timing, interpreter, and audio semantics; compatibility preservation
  is a temporary development scaffold, not a SynthTracker v1 promise.

Acceptance criteria
- [ ] A1) [P1] An automated component contract proves the private loader
  performs bounded structural admission: header magic and minimum size,
  subsong `start`/`end` bounds, table-pointer alignment and in-bounds checks,
  zero-pointer defaults resolved to the documented `trackstart` 0x800,
  `pattstart` 0x400, `macrostart` 0x600 (see `docs/TFMXLegacy/FORMAT.md:65-74`),
  independent on-disk pattern/macro pointer-table scans of up to 128 entries
  each, at least one valid entry per table, and candidate metadata safety
  (counts, normalized arrays, `first_pattern`) with transactional dispose. The
  removed fixture-content recognizer is no longer required for admission.
- [ ] A2) [P1] An automated component contract proves raw SMPL admission:
  SMPL is opaque, requires at least two bytes, and leading zero bytes are
  admitted; no loader sample-range inference is performed from macro content.
- [ ] A3) [P3] An automated bridge contract proves bridge-owned normalized
  pattern/macro arrays (capacity 128): every loader metadata entry is validated
  and copied into bridge-owned arrays, the interpreter's `patterns`/`macros`
  globals reference those arrays and never alias the copied on-disk table
  region inside `editbuf`, bounded trackstep conversion over the resolved
  `[trackstart, first_pattern)` range is preserved, and reset clears the
  bridge-owned arrays.
- [ ] A4) [P2] The four-directory corpus inventory is recorded in this Track
  as supplemental manual evidence only, not a repository automated acceptance
  criterion: directory-by-directory module counts and pairs (`Turrican1`, 7;
  `Turrican2`, 7; `R-type`, 1; `Apprentice`, 6; 21 paired modules total),
  per-module loader outcome records for all 21 pairs, structural families
  clustered from header pointers and pointer-table entries (11 distinct
  families), and default-pointer layouts (14 of 21 modules resolve zero header
  pointers to the documented defaults). Automated structural-loader contract
  evidence stays self-authored and does not depend on external corpus content.
- [ ] A5) [P2] The user-confirmed smoke cases `Turrican2-LVL1` and
  `Turrican1-LVL1` are recorded as supplemental load/start evidence only; they
  are not permanent validation and are not repeated or replaced by any
  assertion of format-wide validity, exact audio, or resolved playback
  details.
- [ ] A6) [P2, P3] The fixture preservation/rebasing rationale is recorded with
  user approval: fixture-content rejection tests are rebased into self-authored
  structural-invalid contract tests (truncation, alignment, and out-of-range
  rejection coverage retained and strengthened), families whose only defect was
  a removed fixture-content rule (for example the leading-`00 00` SMPL rule and
  first-macro sample-range inference) are no longer malformed, and the
  resulting rejection coverage is documented explicitly.
- [ ] A7) [P1, P2, P3] The complete six-dimension version-impact decision is
  recorded in the Decision log (recorded during DRAFT) and validated at
  completion: C API/ABI unchanged; bounded private module-admission change; no
  intended interpreter/timing/audio semantic change beyond reachability;
  persistent DAW format unchanged; platform/audio adapter unchanged; private
  loader/bridge component boundary changes with no public package boundary
  change. Deferred playback details are recorded, relevant focused tests and
  full project validation pass, and README.md, docs/ARCHITECTURE.md,
  docs/ASR.md, and docs/ARTIFACTS.md are updated in the implementation change;
  ADR and glossary are assessed unchanged unless the implementation introduces
  an architectural or terminology change.
- [ ] A8) [P2] Narrow user-judged manual confirmation (see Decision log,
  "user-judged manual compatibility evidence"): after the TDD chunks deliver
  bounded structural admission through the current path, the user manually
  confirms audible playback through the current path for exactly two modules,
  `Turrican2-LVL1` and `Turrican1-LVL1`, and records this confirmation with
  its observed playback-difference notes. This confirmation is a narrow user
  judgment limited to these two modules, supplemental to automated
  self-authored fixture evidence; it neither evaluates audio quality nor
  asserts exact historical fidelity. Timing, effects, loop behavior, PCM/exact
  audio, and other observed playback differences are recorded as deferred
  playback-detail observations, not failed exact-audio requirements. No
  absolute corpus paths and no raw module or sample data are recorded.

Why now / impact
- Track 015's recorded external-module loader deferral left the private loader
  admitting only self-authored fixture shapes; a user-supplied corpus pair
  (recorded in Track 015) failed before TFMX initialization, and the
  selected-corpus inventory shows every paired legacy module would be rejected
  by the committed rules. The reversible two-file PoC demonstrated playable
  legacy songs under bounded structural admission; the corpus is ready for
  bounded structural load/start evidence before the planned Loader and Player
  extraction stage begins.

Scope
- In scope:
  - Bounded structural loader admission in the private `tfmx_loader.c`:
    removal of the finite-fixture content recognizer (exact trackstep
    bindings, exact pattern/macro content, exact layout sizes, first-macro
    sample-range inference) and the SMPL leading-zero rejection, while
    retaining header/magic/file-size, start/end, pointer alignment/in-bounds,
    table-scan termination, candidate metadata, and transactional-dispose
    safety.
  - Zero raw header `trackstart`/`pattstart`/`macrostart` resolution to the
    documented defaults 0x800/0x400/0x600.
  - Independent on-disk pattern and macro pointer-table scans of up to 128
    entries each, stopping before an unreadable cell and on zero, below-0x200,
    unaligned, or out-of-bounds raw entries; at least one entry per table;
    normalization to `editbuf` word indices in the existing metadata arrays.
  - Bridge-owned normalized pattern/macro arrays in `playback_legacy_bridge.c`
    with validation, reset semantics, and bounded trackstep conversion over the
    resolved `[trackstart, first_pattern)` range.
  - Raw SMPL treatment: opaque, minimum two bytes, leading zeros admitted.
  - Selected-corpus manual supplemental evidence: the four-directory inventory
    with per-module loader outcome records for all 21 paired modules, and the
    two user-confirmed smoke cases as supplemental load/start evidence; the
    external corpus is not a repository automated acceptance criterion.
  - User-approved fixture rebasing: fixture-content rejection tests rebased
    into self-authored structural-invalid contract tests with retained and
    strengthened truncated, alignment, and out-of-range rejection coverage.
  - The completed six-dimension version-impact decision and required
    documentation updates (README.md, docs/ARCHITECTURE.md, docs/ASR.md,
    docs/ARTIFACTS.md); ADR and glossary explicitly assessed unchanged unless
    the actual implementation introduces an architectural or terminology
    change.
- Out of scope:
  - A general TFMX format validator, a full loader redesign, or any claim of
    format-wide compatibility; the future loader redesign remains a separate
    roadmap stage.
  - SMPL header or sample-content validation: paired SMPL has no header, and
    sample-address/range safety is not re-derived from macro content.
  - Playback-detail discrepancies observed in the smoke run ("some details
    missing") and all timing, effects, loop behavior, PCM/exact audio, and
    other playback-detail differences: this Track makes no intentional
    interpreter/timing/audio semantic change beyond admissibility and
    reachability; observed differences are deferred to later playback-focused
    work and only recorded, not resolved.
  - Exact-audio, bit-identical, audio-quality, or general listener-audibility
    assertions beyond the narrow user-judged audible-playback confirmation in
    A8 for `Turrican2-LVL1` and `Turrican1-LVL1`.
  - Corpus content: no module or sample bytes are copied into the repository,
    and no absolute local corpus path is recorded; the external corpus is
    supplemental manual evidence, not repository automated acceptance evidence.
  - Modules outside the four selected directories; the wider corpus-wide scan
    remains supplemental manual context, not acceptance evidence.
  - Bridge reentrancy or multi-context playback; the single-global legacy
    constraint remains a known limitation.
  - GUI, public C API, audio-output, or persistent DAW format work.

Milestones
- [x] M1) Resolve open questions Q1-Q6 and record the decisions and the
  complete six-dimension version-impact decision while DRAFT (recorded in the
  Decision log and resolved-questions section below).
- [x] M2) Post-Track PoC cleanup (completed 2026-08-24, after this DRAFT Track
  was created): restore the two PoC source files
  (`src/playback/tfmx_loader.c`, `src/playback/playback_legacy_bridge.c`) to
  their committed state and rebuild the executable, verifying a clean tree;
  evidence is recorded in the PoC/revert inventory below. The Move-to-ACTIVE
  gate is no longer blocked on a future PoC revert.
- [ ] M3) Deliver the loader structural-admission contract through sequential
  TDD chunks.
- [ ] M4) Deliver the bridge-owned normalized-table contract through TDD.
- [ ] M5) Complete user-approved fixture rebasing, selected-corpus manual
  supplemental evidence, validation, documentation, and roadmap
  reconciliation.

Risks / decisions
- Risk: Removing fixture content checks may admit genuinely malformed modules;
  retained bounds checks mitigate this but provide no format-wide guarantee.
- Risk: The historical `load_tfmx` is not a safe validity oracle: its early
  conditions (magic, 1056-byte minimum, non-empty sample) pass for 20/21 pairs
  (the remaining pair fails the minimum-size condition), but its later pointer
  walks are unchecked; source-level admission is not runtime equivalence.
- Risk: Malformed-fixture tests assert the removed fixture-content rules;
  rebasing requires user approval and must not silently drop rejection
  coverage.
- Risk: Pointer-table scan termination and defaults could mis-admit truncated
  or degenerate tables; evidence is bounded to the selected corpus families.
- Risk: Bridge-owned arrays are process-global statics under the legacy
  constraint; non-reentrancy remains deferred and documented.
- Decision: Bounded structural admission, raw SMPL opacity, documented
  zero-pointer defaults, bridge-owned normalized tables, selected-corpus
  manual-only evidence, PoC-revert-first sequencing, and the complete
  six-dimension version-impact decision recorded during DRAFT.
- Version impact: Recorded complete below during DRAFT; validated at
  completion.

Open questions (all resolved during DRAFT review, 2026-08-24)
- [x] Q1) Exact retained structural rule set: the post-PoC `valid_mdat()`
  contract is bounded structural admission — header magic and minimum size,
  subsong `start`/`end` bounds, table-pointer alignment and in-bounds checks,
  zero-pointer defaults 0x800/0x400/0x600, independent on-disk pattern/macro
  pointer-table scans of up to 128 entries each, and at least one valid entry
  per table (a table with no valid entry can never reach a trackstep, so the
  requirement is justified as structural reachability, not content
  recognition). Contract tests are self-authored structural-invalid tests.
- [x] Q2) Fixture rebasing: fixture-content rejection tests are rebased into
  self-authored structural-invalid contract tests; truncated, alignment, and
  out-of-range rejection coverage is retained and strengthened. Families whose
  only defect was a removed fixture-content rule (`silent_sample_payload`'s
  leading-`00 00` SMPL and `sample_range_overflow`'s first-macro inference)
  are no longer malformed under structural admission; the narrowed coverage is
  documented in the rebasing rationale.
- [x] Q3) Corpus admission boundary: the external four-directory corpus is
  supplemental manual evidence only. All 21 paired modules are inventoried for
  loader outcomes; the two user-confirmed smoke cases are retained as
  supplemental load/start evidence; the external corpus is not a repository
  automated acceptance criterion, and the wider corpus-wide scan remains
  supplemental manual context.
- [x] Q4) SMPL contract: retained exactly as PoC — opaque content, minimum two
  bytes, leading zeros admitted, no header; zero-length samples need no
  distinct rule in this Track (they remain rejected by the minimum-size check
  and are deferred to the loader redesign); no loader sample-range inference
  from macro content.
- [x] Q5) Deferred playback details: the observed "missing details" from the
  smoke run, plus timing, effects, loop behavior, PCM/exact audio, and other
  playback-detail discrepancies, are recorded and deferred to later
  playback-focused work. This Track makes no intentional interpreter/timing/
  audio semantic change beyond admissibility and reachability; observed
  differences are not resolved here.
- [x] Q6) Documentation and version impact: required documentation is
  README.md, docs/ARCHITECTURE.md, docs/ASR.md, and docs/ARTIFACTS.md; ADR and
  glossary are explicitly assessed unchanged unless the actual implementation
  introduces an architectural or terminology change. The six-dimension
  version-impact decision is recorded complete in the Decision log (C API/ABI
  unchanged; bounded private module-admission change; no intended
  interpreter/timing/audio semantic change beyond reachability; persistent DAW
  format unchanged; platform/audio adapter unchanged; private loader/bridge
  component boundary changes, no public package boundary).

Decision log
- Decision (bug characterization): User stated on 2026-08-24 that the
  self-authored fixtures were exclusively built to support the legacy corpus
  but do not; the fixture-derived loader admission rules rejecting the intended
  corpus constitute a bug. This is an implementation-behavior finding, not a
  statement that any external module is malformed.
- Decision (selected corpus): User selected the four directories `Turrican1`,
  `Turrican2`, `R-type`, and `Apprentice` on 2026-08-24 as the representative
  corpus; inventory found 21 paired modules across 11 structural families.
- Decision (SMPL raw): User agreed on 2026-08-24 that paired SMPL is raw sample
  data with no header; a leading-`00 00` rejection invents a header rule for
  data that has none and must be removed; sample semantics are deferred to the
  planned loader redesign.
- Decision (reversible PoC): User approved on 2026-08-24 a reversible
  diagnostic PoC modifying exactly two source files
  (`src/playback/tfmx_loader.c`, `src/playback/playback_legacy_bridge.c`) with
  no tests, docs, Track, CMake, config, or Git changes; only the executable was
  built, and tests/audio were not run during the PoC.
- Decision (keep PoC as evidence): User directed on 2026-08-24 that the PoC
  working state is kept unchanged as temporary evidence to guide this Track,
  and that the revert happens after the DRAFT Track exists.
- Decision (smoke confirmation): User confirmed on 2026-08-24 that both
  `Turrican2-LVL1` and `Turrican1-LVL1` played audibly under the PoC build,
  noting "some details missing but we add them later"; this is supplemental
  temporary PoC evidence, retained as supplemental load/start evidence, not
  permanent validation.
- Decision (DRAFT): User approved on 2026-08-24 the exact Track identity
  `.backlog/DRAFT/2026/TRACK_016_DRAFT_bounded_structural_loader_admission.md`.
- Decision (DRAFT review, external corpus): The external four-directory corpus
  is supplemental manual evidence only: inventory all 21 pairs for loader
  outcomes, retain the two user-confirmed smoke cases as supplemental
  load/start evidence, do not make the external corpus a repository automated
  acceptance criterion, and do not store raw module/sample content or the
  user's absolute local path.
- Decision (DRAFT review, bounded outcome): The promised compatibility outcome
  is bounded structural load/start only; timing, effects, loop behavior,
  PCM/exact audio, and other playback-detail discrepancies are explicitly
  deferred; the interpreter/timing/audio impact is stated as no intentional
  semantic change beyond admissibility/reachability, with observed differences
  deferred.
- Decision (DRAFT review, fixture rejection tests): Fixture-content rejection
  tests are rebased into self-authored structural-invalid contract tests;
  truncated, alignment, and out-of-range rejection coverage is retained and
  strengthened; TDD remains required after ACTIVE.
- Decision (DRAFT review, documentation): Documentation updates require
  README.md, docs/ARCHITECTURE.md, docs/ASR.md, and docs/ARTIFACTS.md; ADR and
  glossary are explicitly assessed unchanged unless the actual implementation
  introduces an architectural or terminology change.
- Decision (user-judged manual compatibility evidence): User directed on
  2026-08-24 that acceptance requires the user to manually confirm audible
  playback through the current path for exactly two modules, `Turrican2-LVL1`
  and `Turrican1-LVL1`; this user-judged manual evidence is supplemental to,
  not decided by, automated self-authored fixture evidence, and it neither
  evaluates audio quality nor asserts exact historical fidelity; observed
  timing, effects, loop, PCM, and other playback differences are recorded as
  deferred playback-detail observations, not failed exact-audio requirements;
  no absolute corpus paths and no raw module or sample data are recorded.
- Decision (historical loader): The removed legacy `load_tfmx`
  (`b12f77a^:src/tfmx.c`) is recorded as a shallow, unsafe loader, not a
  validity oracle: 20/21 selected pairs pass its early conditions and the
  remaining pair fails the minimum-size check; no runtime legacy equivalence
  is claimed.
- Decision (PoC revert completed): On 2026-08-24, after this DRAFT Track was
  created, the user-approved PoC cleanup was executed: both PoC source files
  (`src/playback/tfmx_loader.c`, `src/playback/playback_legacy_bridge.c`)
  were restored exactly to their committed state at HEAD; the executable
  rebuild and `git diff --check` passed; no CTest, application, or audio run
  was performed; only this DRAFT Track remains changed. Evidence is recorded
  in the PoC/revert inventory below.
- Decision (version impact, complete): Recorded 2026-08-24 during DRAFT review
  and validated at completion:

  | Dimension | Impact | Reason |
  | --- | --- | --- |
  | C API/ABI | Unchanged | Loader and bridge remain private; no public header, export, library, or target is introduced. |
  | Module compatibility/extension | Bounded private module-admission change | Fixture-only content admission is replaced by bounded structural admission for the selected corpus; no module bytes, format, or extension changes. |
  | Interpreter/timing/audio behavior | No intended semantic change beyond reachability | Structural admission changes which modules reach the unchanged legacy interpreter; timing, effects, loop behavior, PCM/exact audio, and other playback-detail discrepancies are deferred, with observed differences recorded, not resolved. |
  | Persistent DAW format/versioning | Unchanged | No DAW persistence, schema, or version field is added or changed. |
  | Platform/audio-output adapter | Unchanged | No audio-output or platform adapter change; the CoreAudio route is untouched. |
  | Component/package boundaries | Private loader/bridge boundary changes; no public package boundary | The loader owns bounded structural table normalization; the bridge owns validated normalized pattern/macro arrays; no public package boundary changes. |

Plan (execution steps)
- [x] S1) PoC cleanup (completed 2026-08-24, after this DRAFT Track was
  created): restore the two PoC source files
  (`src/playback/tfmx_loader.c`, `src/playback/playback_legacy_bridge.c`) to
  their committed state, rebuild the executable, and verify a clean tree.
  Evidence: both files were restored exactly to HEAD; `cmake --build build
  --target SynthTracker --parallel 2` and `git diff --check` passed; no
  CTest, application, or audio run was performed; only this DRAFT Track
  remains changed.
- [ ] S2) Move Track 016 to ACTIVE (folder, filename, and title status) only
  after user approval. Open questions Q1-Q6 are already resolved and the
  complete six-dimension version-impact decision is already recorded during
  DRAFT; check this Move-to-ACTIVE plan step and begin TDD. This is the next
  real unchecked step; it is no longer blocked on any future PoC revert.
- [ ] S3) TDD chunk 1: define the first focused observable loader structural-
  admission contract and add its failing automated test (deterministic
  runtime-red), then implement the smallest passing change, refactor, and
  validate.
- [ ] S4) TDD chunk 2: define the bridge-owned normalized pattern/macro table
  contract with its failing automated test, then the smallest passing change,
  refactor, and validation.
- [ ] S5) TDD chunk 3: define the raw SMPL admission contract (opaque, minimum
  two bytes, leading zeros admitted, no sample-range inference) with automated
  evidence, under user-approved fixture rebasing.
- [ ] S6) Selected-corpus manual supplemental evidence: record the four-
  directory inventory with per-module loader outcome records for all 21 paired
  modules and the two user-confirmed smoke cases (`Turrican2-LVL1`,
  `Turrican1-LVL1`) as supplemental load/start evidence; the external corpus is
  not a repository automated acceptance criterion. This step also requires the
  user-judged manual compatibility confirmation for A8: the user manually
  confirms audible playback through the current path for exactly two modules,
  `Turrican2-LVL1` and `Turrican1-LVL1`, and the confirmation is recorded in
  this Track with its observed playback-difference notes. Evidence: user
  confirmation statement with date and per-module audible-playback outcome,
  plus the recorded timing, effects, loop, PCM, and other playback differences
  as deferred observations; automated self-authored fixture contract tests
  remain complementary and do not decide this user judgment.
- [ ] S7) Validation only, no implementation: run relevant component,
  application, integration, and bounded compatibility validation; reconcile
  the governed documentation and the six-dimension version-impact decision;
  inspect the living Phase 4 roadmap and reconcile it if the outcome
  materially changes it.
- [ ] S8) Completion only: on acceptance, move Track 016 to COMPLETED and check
  the completion transition step.

Current inventory
- `src/playback/tfmx_loader.c` — committed state (HEAD) is a finite-fixture
  content recognizer: requires `TFMX` magic, reads `end[0]`/`trackstart`/
  `pattstart`/`macrostart` at header offsets 0x140/0x1d0/0x1d4/0x1d8, and
  admits only the exact `step8`/`loop_f1`/`envelope_tempo`/`voices_01` layouts
  with their exact trackstep bindings (`0000 FE01..FE07` except `voices_01`
  `0100`), pattern/macro content, and sample rules; rejects SMPL beginning
  `00 00`; infers sample range from the first macro.
- `src/playback/playback_legacy_bridge.c` — committed state (HEAD) points the
  interpreter's `patterns`/`macros` globals directly into the copied on-disk
  table region inside `editbuf` at the loader-normalized `pattstart`/
  `macrostart`, coupling playback to on-disk table placement.
- PoC working state (historical; reverted exactly to HEAD after Track
  creation — see the PoC/revert inventory below): bounded structural
  pointer-table parsing up to 128 entries per table with zero header-pointer
  defaults (0x800/0x400/0x600); raw SMPL leading zeros admitted (minimum two
  bytes); first-macro sample-range inference removed; bridge-owned normalized
  pattern/macro arrays (capacity 128) replacing the `editbuf` aliasing;
  bounded trackstep conversion over `[trackstart, first_pattern)`.
- `tests/fixtures/` — four self-authored valid pairs (`step8`, `loop_f1`,
  `envelope_tempo`, `voices_01`) and ten malformed pairs
  (`malformed_invalid_active_binding`, `malformed_invalid_inactive_binding`,
  `malformed_invalid_macro_ordering`, `malformed_invalid_pattern_contract`,
  `malformed_invalid_stop_step`, `malformed_out_of_range_pattern`,
  `malformed_sample_range_overflow`, `malformed_silent_sample_payload`,
  `malformed_truncated_mdat`, `malformed_unaligned_track`) with `*_layout.md`
  notes; 42 files total. Tests live in `tests/playback/`
  (`test_playback_context.c`).
- External corpus (uncommitted, outside the repository; supplemental manual
  evidence only, not a repository automated acceptance criterion): four
  directories with 21 paired modules — `Turrican1` (7), `Turrican2` (7),
  `R-type` (1), `Apprentice` (6) — across 11 structural families, with 14 of
  21 modules using default pointer layouts; per-module loader outcome records
  for all 21 pairs are the manual inventory evidence. No module or sample
  byte values and no derived per-file raw-content observations are recorded.
- Historical evidence: `load_tfmx` acceptance conditions from
  `b12f77a^:src/tfmx.c` (magic `TFMX-SONG` variants, 1056-byte minimum, non-
  empty sample; unsafe unchecked pointer walks thereafter).
- Legacy reference citations: `docs/TFMXLegacy/FORMAT.md:65-74` (zero stored
  pointer selects the default; `trackstart` 0x800, `pattstart` 0x400,
  `macrostart` 0x600), `docs/TFMXLegacy/FORMAT.md:17` (dual-file `mdat` +
  `smpl`; `smpl` holds sample data), `docs/TFMXLegacy/PATTERNS.md:81-92`
  (track-to-pattern binding; `PNum == 0xFE` turns the channel off; `PNum >=
  0x90` is ignored; `0x80`-`0x8F` retains state). No documented rule rejects
  SMPL beginning `00 00`.

PoC / revert inventory (explicit)
- Pre-revert working state (verified 2026-08-24): exactly two files modified,
  both uncommitted: `src/playback/tfmx_loader.c` and
  `src/playback/playback_legacy_bridge.c`. No other file, test, doc, CMake,
  config, memory, or Git change exists.
- Loader PoC behavior: `valid_mdat()` retains header/magic/file-size,
  subsong `start`/`end`, pointer alignment/in-bounds, and candidate metadata
  safety; zero raw `trackstart`/`pattstart`/`macrostart` resolve to 0x800/
  0x400/0x600; each on-disk pattern/macro pointer table is scanned
  independently up to 128 entries, stopping before an unreadable cell and on
  zero, below-0x200, unaligned, or out-of-bounds raw entries, requiring at
  least one entry per table; every valid raw offset is normalized to an
  `editbuf` word index in the existing metadata arrays and `first_pattern`;
  the exact trackstep/pattern/macro content recognizer and first-macro
  sample-range inference are removed; `tfmx_loader_read()` drops the
  leading-SMPL-`00 00` rejection and retains `smpl_size >= 2`.
- Bridge PoC behavior: private bridge-owned static arrays
  `bridge_patterns`/`bridge_macros` (capacity 128) hold the validated,
  normalized loader metadata; every entry is bounds-validated and copied on
  start; the interpreter's `patterns`/`macros` globals reference these
  bridge-owned arrays instead of aliasing the copied on-disk table region;
  the bounded trackstep conversion over the resolved `[trackstart,
  first_pattern)` range is preserved; `tfmx_playback_legacy_bridge_reset`
  clears the bridge-owned arrays.
- PoC build evidence (pre-revert): `cmake --build build --target SynthTracker
  --parallel 2` passed; `git diff --check` passed; git status showed exactly
  the two permitted source files. Tests, the application, and audio were not
  run during the PoC; malformed-fixture rejection tests would be intentionally
  invalid while the PoC was in place.
- Smoke evidence (user-confirmed, supplemental only): `Turrican2-LVL1` and
  `Turrican1-LVL1` played audibly; the user reported some playback details
  missing, to be addressed later. This is temporary PoC evidence, not
  permanent validation; it asserts no format-wide validity, exact audio, or
  resolution of deferred playback details.
- Revert completed (after Track creation, 2026-08-24): both files were
  restored exactly to their committed state at HEAD; `cmake --build build
  --target SynthTracker --parallel 2` passed; `git diff --check` passed; no
  CTest, application, or audio run was performed; git status shows only this
  DRAFT Track changed. No PoC residue remains; the committed loaders at HEAD
  are again a finite-fixture content recognizer and an `editbuf`-aliasing
  bridge, as described in the Current inventory above.

Artifacts
- Living roadmap: `SynthTracker modernization roadmap` and
  `Phase 4 — Component extraction` in project memory; Phase 4 Stage 3
  (Audio Output extraction) is in progress on
  `stage/04-03-audio-output-extraction`; Loader and Player extraction is the
  planned next stage. This Track is a bounded loader-focused follow-up to the
  Track 015 external-module loader deferral, not a roadmap-stage change.
- Predecessor: `.backlog/COMPLETED/2026/TRACK_015_COMPLETED_coreaudio_live_route_and_sdl_retirement.md`
  (external-module loader deferral, `TFMX_LOAD_INVALID_FORMAT` record),
  `.backlog/COMPLETED/2026/TRACK_005_COMPLETED_fixture_corpus_compatibility.md`
  (finite-fixture recognizer origin), and
  `.backlog/COMPLETED/2026/TRACK_002_COMPLETED_compatibility_safeguards.md`
  (`step8` origin).
- Legacy reference: `docs/TFMXLegacy/README.md`, `docs/TFMXLegacy/FORMAT.md`,
  `docs/TFMXLegacy/PATTERNS.md`; citations follow
  `docs/TFMXLegacy/PROVENANCE.md` (no external module data or path recorded).
- PoC evidence sessions and the four-directory inventory analysis are recorded
  in session history (2026-08-24); this Track file is the durable record.
- Critique evidence: the active critique record "General legacy-module playback
  is blocked by fixture-specific loader admission" (project memory
  `synthtracker/critiques`, `mem_3dcdab9e9ce2421f86b052413e53bfd1`) is the
  originating evidence for P1; this Track's decisions and the PoC/revert
  inventory above preserve and answer it without altering the memory record.

Completion notes
- (DRAFT) No implementation performed. The user-approved PoC cleanup was
  completed after this DRAFT Track was created: both PoC source files were
  restored exactly to HEAD, the executable rebuild and `git diff --check`
  passed, and no CTest, application, or audio run was performed; only this
  DRAFT Track remains changed (evidence in the PoC/revert inventory).
- (DRAFT review, 2026-08-24) Open questions Q1-Q6 closed and the complete
  six-dimension version-impact decision recorded while DRAFT; the external
  corpus is supplemental manual evidence only, recorded by directory and
  module identifiers and permitted aggregate metadata only (21 pairs, 11
  structural families, 14 default-pointer layouts), with no module or sample
  byte values, raw pointer details, derived per-file raw-content observations,
  or absolute local paths; the promised outcome is bounded
  structural load/start with deferred playback details; fixture-content
  rejection tests are rebased into self-authored structural-invalid contract
  tests with retained and strengthened truncated, alignment, and out-of-range
  coverage; documentation impact is README.md, docs/ARCHITECTURE.md,
  docs/ASR.md, and docs/ARTIFACTS.md with ADR/glossary assessed unchanged.
  Track remains DRAFT and not ACTIVE; the Move-to-ACTIVE gate (S2) is the
  next real unchecked step and still requires user approval.
