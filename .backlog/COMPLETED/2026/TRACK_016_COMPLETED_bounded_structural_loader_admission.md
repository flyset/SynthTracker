# TRACK 016 [COMPLETED]: bounded_structural_loader_admission

Track
- ID: TRACK_016
- Repository: SynthTracker
- Branch: `stage/04-04-loader-player-extraction`
- Current path: `.backlog/COMPLETED/2026/TRACK_016_COMPLETED_bounded_structural_loader_admission.md`

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
- This Track is COMPLETED: all plan steps S1–S8 are checked, including the
  Move-to-ACTIVE plan step (S2) and the completion step (S8).
- Implementation began only after the Track was ACTIVE and its Move-to-ACTIVE
  plan step (S2) was checked, and followed TDD for every chunk: a focused
  failing automated test, the smallest passing change, refactor, then relevant
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
  module identifiers and permitted aggregate metadata only (directory counts,
  21 paired modules total, 11 structural families, 14 default-pointer
  layouts) plus bounded partial, unenumerated manual evidence that every
  tested pair was audible; it never enumerates or requires individual loader/
  playback outcome records for all 21 pairs. It never records absolute local
  paths, never literal module or sample byte values or derived per-file
  raw-content observations, and never copies module or sample content into
  the repository, and it makes no compatibility or exact-fidelity claim from
  corpus evidence. The two user-confirmed smoke cases are retained as
  supplemental load/start evidence and remain the only named two-module
  confirmation (A5/A8).
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
- [x] A1) [P1] An automated component contract proves the private loader
  performs bounded structural admission: header magic and minimum size,
  subsong `start`/`end` bounds, table-pointer alignment and in-bounds checks,
  zero-pointer defaults resolved to the documented `trackstart` 0x800,
  `pattstart` 0x400, `macrostart` 0x600 (see `docs/TFMXLegacy/FORMAT.md:65-74`),
  independent on-disk pattern/macro pointer-table scans of up to 128 entries
  each, at least one valid entry per table, the revised load/start-range
  invariants `first_pattern > trackstart` and all subsong-0 `end + 1` 16-byte
  tracksteps within `[trackstart, first_pattern)`, and candidate metadata
  safety (counts, normalized arrays, `first_pattern`) with transactional
  dispose. The removed fixture-content recognizer is no longer required for
  admission. Pre-correction historical evidence (2026-08-24, validated at S7
  before the S6c restrictive admission correction; retained as historical,
  not current validation): covered by focused
  `test_loader_normalizes_fixture_tables`,
  `test_loader_resolves_default_header_pointers`,
  `test_loader_scans_independent_tables_with_bounded_capacity`, and the
  retained/rebased malformed-table contract tests; focused
  `test_playback_context` passed 37/37. Re-opened pending the S6c corrective
  TDD step and its equality, before-trackstart, and end-span boundary
  structural-invalid coverage.
  S6c validation (2026-08-24, checked): the corrective TDD step proved the
  revised load/start-range invariants at loader primary admission — strict
  `first_pattern > trackstart`, and the inclusive subsong-0 `end[0] + 1`
  requiring all complete 16-byte tracksteps within
  `[trackstart, first_pattern)` — with the three self-authored
  structural-invalid pairs `first_pattern_equal_trackstart`,
  `first_pattern_before_trackstart`, and `end_span` and their layout notes,
  covered by focused loader rejection tests
  (`test_loader_rejects_first_pattern_equal_trackstart`,
  `test_loader_rejects_first_pattern_before_trackstart`,
  `test_loader_rejects_end_span`) and the malformed-case contract tests.
  Focused red: 46 total, 38 passing, 8 intended failures (the new S6c
  boundary tests); focused green: 46/46; full CTest 8/8; `git diff --check`
  passed. Impact is framed as restrictive private structural admission: no
  public/API, persistence, platform/audio adapter, or accepted-module
  interpreter/timing/audio behavior change.
- [x] A2) [P1] An automated component contract proves raw SMPL admission:
  SMPL is opaque, requires at least two bytes, and leading zero bytes are
  admitted; no loader sample-range inference is performed from macro content.
  Validated at S7 (2026-08-24): covered by focused
  `test_loader_admits_leading_zero_smpl_without_header_rule`,
  `test_loader_treats_smpl_as_opaque_without_macro_range_inference`, and
  `test_loader_rejects_one_byte_smpl`; focused `test_playback_context` passed
  37/37.
- [x] A3) [P3] An automated bridge contract proves bridge-owned normalized
  pattern/macro arrays (capacity 128): every loader metadata entry is validated
  and copied into bridge-owned arrays, the interpreter's `patterns`/`macros`
  globals reference those arrays and never alias the copied on-disk table
  region inside `editbuf`, bounded trackstep conversion over the resolved
  `[trackstart, first_pattern)` range is preserved with the revised
  load/start-range invariants (`first_pattern > trackstart`; all subsong-0
  `end + 1` 16-byte tracksteps within `[trackstart, first_pattern)`) defended
  at the bridge, and reset clears the bridge-owned arrays.
  Pre-correction historical evidence (2026-08-24, validated at S7 before the
  S6c restrictive admission correction; retained as historical, not current
  validation): covered by focused
  `test_legacy_bridge_owns_voices_01_tables_and_rejects_out_of_range_metadata`
  and `test_legacy_bridge_reset_clears_unused_table_slots`; focused
  `test_playback_context` passed 37/37. Re-opened pending the S6c bridge
  defense coverage.
  S6c validation (2026-08-24, checked): bridge defense now enforces both
  load/start-range invariants before legacy state binding/start — strict
  `first_pattern > trackstart`, and the inclusive subsong-0 `end[0] + 1`
  complete 16-byte trackstep span within `[trackstart, first_pattern)` —
  proven by the focused bridge rejection tests
  (`test_legacy_bridge_rejects_first_pattern_equal_trackstart`,
  `test_legacy_bridge_rejects_first_pattern_before_trackstart`,
  `test_legacy_bridge_rejects_end_span`). Focused red: 46 total, 38 passing,
  8 intended failures; focused green: 46/46; full CTest 8/8;
  `git diff --check` passed. Impact is framed as restrictive private bridge
  defense: no public/API, persistence, adapter, or accepted-module
  interpreter/timing/audio behavior change.
- [x] A4) [P2] The four-directory corpus inventory is recorded in this Track
  as supplemental manual evidence only, not a repository automated acceptance
  criterion: directory-by-directory module counts and pairs (`Turrican1`, 7;
  `Turrican2`, 7; `R-type`, 1; `Apprentice`, 6; 21 paired modules total),
  structural families clustered from header pointers and pointer-table entries
  (11 distinct families), and default-pointer layouts (14 of 21 modules
  resolve zero header pointers to the documented defaults), plus bounded
  partial, unenumerated manual evidence that every tested pair was audible.
  No individual loader/playback outcome is enumerated or required for all 21
  pairs; A5/A8 remain the only named two-module confirmation. Automated
  structural-loader contract evidence stays self-authored and does not depend
  on external corpus content. A4 remains checked: this aggregate-scope policy
  evidence is unaffected by the S6c restrictive admission correction.
- [x] A5) [P2] The user-confirmed smoke cases `Turrican2-LVL1` and
  `Turrican1-LVL1` are recorded as supplemental load/start evidence only; they
  are not permanent validation and are not repeated or replaced by any
  assertion of format-wide validity, exact audio, or resolved playback
  details. The prior user observations (2026-08-24) are preserved as
  pre-correction historical evidence; A5 was re-opened so the two named smoke
  cases are refreshed as supplemental load/start evidence after the S6c
  restrictive admission correction.
  S6d refresh (2026-08-25, checked): the user reconfirmed after the S6c
  restrictive admission correction that `Turrican2-LVL1` and
  `Turrican1-LVL1` are audible and playing normally through the current path,
  with no differences noted in those exact confirmations; this refreshes only
  the two named confirmations, the broader unenumerated manual evidence
  remains pre-S6c historical evidence, and no all-21 claim is made.
- [x] A6) [P2, P3] The fixture preservation/rebasing rationale is recorded with
  user approval: fixture-content rejection tests are rebased into self-authored
  structural-invalid contract tests (truncation, alignment, and out-of-range
  rejection coverage retained and strengthened), families whose only defect was
  a removed fixture-content rule (for example the leading-`00 00` SMPL rule and
  first-macro sample-range inference) are no longer malformed, and the
  resulting rejection coverage is documented explicitly.
  Pre-correction historical evidence (2026-08-24, validated at S7 before the
  S6c restrictive admission correction; retained as historical, not current
  validation): the seven semantic malformed pairs were rebased one-for-one
  into `empty_pattern_table`, `empty_macro_table`, `unaligned_pattern_entry`,
  `unaligned_macro_entry`, `out_of_range_macro`, `below_note_data_pattern`,
  and `truncated_macro_table` (S5a evidence, fixture inventory); retained
  `truncated_mdat`, `unaligned_track`, and `out_of_range_pattern` are
  unchanged; rejection coverage passed in the focused 37/37 run. Re-opened
  pending the S6c boundary structural-invalid additions (equality,
  before-trackstart, and end-span cases).
  S6c validation (2026-08-24, checked): the S6c boundary structural-invalid
  additions are recorded — three self-authored malformed pairs
  `first_pattern_equal_trackstart`, `first_pattern_before_trackstart`, and
  `end_span` (each with `mdat.`/`smpl.` fixture files and a layout note) are
  added to the fixture inventory, covered by focused loader rejection,
  bridge defense rejection, and malformed-case contract tests. Focused red:
  46 total, 38 passing, 8 intended failures (the new S6c boundary tests);
  focused green: 46/46; full CTest 8/8; `git diff --check` passed. The
  rebasing rationale is unchanged: rejection coverage is retained and
  strengthened, and the added coverage is documented in this Track and the
  fixture layout notes.
- [x] A7) [P1, P2, P3] The complete six-dimension version-impact decision is
  recorded in the Decision log (recorded during DRAFT) and validated at
  completion: C API/ABI unchanged; bounded private module-admission change; no
  intended interpreter/timing/audio semantic change beyond reachability;
  persistent DAW format unchanged; platform/audio adapter unchanged; private
  loader/bridge component boundary changes with no public package boundary
  change. Deferred playback details are recorded, relevant focused tests and
  full project validation pass, and README.md, docs/ARCHITECTURE.md,
  docs/ASR.md, and docs/ARTIFACTS.md are updated in the implementation change;
  ADR and glossary are assessed unchanged unless the implementation introduces
  an architectural or terminology change. Pre-correction historical validation
  (2026-08-24, at S7 before the S6c restrictive admission correction; retained
  as historical, not current validation): each dimension matches actual
  delivery — C API/ABI unchanged (no public header,
  export, library, or target introduced; only private `tfmx_loader.c` and
  `playback_legacy_bridge.c` changed); bounded private module-admission
  expansion (fixture-only content admission replaced by bounded structural
  admission; no module bytes, format, or extension changes); no intended
  interpreter/timing/audio semantic change beyond reachability (structural
  admission changes only which modules reach the unchanged legacy interpreter;
  deferred playback-detail observations recorded, not resolved); persistent DAW
  format unchanged (no DAW persistence code touched); platform/audio adapter
  unchanged (no `src/audio_output`, `application.c`, or `main.c` change);
  private loader/bridge boundary changed (loader owns bounded structural table
  normalization; bridge owns capacity-128 validated normalized arrays) with no
  public package boundary change. Focused `test_playback_context` passed 37/37,
  application CTest 3/3, integration CTest 2/2, and full CTest 8/8; `git diff
  --check` passed. Re-opened: validation must be re-run after the S6c
  correction, which narrows private admission with the load/start-range
  invariants and does not change API/ABI, persistence, platform/audio, or the
  public boundary.
  Final validation (2026-08-25, checked): the complete six-dimension
  version-impact decision is validated against actual post-S6c/S6d delivery —
  C API/ABI unchanged: no public API/ABI; no public header, export, library,
  or target introduced; only private `tfmx_loader.c` and
  `playback_legacy_bridge.c` changed. Module admission: restrictive private
  admission correction — the S6c load/start-range invariants (strict
  `first_pattern > trackstart`; all subsong-0 `end + 1` complete 16-byte
  tracksteps within `[trackstart, first_pattern)`) narrow private admission
  only; no module bytes, format, or extension changes. Interpreter/timing/
  audio: no intended accepted-module interpreter/timing/audio semantic change
  beyond reachability — structural admission and the bridge defense change
  only which modules reach and start through the unchanged legacy interpreter;
  deferred playback-detail observations are recorded, not resolved. Persistent
  DAW format: no DAW persistence; no DAW persistence, schema, or version code
  touched. Platform/audio adapter: unchanged; no `src/audio_output`,
  `application.c`, or `main.c` change. Component/package boundary: private
  loader/bridge only — loader owns bounded structural table normalization with
  the S6c invariants; bridge owns capacity-128 validated normalized arrays with
  the S6c defense; no public package boundary change. Focused
  `test_playback_context` passed 46/46, application CTest 3/3, integration
  CTest 2/2, and full CTest 8/8; `git diff --check` passed. The governed
  documentation was refreshed in the implementation change with the exact
  subsong-0 `start[0]`/inclusive `end[0]` wording and the strict first-pattern
  and span safeguards; ADR and glossary are assessed unchanged.
- [x] A8) [P2] Narrow user-judged manual confirmation (see Decision log,
  "user-judged manual compatibility evidence"): after the TDD chunks deliver
  bounded structural admission through the current path, the user manually
  confirms audible playback through the current path for exactly two modules,
  `Turrican2-LVL1` and `Turrican1-LVL1`, with no differences noted in those
  exact user confirmations; the separately recorded `Turrican-TITLE` missing
  tempo change and unavailable `-p 1` subsong selection remain historical
  out-of-scope deferred playback observations, not observations of these two
  modules. This confirmation is a narrow user judgment limited to these two
  modules, supplemental to automated self-authored fixture evidence; it
  neither evaluates audio quality nor asserts exact historical fidelity.
  Timing, effects, loop behavior, PCM/exact audio, and other observed playback
  differences are recorded as deferred playback-detail observations, not
  failed exact-audio requirements. No absolute corpus paths and no raw module
  or sample data are recorded. The prior user observations (2026-08-24) are
  preserved as pre-correction historical evidence; A8 was re-opened so this
  narrow user-judged confirmation is refreshed after the S6c restrictive
  admission correction.
  S6d refresh (2026-08-25, checked): the user reconfirmed after the S6c
  restrictive admission correction that `Turrican2-LVL1` and
  `Turrican1-LVL1` are audible and playing normally through the current path,
  with no differences noted in those exact user confirmations; this refreshes
  only the two named confirmations, the broader unenumerated manual evidence
  remains pre-S6c historical evidence, and no all-21 claim is made.

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
    with permitted aggregate metadata only (directory counts, 21 paired
    modules total, 11 structural families, 14 default-pointer layouts) plus
    bounded partial, unenumerated manual evidence that every tested pair was
    audible; no individual loader/playback outcome for all 21 pairs is
    enumerated or required, and the two user-confirmed smoke cases remain the
    only named two-module confirmation as supplemental load/start evidence;
    the external corpus is not a repository automated acceptance criterion.
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
  - Playback-detail discrepancies observed in the smoke run (concrete
    deferred playback observations: `Turrican-TITLE` has a missing tempo
    change, and subsong selection through `-p 1` is unavailable) and all
    timing, effects, loop behavior, PCM/exact audio, and other
    playback-detail differences: these are out-of-scope deferred playback
    observations, not a format-wide defect claim, exact-audio conclusion, or
    implementation commitment; this Track makes no intentional
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
- [x] M3) Deliver the loader structural-admission contract through sequential
  TDD chunks. Evidence (2026-08-24): completed through the loader TDD chunks
  S3 (default-pointer resolution), S5a (independent bounded pattern/macro
  table scans), S5b (raw-SMPL admission), and S6c (load/start-range
  invariants at loader primary admission: strict `first_pattern > trackstart`
  and the inclusive subsong-0 `end[0] + 1` complete 16-byte trackstep span
  within `[trackstart, first_pattern)`), each with focused red/green and full
  8-test CTest validation; detailed evidence in S3/S5a/S5b/S6c below. The S6c
  focused red run was 46 total with 38 passing and 8 intended failures (the
  new boundary tests); the green run passed 46/46; full CTest 8/8;
  `git diff --check` passed.
  Pre-correction historical evidence (S3/S5a/S5b, retained as historical, not
  current validation): re-opened pending the S6c corrective TDD step, which
  adds the load/start-range invariants (`first_pattern > trackstart`; all
  subsong-0 `end + 1` 16-byte tracksteps within `[trackstart, first_pattern)`)
  to loader primary admission; S6c now delivers that addition.
- [x] M4) Deliver the bridge-owned normalized-table contract through TDD.
  Evidence (2026-08-24): completed through S4 (bridge-owned capacity-128
  pattern/macro arrays, index/count validation, reset clearing, bounded
  trackstep conversion), covered by
  `test_legacy_bridge_owns_voices_01_tables_and_rejects_out_of_range_metadata`
  and `test_legacy_bridge_reset_clears_unused_table_slots` with focused 35/35
  and full 8-test CTest validation, and through S6c (bridge defense for both
  load/start-range invariants — strict `first_pattern > trackstart` and the
  inclusive subsong-0 `end[0] + 1` complete 16-byte trackstep span within
  `[trackstart, first_pattern)` — proven by three focused bridge rejection
  tests; focused red 46 total/38 passing/8 intended failures, focused green
  46/46, full CTest 8/8, `git diff --check` passed); detailed evidence in
  S4/S6c below.
  Pre-correction historical evidence (S4, retained as historical, not current
  validation): re-opened pending the S6c corrective TDD step, which adds
  bridge defense for the load/start-range invariants; S6c now delivers that
  defense.
- [x] M5) Complete user-approved fixture rebasing, selected-corpus manual
  supplemental evidence, validation, documentation, and roadmap
  reconciliation. Evidence (2026-08-24): user-approved fixture rebasing and
  S6 evidence (seven rebased structural-invalid pairs in A6; A4/A5 aggregate
  metadata and the A8 two-module user confirmation), plus S7 validation
  (focused 37/37, application CTest 3/3, integration CTest 2/2, full CTest
  8/8, governed documentation updated, six-dimension decision validated,
  living Phase 4 roadmap inspected); detailed evidence in S6/S7 and the
  Completion notes below. Pre-correction historical evidence: re-opened
  pending the S6c corrective TDD step (docs and full validation re-run after
  the restrictive admission correction).
  Final evidence (2026-08-25, checked): the post-S6c/S6d completion is
  recorded — S6c delivered the restrictive load/start-range invariants at
  loader primary admission and bridge defense; S6d refreshed the two named
  smoke confirmations (`Turrican2-LVL1`, `Turrican1-LVL1`); final S7
  validation passed focused component 46/46, application CTest 3/3,
  integration CTest 2/2, full CTest 8/8, and `git diff --check`, with the
  build directory configured under the CMocka prefix; the governed
  documentation carries the exact subsong-0 `start[0]`/inclusive `end[0]`
  wording and the strict first-pattern and span safeguards with ADR and
  glossary assessed unchanged; the six-dimension version-impact decision is
  validated (no public API/ABI; restrictive private admission correction; no
  intended accepted-module interpreter/timing/audio change beyond
  reachability; no DAW persistence; no platform/audio; private loader/bridge
  only); the S6d named smoke confirmation and the historical deferred
  observations (`Turrican-TITLE` missing tempo change; unavailable `-p 1`
  subsong selection) remain bounded/non-promissory out-of-scope records; the
  living Phase 4 roadmap was inspected and remains current. Pre-S8 historical
  statement (as of this M5 final evidence on 2026-08-25, before the S8
  completion step): final memory reconciliation remained for the S8 completion
  step and no memory mutation was performed yet.

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
  Reopened and revised under the user-approved S6c corrective amendment
  (2026-08-24): the exact retained rule set additionally requires, for a
  structurally valid load/start range, `first_pattern > trackstart` and that
  all subsong-0 `end + 1` 16-byte tracksteps fit within
  `[trackstart, first_pattern)`; rationale: these invariants prevent
  loader-success/bridge-start failure and pattern-as-trackstep
  interpretation, narrowing private admission without changing API/ABI,
  persistence, platform/audio, or the public boundary. The revised invariants
  are validated by the corrective TDD step S6c.
- [x] Q2) Fixture rebasing: fixture-content rejection tests are rebased into
  self-authored structural-invalid contract tests; truncated, alignment, and
  out-of-range rejection coverage is retained and strengthened. Families whose
  only defect was a removed fixture-content rule (`silent_sample_payload`'s
  leading-`00 00` SMPL and `sample_range_overflow`'s first-macro inference)
  are no longer malformed under structural admission; the narrowed coverage is
  documented in the rebasing rationale.
- [x] Q3) Corpus admission boundary: the external four-directory corpus is
  supplemental manual evidence only. Only aggregate metadata (directory
  counts, 21 paired modules total, 11 structural families, 14 default-pointer
  layouts) plus bounded partial, unenumerated manual evidence that every
  tested pair was audible is retained; individual loader/playback outcomes
  for all 21 pairs are neither enumerated nor required. The two
  user-confirmed smoke cases are retained as supplemental load/start evidence
  and remain the only named two-module confirmation; the external corpus is
  not a repository automated acceptance criterion, and the wider corpus-wide
  scan remains supplemental manual context.
- [x] Q4) SMPL contract: retained exactly as PoC — opaque content, minimum two
  bytes, leading zeros admitted, no header; zero-length samples need no
  distinct rule in this Track (they remain rejected by the minimum-size check
  and are deferred to the loader redesign); no loader sample-range inference
  from macro content.
- [x] Q5) Deferred playback details: the concrete deferred playback
  observations from the smoke run (`Turrican-TITLE` has a missing tempo
  change; subsong selection through `-p 1` is unavailable) plus timing,
  effects, loop behavior, PCM/exact audio, and other playback-detail
  discrepancies, are recorded as out-of-scope deferred playback observations
  with no format-wide defect claim, exact-audio conclusion, or implementation
  commitment, and deferred to later playback-focused work. This Track makes
  no intentional interpreter/timing/audio semantic change beyond
  admissibility and reachability; observed differences are not resolved here.
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
  with no differences noted in those exact confirmations. The historical
  generic "some details missing but we add them later" note is reconciled as
  the concrete deferred playback observations that `Turrican-TITLE` has a
  missing tempo change and that subsong selection through `-p 1` is
  unavailable; these are out-of-scope deferred playback observations, not a
  format-wide defect claim, exact-audio conclusion, or implementation
  commitment. This is supplemental temporary PoC evidence, retained as
  supplemental load/start evidence, not permanent validation.
- Decision (DRAFT): User approved on 2026-08-24 the exact Track identity
  `.backlog/DRAFT/2026/TRACK_016_DRAFT_bounded_structural_loader_admission.md`.
- Decision (DRAFT review, external corpus): The external four-directory corpus
  is supplemental manual evidence only: inventory all 21 pairs for loader
  outcomes, retain the two user-confirmed smoke cases as supplemental
  load/start evidence, do not make the external corpus a repository automated
  acceptance criterion, and do not store raw module/sample content or the
  user's absolute local path.
- Decision (supersession, 2026-08-24, user-approved): The all-21 individual
  loader/playback-outcome inventory requirement above is superseded by a
  durable rule: the four-directory corpus is supplemental manual evidence;
  only aggregate metadata (directory counts, 21 paired modules total, 11
  structural families, 14 default-pointer layouts) plus bounded partial,
  unenumerated manual evidence that every tested pair was audible is
  retained, and no individual result for all 21 pairs is enumerated or
  required. A5/A8 remain the only named two-module confirmation, with no
  paths/content/raw observations and no compatibility/exact-fidelity claim.
  Rationale: all-21 individual testing is impractical; this weakens only the
  manual corpus evidence claim and does not change loader behavior, automated
  contracts, Phase 4 bounded/non-promissory compatibility policy, or
  no-path/no-content rules.
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
- Decision (S6c restrictive admission correction, user-approved, 2026-08-24):
  a corrective TDD step S6c is added after S6 and before S7: a structurally
  valid load/start range requires `first_pattern > trackstart`, and all
  subsong-0 `end + 1` 16-byte tracksteps must fit within
  `[trackstart, first_pattern)`, covered at loader primary admission and
  bridge defense with equality, before-trackstart, and end-span boundary
  structural-invalid tests, then governed docs and full validation.
  Rationale (user-approved): the correction narrows private admission to
  prevent loader-success/bridge-start failure and pattern-as-trackstep
  interpretation; it does not change API/ABI, persistence, platform/audio,
  or the public boundary.

Plan (execution steps)
- [x] S1) PoC cleanup (completed 2026-08-24, after this DRAFT Track was
  created): restore the two PoC source files
  (`src/playback/tfmx_loader.c`, `src/playback/playback_legacy_bridge.c`) to
  their committed state, rebuild the executable, and verify a clean tree.
  Evidence: both files were restored exactly to HEAD; `cmake --build build
  --target SynthTracker --parallel 2` and `git diff --check` passed; no
  CTest, application, or audio run was performed; only this DRAFT Track
  remains changed.
- [x] S2) Move Track 016 to ACTIVE (folder, filename, and title status) only
  after user approval. Open questions Q1-Q6 are already resolved and the
  complete six-dimension version-impact decision is already recorded during
  DRAFT; check this Move-to-ACTIVE plan step and begin TDD. It was next at
  activation; it is no longer blocked on any future PoC revert.
- [x] S3) TDD chunk 1: define the first focused observable loader structural-
  admission contract and add its failing automated test (deterministic
  runtime-red), then implement the smallest passing change, refactor, and
  validate. Evidence (2026-08-24): the red run used
  `cmake --build build --target test_playback_context --parallel 2` followed by
  `./build/test_playback_context`; it ran 33 tests, with only
  `test_loader_resolves_default_header_pointers` failing because actual value
  `3` (`TFMX_LOAD_INVALID_FORMAT`) did not equal the expected success value and
  the other 32 tests passed. The green implementation changed only
  `src/playback/tfmx_loader.c`, resolving raw zero `trackstart`, `pattstart`,
  and `macrostart` to `0x800`, `0x400`, and `0x600` before the existing
  validation. The focused test added
  `tests/fixtures/mdat.default_pointers` and
  `tests/fixtures/default_pointers_layout.md`, asserts the resolved metadata,
  normalized pattern/macro entries, `first_pattern`, and post-read disposal.
  `cmake --build build --target test_playback_context --parallel 2 &&
  ./build/test_playback_context` passed all 33 tests; the broader
  `cmake --build build --parallel 2 && ctest --test-dir build
  --output-on-failure` passed all 8 CTest tests. The binary fixture check
  reported `size=0x850, raw header pointers=0, default tables/track/content
  verified`; `git diff --check` passed.
- [x] S4) TDD chunk 2: define the bridge-owned normalized pattern/macro table
  contract with its failing automated test, then the smallest passing change,
  refactor, and validation. Evidence (2026-08-24): the focused red run used
  `cmake --build build --target test_playback_context --parallel 2 &&
  ./build/test_playback_context`; it ran 34 tests, with only
  `test_legacy_bridge_owns_voices_01_tables_and_rejects_out_of_range_metadata`
  failing because the mutated copied macro-table slot changed voice 0 volume to
  30 instead of the intended 18. The smallest green implementation changed
  only `src/playback/playback_legacy_bridge.c` and
  `tests/playback/test_playback_context.c`: private capacity-128 normalized
  pattern/macro arrays are populated before binding the legacy globals, counts
  are constrained to 1..128, every metadata index is non-negative and within a
  complete copied MDAT word, the aligned in-range `[trackstart, first_pattern)`
  conversion remains bounded, and reset clears the arrays and existing legacy
  state. The focused command then passed all 34 tests, including rejection of
  pattern and macro indices beyond the complete copied MDAT payload. Compatibility
  impact is bounded to private bridge ownership and metadata admission: the
  unchanged normalized table values, trackstep conversion, interpreter, timing,
  audio, and raw-SMPL behavior are preserved; malformed out-of-range metadata
  is rejected before start, with no public or private interface change. Full
  validation used `cmake --build build --parallel 2 && ctest --test-dir build
  --output-on-failure` and passed all 8 CTest tests; `git diff --check` passed.
  The separate deterministic reset-proof test
  `test_legacy_bridge_reset_clears_unused_table_slots` seeds `voices_01`,
  resets, restarts with copied metadata `pattern_count=1`/`macro_count=2`, and
  proves stale pattern slot 1 cannot activate voice 1; it then re-seeds/resets
  and repeats with `pattern_count=2`/`macro_count=1` to prove stale macro slot 1
  cannot activate voice 1. With only the two bridge-array clearing calls
  temporarily removed, `cmake --build build --target test_playback_context
  --parallel 2 && ./build/test_playback_context` failed deterministically:
  35 tests ran, 34 passed, and this reset-proof test failed because
  `pattern_restart_activated_voice_one` was not false. The source was restored
  immediately byte-for-byte to the intended S4 production content (SHA-256
  `f2e9c283b4a048d16c1a52f43c483a41758f4d184f9b1c95d1a44897dc5bb8e5`), with
  no persistent production diff beyond the approved S4 change. Final focused
  validation passed all 35 tests; full build/CTest passed all 8 tests; and
  `git diff --check` passed.
- [x] S5a) TDD chunk 3: structural-admission TDD chunk that replaces finite
  fixture-content recognition with bounded independent on-disk pattern/macro
  pointer-table scans (up to 128 entries each, at least one valid entry per
  table) and rebases semantic-invalid fixture evidence into self-authored
  structural-invalid evidence, under user-approved fixture rebasing: define the
  focused observable loader structural-admission contract and add its failing
  automated test, then implement the smallest passing change, refactor, and
  validate. Evidence (2026-08-24): after the focused test and fixture rebasing,
  the red command `cmake --build build --target test_playback_context
  --parallel 2 && ./build/test_playback_context` ran 34 tests, with 32 passing
  and two intended failures: `test_loader_normalizes_fixture_tables` still
  reported macro count 1 instead of 2, and the new table-scan contract reported
  `TFMX_LOAD_INVALID_FORMAT` (status 3) instead of success. The smallest green
  implementation changed only `src/playback/tfmx_loader.c`: a private,
  independent scan accepts readable aligned raw targets, stops at a zero,
  invalid target, or unreadable cell, caps each table at 128 entries, requires
  one entry per table, normalizes into the existing metadata arrays, and derives
  `first_pattern` from the first normalized pattern. The focused green command
  then passed all 34 tests. Full validation used `cmake --build build --parallel
  2 && ctest --test-dir build --output-on-failure` and passed all 8 CTest tests;
  `git diff --check` passed.
  Compatibility impact at the S5a boundary was bounded to private loader
  admission and metadata reachability: magic/minimum-size, start/end trackstep
  bounds, zero-pointer defaults, resolved pointer checks, first-macro
  sample-range inference, and leading-`00 00` SMPL rejection remained unchanged
  pending S5b. No intentional trackstep, interpreter, timing, audio, or SMPL
  behavior change was made in S5a.
- [x] S5b) TDD chunk 4: raw-SMPL admission TDD chunk proving opaque at-least-
  two-byte SMPL admission, leading-zero admission, and no macro-derived
  sample-range inference, under user-approved fixture rebasing: define the
  focused observable raw-SMPL contract and add its failing automated test,
  then implement the smallest passing change, refactor, and validate. Evidence
  (2026-08-24): the red command `cmake --build build --target
  test_playback_context --parallel 2 && ./build/test_playback_context` ran 37
  tests, with 35 passing, the leading-zero and opaque-macro-range admission
  tests failing with status `3` (`TFMX_LOAD_INVALID_FORMAT`), and the one-byte
  minimum-size rejection test passing. The green run with the same command
  passed all 37 tests. The smallest production change changed only
  `src/playback/tfmx_loader.c`: `smpl_size` was removed from `valid_mdat()` and
  its call, first-macro sample start/length inference and rejection were
  removed, the leading-two-zero-byte rejection was removed, and the candidate
  `smpl_size < 2` rejection was retained. The three tests call
  `tfmx_loader_read()` directly and dispose candidates without context
  start/tick/render. Added evidence is limited to `smpl.raw_leading_zero`
  (`00 00`), `smpl.raw_one_byte` (`00`), the single-byte derivative
  `mdat.raw_smpl_opaque` (only `mdat.step8` byte `0x26B` changed from `02` to
  `03`), and `raw_smpl_layout.md`; `mdat.step8`, `smpl.step8`, and
  `mdat.table_scan_2p3m` are reused. Compatibility impact is bounded to
  private loader admission: raw SMPL remains opaque with a two-byte minimum,
  leading zeros are admitted, and macro-derived sample-range validation is no
  longer applied. MDAT structural checks and normalized table scans are
  unchanged; there is no intended trackstep, interpreter, timing, audio, API,
  bridge, context, player, or playback-semantic change beyond reachability,
  and the fixtures make no playback claim. Full validation passed after the
  green run: `cmake --build build --parallel 2 && ctest --test-dir build
  --output-on-failure` passed all 8 CTest tests; `git diff --check` passed.
- Plan rationale (user-approved): the S5 decomposition into S5a/S5b keeps each
  TDD chunk small and sequential and does not alter Track 016 scope, acceptance
  criteria, decisions, or the remaining plan steps S6-S8.
- [x] S6) Selected-corpus manual supplemental evidence (completed 2026-08-24
  as pre-S6c historical manual evidence): record the four-
  directory inventory with permitted aggregate metadata only (directory
  counts, 21 paired modules total, 11 structural families, 14 default-pointer
  layouts) plus bounded partial, unenumerated manual evidence that every
  tested pair was audible; no individual loader/playback outcome for all 21
  pairs is enumerated or required. The two user-confirmed smoke cases
  (`Turrican2-LVL1`, `Turrican1-LVL1`) remain the only named two-module
  confirmation (A5/A8) as supplemental load/start evidence; the external
  corpus is not a repository automated acceptance criterion.   Evidence
  (2026-08-24): the user-approved supersession decision replaced the all-21
  individual inventory requirement with the aggregate-metadata rule above,
  and the user-judged manual compatibility confirmation for A8 (audible
  playback through the current path for exactly two modules,
  `Turrican2-LVL1` and `Turrican1-LVL1`, with no differences noted in those
  exact user confirmations) is recorded in the partial manual evidence below
  with its dated statement; the separately recorded `Turrican-TITLE` missing
  tempo change and unavailable `-p 1` subsong selection remain historical
  out-of-scope deferred playback observations. Automated self-authored
  fixture contract tests remain complementary and do not decide this user
  judgment. This S6 evidence is pre-S6c historical manual evidence: it does
  not satisfy the refreshed A5/A8 gate, which is rechecked only after the
  S6d refresh and user reconfirmation.
- [x] S6c) Corrective TDD step (user-approved, 2026-08-24), inserted after S6
  and before S7: prove the revised load/start-range structural invariants for
  a structurally valid load/start range — `first_pattern > trackstart`, and
  all subsong-0 `end + 1` 16-byte tracksteps fit within
  `[trackstart, first_pattern)`. Cover loader primary admission and bridge
  defense, including equality (`first_pattern == trackstart`),
  before-trackstart, and end-span boundary structural-invalid tests; then
  update the governed documentation (README.md, docs/ARCHITECTURE.md,
  docs/ASR.md, docs/ARTIFACTS.md) and run full validation. User-approved
  rationale: the correction narrows private admission to prevent
  loader-success/bridge-start failure and pattern-as-trackstep
  interpretation; it does not change API/ABI, persistence, platform/audio, or
  the public boundary. TDD: focused failing automated test for the intended
  observable behavior, smallest passing change, refactor, then validation.
  Evidence (2026-08-24, completed): TDD chunk executed. Red: the focused
  command `cmake --build build --target test_playback_context --parallel 2 &&
  ./build/test_playback_context` ran 46 tests, with 38 passing and 8 intended
  failures — the new S6c boundary tests: loader primary rejection
  (`test_loader_rejects_first_pattern_equal_trackstart`,
  `test_loader_rejects_first_pattern_before_trackstart`,
  `test_loader_rejects_end_span`), bridge defense rejection
  (`test_legacy_bridge_rejects_first_pattern_equal_trackstart`,
  `test_legacy_bridge_rejects_first_pattern_before_trackstart`,
  `test_legacy_bridge_rejects_end_span`), and the malformed-case contract
  coverage for the three new pairs. The smallest passing change enforced
  strict `first_pattern > trackstart` and the inclusive `end[0] + 1`
  complete 16-byte trackstep span within `[trackstart, first_pattern)` in
  loader primary admission (`src/playback/tfmx_loader.c`) and repeated both
  checks defensively in the bridge before legacy state binding/start
  (`src/playback/playback_legacy_bridge.c`); three self-authored
  structural-invalid pairs with layout notes were added
  (`mdat.`/`smpl.` `first_pattern_equal_trackstart`,
  `first_pattern_before_trackstart`, `end_span`). Green: the same focused
  command passed all 46 tests. Full validation: `cmake --build build
  --parallel 2 && ctest --test-dir build --output-on-failure` passed all 8
  CTest tests; `git diff --check` passed. Governed documentation was updated
  in the implementation change with the exact subsong-0 wording (inclusive
  `end[0]`; all `end + 1` complete 16-byte tracksteps within
  `[trackstart, first_pattern)`) in README.md, docs/ARCHITECTURE.md,
  docs/ASR.md, and docs/ARTIFACTS.md; ADR and glossary are assessed
  unchanged. Impact is framed as restrictive private structural admission:
  no public/API, persistence, platform/audio adapter, or accepted-module
  interpreter/timing/audio behavior change. S6d is the next unchecked step.
- [x] S6d) Post-S6c manual-evidence refresh (completed 2026-08-25): after the
  S6c green validation (and its governed documentation and full-validation
  steps), refresh only the two named smoke confirmations (`Turrican2-LVL1`,
  `Turrican1-LVL1`) and the bounded partial, unenumerated manual evidence
  that every tested pair was audible; no all-21 individual
  loader/playback-outcome replay is performed. Evidence (2026-08-25): the
  user reconfirmed after the S6c restrictive admission correction that
  `Turrican2-LVL1` and `Turrican1-LVL1` are audible and playing normally
  through the current path, with no differences noted in those exact
  confirmations. This refresh covers only the two named confirmations; the
  broader unenumerated manual evidence remains pre-S6c historical evidence
  and no all-21 claim is made. A5 and A8 are rechecked with this refresh.
  The separately recorded `Turrican-TITLE` missing tempo change and
  unavailable `-p 1` subsong selection remain historical out-of-scope
  deferred playback observations, not observations of these two modules.
  Aggregate metadata and the no-path/no-content/non-promissory rules are
  unchanged. S7 is the next unchecked step.
- [x] S7) Validation only, no implementation: run relevant component,
  application, integration, and bounded compatibility validation; reconcile
  the governed documentation and the six-dimension version-impact decision;
  inspect the living Phase 4 roadmap and reconcile it if the outcome
  materially changes it. Pre-correction historical evidence (2026-08-24):
  retained as historical, not current validation — the build directory was
  configured with the CMocka prefix (`CMAKE_PREFIX_PATH=/opt/homebrew/opt/
  cmocka`, `cmocka 2.0.2`); focused `cmake --build build --target
  test_playback_context --parallel 2 && ./build/test_playback_context` passed
  37/37; application CTest 3/3 (`test_application`,
  `test_application_removed_options`, `test_application_audio_lifecycle`);
  integration CTest 2/2 (`test_synthtracker_cli_identity`,
  `player_compile_probe`); full `cmake --build build --parallel 2 && ctest
  --test-dir build --output-on-failure` passed all 8 CTest tests; `git diff
  --check` passed. Governed documentation was updated in the implementation
  change: README.md, docs/ARCHITECTURE.md, docs/ASR.md, and docs/ARTIFACTS.md;
  ADR and glossary are assessed unchanged. Every six-dimension decision was
  validated against actual delivery (see A7): C API/ABI unchanged; bounded
  private admission expansion; no intended interpreter/timing/audio semantic
  change beyond reachability; DAW persistence unchanged; platform/audio
  adapter unchanged; private loader/bridge boundary changed with no public
  package boundary. The living Phase 4 roadmap (`SynthTracker modernization
  roadmap` and `Phase 4 — Component extraction` in project memory) was
  inspected: Stage 4, Loader and Player extraction, remains in progress on
  `stage/04-04-loader-player-extraction` and this Track is its implementation
  gate; the roadmap direction remains current and needs no reconciliation
  from this Track's outcome. No memory mutation is performed because Track 016
  is not yet complete and its DRAFT-to-ACTIVE status is a routine execution
  transition, not a memory-worthy event. S7 was re-opened pending the S6c
  corrective TDD step, after which component, application, integration,
  bounded compatibility, governed documentation, and version-impact
  validation must be re-run.
  Final validation (2026-08-25, checked): the post-S6c/S6d S7 validation
  re-run is complete. The build directory is configured with the CMocka
  prefix (`CMAKE_PREFIX_PATH=/opt/homebrew/opt/cmocka`, `cmocka 2.0.2`);
  focused `cmake --build build --target test_playback_context --parallel 2 &&
  ./build/test_playback_context` passed 46/46; application CTest 3/3
  (`test_application`, `test_application_removed_options`,
  `test_application_audio_lifecycle`); integration CTest 2/2
  (`test_synthtracker_cli_identity`, `player_compile_probe`); full `cmake
  --build build --parallel 2 && ctest --test-dir build --output-on-failure`
  passed all 8 CTest tests; `git diff --check` passed. The governed
  documentation is refreshed with the exact subsong-0 `start[0]`/inclusive
  `end[0]` wording and the strict first-pattern and span safeguards
  (`first_pattern` strictly after `trackstart`; all subsong-0 `end + 1`
  complete 16-byte tracksteps within `[trackstart, first_pattern)`) in
  README.md, docs/ARCHITECTURE.md, docs/ASR.md, and docs/ARTIFACTS.md; ADR and
  glossary are assessed unchanged. Every six-dimension decision is validated
  against actual delivery (see A7): no public API/ABI; restrictive private
  admission correction; no intended accepted-module interpreter/timing/audio
  semantic change beyond reachability; no DAW persistence; no platform/audio
  adapter; private loader/bridge only, with no public package boundary change.
  The S6d named smoke confirmation (`Turrican2-LVL1`, `Turrican1-LVL1`,
  user-reconfirmed 2026-08-25) is recorded, and the historical deferred
  observations (`Turrican-TITLE` missing tempo change; unavailable `-p 1`
  subsong selection) remain bounded, non-promissory out-of-scope records. The
  living Phase 4 roadmap (`SynthTracker modernization roadmap` and
  `Phase 4 — Component extraction` in project memory) was inspected: Stage 4,
  Loader and Player extraction, remains in progress on
  `stage/04-04-loader-player-extraction` and this Track is its implementation
  gate; the roadmap direction remains current and needs no reconciliation
   from this Track's outcome. Pre-S8 historical statement (as of this S7
   final validation on 2026-08-25, before the S8 completion step): no memory
   mutation is performed; final memory reconciliation remains for the S8
   completion step; the DRAFT-to-ACTIVE transition is a routine execution
   transition, not a memory-worthy event; S8 is the next unchecked step. S8
   has since been checked and Track 016 is COMPLETED.
- [x] S8) Completion only (checked 2026-08-25): Track 016 is accepted and moved
  to COMPLETED (folder, filename, and title status synchronized); the
  completion transition step is checked. No commit/push/changelog yet.

Partial manual evidence (2026-08-24; pre-S6c historical evidence)
- The user confirmed audible playback through the current path for exactly two
  modules, `Turrican2-LVL1` and `Turrican1-LVL1`, with no deferred playback
  differences noted for either; every tested but unenumerated pair was audible.
  The four-directory corpus is supplemental manual evidence: only aggregate
  metadata (directory counts, 21 paired modules total, 11 structural
  families, 14 default-pointer layouts) plus this bounded partial,
  unenumerated manual evidence is retained; no individual loader/playback
  outcome for all 21 pairs is enumerated or required, and A5/A8 remain the
  only named two-module confirmation. This is supplemental, non-promissory
  manual evidence with no exact-fidelity claim: it asserts no format-wide
  validity, no exact audio behavior, and no resolution of deferred playback
  details, and it preserves the no-path/no-content boundary (no absolute
  corpus paths, no module or sample byte values, no derived per-file
  raw-content observations). This is pre-S6c historical manual evidence
  under the user-approved 2026-08-24 aggregate-metadata supersession; it
  satisfies A4 and the completed S6 evidence, but it does not satisfy the
  refreshed A5/A8 gate, which is rechecked only after the S6d refresh and
  user reconfirmation.

Current inventory
- `src/playback/tfmx_loader.c` — bounded structural admission: it retains
  `TFMX` magic/minimum size, `start`/`end` trackstep bounds, zero header-pointer
  defaults, and resolved pointer minimum/alignment/in-bounds checks, then scans
  the pattern and macro tables independently for up to 128 readable, aligned
  raw targets, stopping before an unreadable cell or invalid terminator/entry
  and requiring at least one accepted entry per table. It normalizes accepted
  offsets into the existing metadata arrays and derives `first_pattern` from
  `patterns[0]`. S6c adds loader primary enforcement of the load/start-range
  invariants: strict `first_pattern > trackstart`, and the inclusive subsong-0
  `end[0] + 1` requiring all complete 16-byte tracksteps within
  `[trackstart, first_pattern)`. Raw SMPL is opaque with a retained two-byte
  minimum: leading `00 00` is admitted and no first-macro sample-range
  inference is performed; finite layout flags and exact trackstep/pattern/macro
  content comparisons are removed.
- `src/playback/playback_legacy_bridge.c` — committed state (HEAD) points the
  interpreter's `patterns`/`macros` globals directly into the copied on-disk
  table region inside `editbuf` at the loader-normalized `pattstart`/
  `macrostart`, coupling playback to on-disk table placement. S4 now replaces
  that alias with private capacity-128 bridge-owned normalized arrays, validates
  1..128 counts and every non-negative metadata index against the complete
  copied MDAT word range, copies before binding the globals, preserves bounded
  aligned trackstep conversion over `[trackstart, first_pattern)`, and clears
  the arrays during reset. S6c adds bridge defense: both load/start-range
  invariants (strict `first_pattern > trackstart`; inclusive subsong-0
  `end[0] + 1` complete 16-byte trackstep span within
  `[trackstart, first_pattern)`) are re-checked before legacy state
  binding/start.
- PoC working state (historical; reverted exactly to HEAD after Track
  creation — see the PoC/revert inventory below): bounded structural
  pointer-table parsing up to 128 entries per table with zero header-pointer
  defaults (0x800/0x400/0x600); raw SMPL leading zeros admitted (minimum two
  bytes); first-macro sample-range inference removed; bridge-owned normalized
  pattern/macro arrays (capacity 128) replacing the `editbuf` aliasing;
  bounded trackstep conversion over `[trackstart, first_pattern)`.
- `tests/fixtures/` — four self-authored valid pairs (`step8`, `loop_f1`,
  `envelope_tempo`, `voices_01`) and thirteen malformed pairs. The retained
  `truncated_mdat`, `unaligned_track`, and `out_of_range_pattern` pairs are
  unchanged; the seven semantic pairs are rebased one-for-one to
  `empty_pattern_table`, `empty_macro_table`, `unaligned_pattern_entry`,
  `unaligned_macro_entry`, `out_of_range_macro`, `below_note_data_pattern`,
  and `truncated_macro_table`, each with a structural layout note. S6c adds
  the three self-authored boundary structural-invalid pairs
  `first_pattern_equal_trackstart`, `first_pattern_before_trackstart`, and
  `end_span`, each with an `mdat.`/`smpl.` fixture pair and a layout note.
  The loader-only `mdat.table_scan_2p3m` and `mdat.table_scan_cap128` fixtures have
  layout notes and reuse `smpl.step8`; `mdat.default_pointers` and its layout
  note remain present. S5b adds the loader-only raw-SMPL evidence
  (`smpl.raw_leading_zero`, `smpl.raw_one_byte`, `mdat.raw_smpl_opaque`, and
  `raw_smpl_layout.md`) while reusing the three fixtures named above. There
  are 61 fixture files total. Tests live in `tests/playback/`
  (`test_playback_context.c`), including the three direct S5b loader tests
  and the S6c loader/bridge boundary rejection and malformed-case tests.
- External corpus (uncommitted, outside the repository; supplemental manual
  evidence only, not a repository automated acceptance criterion): four
  directories with 21 paired modules — `Turrican1` (7), `Turrican2` (7),
  `R-type` (1), `Apprentice` (6) — across 11 structural families, with 14 of
  21 modules using default pointer layouts. Only this aggregate metadata plus
  bounded partial, unenumerated manual evidence that every tested pair was
  audible is retained; no individual loader/playback outcome for all 21 pairs
  is enumerated or required, and A5/A8 remain the only named two-module
  confirmation. No module or sample byte values and no derived per-file
  raw-content observations are recorded.
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
  `Turrican1-LVL1` played audibly, with no differences noted in those exact
  confirmations. The historical generic "some playback details missing" note
  is reconciled as the concrete out-of-scope deferred playback observations
  that `Turrican-TITLE` has a missing tempo change and that subsong selection
  through `-p 1` is unavailable; these are not a format-wide defect claim,
  exact-audio conclusion, or implementation commitment. This is temporary PoC
  evidence, not permanent validation; it asserts no format-wide validity,
  exact audio, or resolution of deferred playback details.
- Revert completed (after Track creation, 2026-08-24): both files were
  restored exactly to their committed state at HEAD; `cmake --build build
  --target SynthTracker --parallel 2` passed; `git diff --check` passed; no
  CTest, application, or audio run was performed; git status shows only this
  DRAFT Track changed. No PoC residue remained at the time of revert; the
  pre-S3 committed loaders were again a finite-fixture content recognizer and
  an `editbuf`-aliasing bridge, as described in the then-current inventory.

Artifacts
- Living roadmap: `SynthTracker modernization roadmap` and
  `Phase 4 — Component extraction` in project memory; the Phase 4 record
  (revision 17) and the canonical roadmap index (revision 24) record Track
  016's delivered baseline. Phase 4 Stage 3 (Audio Output extraction) is
  completed and merged to main at d14beb8; Stage 4, Loader and Player
  extraction, remains in progress on `stage/04-04-loader-player-extraction`.
  Track 016 is COMPLETED with all plan steps S1–S8 checked; final roadmap
  reconciliation is complete. No commit, push, or changelog entry exists yet.
  This Track is a bounded loader-focused follow-up to the Track 015
  external-module loader deferral, not a roadmap-stage change.
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
  The Move-to-ACTIVE gate (S2) was completed after user approval, S3 and S4
  were completed with focused red/green and full 8-test CTest validation,
  S5a was completed with focused red/green, structural fixture rebasing, and
  full 8-test CTest validation, and S5b was completed with focused red/green,
  raw-SMPL fixture evidence, and full 8-test CTest validation. On 2026-08-24
  a user-approved Track-only revision replaced the all-21 individual
  loader/playback-outcome inventory requirement with a durable rule: the
  four-directory corpus is supplemental manual evidence retaining only
  aggregate metadata (directory counts, 21 paired modules total, 11
  structural families, 14 default-pointer layouts) plus bounded partial,
  unenumerated manual evidence that every tested pair was audible; no
  individual result for all 21 pairs is enumerated or required, and A5/A8
  remain the only named two-module confirmation. A4 and S6 were completed
  under this revision; it weakens only the manual corpus evidence claim and
  does not change loader behavior, automated contracts, Phase 4
  bounded/non-promissory compatibility policy, or no-path/no-content rules.
  S7 was completed on 2026-08-24 with focused 37/37, application CTest 3/3,
  integration CTest 2/2, full CTest 8/8, `git diff --check` passing, the
  governed documentation updated, the six-dimension decision validated
  against actual delivery, and the Phase 4 roadmap inspected as still current
  (no memory mutation). S6c is the next unchecked step; S8 (completion)
  remains unchecked.
- (Corrective plan amendment, user-approved, 2026-08-24) Track-only revision:
  added the unchecked corrective TDD step S6c after S6 and before S7 to prove
  that a structurally valid load/start range requires `first_pattern >
  trackstart` and that all subsong-0 `end + 1` 16-byte tracksteps fit within
  `[trackstart, first_pattern)`, with loader primary admission and bridge
  defense coverage, equality, before-trackstart, and end-span boundary
  structural-invalid tests, then governed docs and full validation. Q1 was
  reopened and revised to record these exact structural invariants and their
  rationale. A1, A3, A6, and A7 were unchecked with all prior evidence
  preserved as pre-correction historical evidence; M3, M4, and M5 and S7 were
  unchecked with their evidence preserved as pre-correction historical
  evidence; A5, A8, and S6 were unchecked so the two named smoke cases and
  the bounded manual compatibility evidence are refreshed after the
  restrictive correction, with their prior user observations preserved as
  historical. A4 remains checked as aggregate-scope policy evidence. All
  current-next-step statements point to S6c; S8 (completion) remains
  unchecked. User-approved rationale: the correction narrows private
  admission to prevent loader-success/bridge-start failure and
  pattern-as-trackstep interpretation; it does not change API/ABI,
  persistence, platform/audio, or the public boundary.
- (Corrective-plan cleanup, user-approved, 2026-08-24) Track-only revision of
  this ACTIVE file: S6 is re-checked as completed pre-S6c historical manual
  evidence, with A4 preserved checked as aggregate-scope policy evidence; an
  unchecked S6d is added after S6c and before S7 to refresh only the two
  named smoke confirmations (`Turrican2-LVL1`, `Turrican1-LVL1`) and the
  bounded partial manual evidence after the S6c green validation, rechecking
  A5/A8 only after the user reconfirms, with no all-21 replay; the partial
  manual evidence and current S6 wording now label the recorded manual
  evidence as pre-S6c historical evidence that does not satisfy the
  refreshed A5/A8 gate; checked S2's wording now states the Move-to-ACTIVE
  step was next at activation.   S6c remains the current next unchecked step;
  S7, S8, and all reopened A/M statuses are unchanged; all historical
  evidence and the no-path/no-content/non-promissory rules are preserved.
- (S6c evidence update, user-approved, 2026-08-24) Track-only revision of
  this ACTIVE file: S6c is checked with its corrective TDD evidence. The
  loader primary admission and the bridge defense now enforce strict
  `first_pattern > trackstart` and the inclusive subsong-0 `end[0] + 1`
  complete 16-byte trackstep span within `[trackstart, first_pattern)`,
  proven by the three self-authored structural-invalid pairs
  (`first_pattern_equal_trackstart`, `first_pattern_before_trackstart`,
  `end_span`) with layout notes and by focused loader rejection, bridge
  defense rejection, and malformed-case contract tests. Focused red: 46
  total, 38 passing, 8 intended failures; focused green: 46/46; full CTest
  8/8; `git diff --check` passed. The governed documentation carries the
  exact subsong-0 wording (inclusive `end[0]`; all `end + 1` complete 16-byte
  tracksteps within `[trackstart, first_pattern)`) in README.md,
  docs/ARCHITECTURE.md, docs/ASR.md, and docs/ARTIFACTS.md; ADR and glossary
  are assessed unchanged. A1, A3, A6, M3, and M4 are checked with all prior
  evidence preserved as historical; A5, A8, M5, A7, S7, and S8 remain
  unchecked. S6d is the next unchecked step. Impact is framed as restrictive
  private structural admission with no public/API, persistence, platform/audio
  adapter, or accepted-module interpreter/timing/audio behavior change; the
  no-path/no-content/non-promissory rules are preserved.
- (S6d evidence refresh, user-approved, 2026-08-25) Track-only revision of
  this ACTIVE file: S6d is checked with the post-S6c manual-evidence refresh.
  The user reconfirmed after the S6c restrictive admission correction that
  exactly `Turrican2-LVL1` and `Turrican1-LVL1` are audible and playing
  normally through the current path, with no differences noted in those exact
  confirmations. This refresh covers only the two named confirmations; the
  broader unenumerated manual evidence remains pre-S6c historical evidence
  and no all-21 claim is made. A5 and A8 are rechecked with this refresh. The
  separately recorded `Turrican-TITLE` missing tempo change and unavailable
  `-p 1` subsong selection remain historical out-of-scope deferred playback
  observations, not observations of these two modules. Aggregate metadata and
  the no-path/no-content/non-promissory rules are unchanged. S7 is the next
  unchecked step; M5, A7, S7, and S8 remain unchecked.
- (Final S7 reconciliation, user-approved, 2026-08-25) Track-only revision of
  this ACTIVE file: S7, A7, and M5 are checked with the final post-S6c/S6d
  validation. The build directory is configured with the CMocka prefix
  (`CMAKE_PREFIX_PATH=/opt/homebrew/opt/cmocka`, `cmocka 2.0.2`); focused
  component 46/46; application CTest 3/3; integration CTest 2/2; full CTest
  8/8; `git diff --check` passed. The governed documentation is refreshed
  with the exact subsong-0 `start[0]`/inclusive `end[0]` wording and the
  strict first-pattern and span safeguards (`first_pattern` strictly after
  `trackstart`; all subsong-0 `end + 1` complete 16-byte tracksteps within
  `[trackstart, first_pattern)`) in README.md, docs/ARCHITECTURE.md,
  docs/ASR.md, and docs/ARTIFACTS.md; ADR and glossary are assessed
  unchanged. All six dimensions are validated: no public API/ABI; restrictive
  private admission correction; no intended accepted-module
  interpreter/timing/audio semantic change beyond reachability; no DAW
  persistence; no platform/audio adapter; private loader/bridge only, with no
  public package boundary change. The S6d named smoke confirmation
  (`Turrican2-LVL1`, `Turrican1-LVL1`) is recorded and the historical
  deferred observations (`Turrican-TITLE` missing tempo change; unavailable
  `-p 1` subsong selection) remain bounded, non-promissory out-of-scope
   records. The living Phase 4 roadmap direction was inspected and remains
   current with no reconciliation needed. Pre-S8 historical statement (as of
   this final S7 reconciliation on 2026-08-25, before the S8 completion step):
   final memory reconciliation remained for the S8 completion step, no memory
   mutation was performed yet, and S8 was the next unchecked step. S8 has
   since been checked and the Track is COMPLETED.
- (S8 completion, user-approved, 2026-08-25) Track-only completion of this
  COMPLETED file: Track 016 delivered bounded private structural loader
  admission and bridge-owned normalized tables; final S7 validation and the
  governed documentation are complete. The Phase 4 roadmap (revision 17) and
  canonical roadmap index (revision 24) record the delivered baseline. Stage 4
  remains in progress, with the general loader redesign, remaining Player
  extraction, and the deferred playback-detail work outside this Track. No
  commit, push, or changelog entry exists yet; S8 is checked and the Track is
  COMPLETED.
