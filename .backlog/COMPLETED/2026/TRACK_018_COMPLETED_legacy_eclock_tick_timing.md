# TRACK 018 [COMPLETED]: legacy eClock tick timing

Track
- ID: TRACK_018
- Repository: SynthTracker
- Branch: stage/04-04-loader-player-extraction
- Current path: `.backlog/COMPLETED/2026/TRACK_018_COMPLETED_legacy_eclock_tick_timing.md`

Problems (PORE)
- P1: As a legacy CLI listener, I cannot hear or measure tempo, speed, and
  timeshare control changes, because the legacy interpreter updates `eClocks`
  during playback but the private playback context always passes the fixed
  default `14318` to the mixer, so rendered tick duration never follows the
  song's timing controls.
- P2: As a maintainer, I cannot plan this handoff safely, because the
  CoreAudio callback pulls rendered frames and is not causal: the current
  pull-driven renderer cannot tell which interpreter `eClocks` value belongs
  to which rendered tick, and the mixer's fixed input hides the disconnect.
- P3: As a maintainer, I cannot close the deferred Turrican2-TITLE tempo
  observation, because the interpreter provably updates `eClocks` on timing
  control steps while the private tick path discards that value; the fixed
  mixer input makes the interpreter's timing updates observably dead.

Objective
- Deliver a general *private* handoff through the existing private tick path
  so that header-tempo, speed-control, and timeshare interpreter changes to
  `eClocks` can determine the rendered duration of each tick, with automated
  evidence and no broad compatibility claim.

Non-negotiables
- This Track is COMPLETED: the problem, plan, and decisions were confirmed,
  the Move-to-ACTIVE plan step (S1) was checked, and implementation proceeded
  one declared TDD chunk at a time; the completion transition plan step (S5)
  is now checked, and no further implementation is authorized from this
  Track.
- All implementation follows TDD: a focused failing automated test, the
  smallest passing implementation, then refactoring and validation.
- Acceptance must be proven by self-authored automated tests that observe the
  component contract. Supplemental evidence from external modules (direct
  checks, listener judgment) is bounded and never replaces automated coverage.
- Preserve exact-N behavior: the mixer's `pending_frames` plus accumulated
  remainder arithmetic determines the rendered frame count per tick.
- Preserve zero-frame behavior only for the CoreAudio device-request
  no-playback-advance case: when the device requests no frames, the renderer
  must not advance into an invalid state. A zero-frame legacy music tick under
  an accepted timing control is out of the accepted timing-control domain and
  remains unsupported (currently not a normal valid outcome).
- Preserve partial-tick continuity: the remainder must carry across ticks so
  average timing stays exact over multiple ticks.
- Preserve callback allocation/locking restrictions: the CoreAudio callback
  admission path remains lock-free with pre-allocated workspace; no
  allocation, locking, or blocking work is added to the callback path.
- Preserve private/non-reentrant boundaries: playback components remain
  private, single-context, and make no reentrancy or multi-context claim.
- Retain Phase 4's temporary no-broad-compatibility policy: evidence is
  bounded scaffold evidence, not a SynthTracker v1 compatibility promise.

Acceptance criteria
- [x] A1) [P1] A self-authored automated test proves that when the
  interpreter's `eClocks` changes (for example via a timing control step)
  during the bridge tick, the same rendered tick's frame count changes
  accordingly through the private tick path, instead of remaining at the
  fixed-default duration.
- [x] A2) [P1] S3 timing-source coverage through one private handoff
  mechanism: (a) a test-first self-authored speed-control fixture/contract
  proving the default speed duration and then the corrected speed-control
  duration — high-mask pass with low9 16..511 sets `eClocks = 0x1B51F8 /
  low9` on that bridge tick, rendered as the same-tick exact-N duration via
  the existing S2 handoff — with long-run remainder evidence; (b) a separate
  self-authored header-tempo fixture/contract (DELIVERED by this partial-S3
  update: `mdat.header_tempo`/`smpl.header_tempo` and `header_tempo_layout.md`,
  header tempo 100 established in StartSong before the first bridge tick, no
  speed/timeshare control in the fixture, 44.1 kHz component contract
  `1102/1103/1103` with correct carried remainders, test-first red via a
  temporary isolated fixed-context timing mutation `881 != 1102`, workspace
  green `test_playback_context` 53/53 with clean `git diff --check`); (c)
  existing timeshare evidence; and (d) appropriate fake-CoreAudio and
  application-level composition — the speed-source fake-CoreAudio
  workspace/HAL composition and the finite header/speed application
  composition are both DELIVERED by partial-S3 updates (see Current
  inventory), with exact timing-source behavior remaining component-owned;
  full validation is complete (authoritative final S3 evidence: the full
  build `cmake --build build --parallel 2` passed, full `ctest --test-dir
  build --output-on-failure` passed 8/8, and `git diff --check` is clean);
  bounded documentation reconciliation was performed with its outcome
  recorded by this Track update, and no ADR change is needed.
- [x] A3) [P2] Focused automated tests prove exact-N preservation: the
  rendered frame count per tick equals the `eClocks`-derived exact-N result
  and the accumulated remainder is preserved across consecutive ticks;
  zero-frame behavior is preserved only for the CoreAudio device-request
  no-playback-advance case, and a zero-frame legacy music tick under an
  accepted timing control remains out of domain and unsupported.
- [x] A4) [P2, P3] Automated component and application tests, the full CTest
  suite, build validation, and `git diff --check` pass; the CoreAudio
  callback path and lock-free/pre-allocated restrictions are covered or
  expressly assessed unchanged. (DELIVERED, final S3 review GO: the full
  build `cmake --build build --parallel 2` passed, the full CTest suite
  `ctest --test-dir build --output-on-failure` passed 8/8, and
  `git diff --check` is clean; the CoreAudio callback path and
  lock-free/pre-allocated restrictions remain covered by the existing
  callback admission tests and are expressly assessed unchanged by S3.)
- [x] A5) [P3] The six-dimension interface-impact decision is recorded in the
  Decision log (supplied by this DRAFT revision) and gates the Move-to-ACTIVE
  step; after automated self-authored TDD/validation passes, the user performs
  the supplemental manual Turrican2-TITLE check, and only an explicit
  confirmed/not-confirmed/inconclusive outcome is recorded, with no external
  files, paths, module content, or raw controls retained. The manual check is
  never automated acceptance and makes no broad or exact-audio claim.
  (CHECKED, S4 outcome recorded by this Track update: the user's explicit
  supplemental manual outcome is that the previously missing Turrican2-TITLE
  tempo transition is now audible — supplemental user judgment following the
  completed automated evidence, not an automated criterion, an exact-audio
  comparison, or a broad compatibility claim.)
- [x] A6) [P1, P2] Bounded documentation reconciliation is performed and its
  findings recorded in this Track: the PLAYER legacy local-code reference,
  Architecture, ASR evidence, and README/Glossary/ADR review. (DELIVERED,
  final S3 review GO: the six timing docs — README, Architecture, ASR,
  Glossary, AUDIO_RENDERING_DESIGN, and TFMXLegacy PLAYER — were reconciled
  and updated for the delivered state; the four public/current docs —
  README, Architecture, ASR, and AUDIO_RENDERING_DESIGN — were further
  updated after the manual outcome; ADR review found no ADR update or new
  ADR required; no broad compatibility, exact-audio, public API/ABI, or
  target-architecture claim is made.)

Why now / impact
- Turrican2-TITLE's interpreter demonstrably updates `eClocks` (song start
  sets the default, tempo and speed control steps rewrite it, and the
  timeshare step scales it) while `src/playback/playback_context.c:129` passes
  the fixed `14318` to the mixer on every tick. The rendered duration is
  therefore constant regardless of interpreter timing state, which leaves the
  deferred Turrican2-TITLE tempo critique unresolved and makes private timing
  behavior observably wrong for any song using timing controls. CoreAudio is
  not causal: the HAL callback pulls frames, so timing truth must live on the
  private tick side and be handed off explicitly, not inferred at the audio
  boundary.

Scope
- In scope:
  - A general private `eClocks`-value handoff from the interpreter state to
    the mixer's tick-duration computation through the existing private tick
    path.
  - Preservation and automated evidence of exact-N frame counts, accumulated
    remainder continuity, and the CoreAudio device-request zero-frame/
    no-advance behavior.
  - Self-authored automated fixtures and component/application tests that
    exercise header-tempo, speed-control, and timeshare timing changes.
  - A supplemental manual Turrican2-TITLE check performed by the user after
    automated self-authored TDD/validation passes; only the explicit
    confirmed/not-confirmed/inconclusive outcome is recorded, with no external
    files, paths, module content, or raw controls retained, and never as
    automated acceptance or a broad/exact-audio claim.
  - Compatibility impact assessment and the required six-dimension
    interface-impact decision before ACTIVE.
- Out of scope:
  - Any public API/ABI, public header, or target-architecture change.
  - Callback-path, locking, or allocation changes; reentrancy or
    multi-context playback.
  - A mixer redesign, resampler, runtime device-rate-change policy, or
    general real-module loader admission.
  - Audio-quality, exact-audio, effects, loop, or other timing-discrepancy
    work beyond the `eClocks`-to-rendered-duration handoff.
  - External module/sample content as an automated acceptance criterion.

Milestones
- [x] M1) Record the current inventory, confirm the DRAFT baseline, and
  establish the deferred Turrican2-TITLE critique as the tracked evidence
  reference. (CHECKED on the completion transition: the inventory, baseline,
  and critique-reference work was completed during planning, reconciling the
  earlier unchecked oversight.)
- [x] M2) Resolve the open questions and record the complete six-dimension
  interface-impact decision while DRAFT (recorded by the DRAFT revision and
  confirmed by the user; the milestone box is checked on the Move-to-ACTIVE
  transition).
- [x] M3) Move Track 018 to ACTIVE (folder, filename, and title status) via
  the checked Move-to-ACTIVE plan step; the DRAFT decisions (Q1–Q4 and the
  six-dimension interface-impact decision) are recorded and user-confirmed,
  and S1 is executed and checked on this transition.
- [x] M4) Deliver the speed-control source as the S3 chunk through TDD: the
  speed fixture/contract (default then corrected speed duration with long-run
  remainder evidence) and the smallest passing correction to the
  speed-control expression (`eClocks = 0x1B51F8 / low9` for high-mask pass
  and low9 16..511, prescale/mask behavior outside the expression preserved)
  are delivered, with the capacity assessment (bounded divisor 100). The
  separate self-authored header-tempo source fixture/test is also delivered
  by this partial-S3 update, and the speed-source fake-CoreAudio workspace/HAL
  composition with its test-only lifecycle cleanup is delivered by this
  update, and the application-level composition for the finite header/speed
  fixture is delivered by a later partial-S3   update. (DELIVERED, final S3 review GO: full validation is complete — the
  full build `cmake --build build --parallel 2` passed, the full CTest
  suite `ctest --test-dir build --output-on-failure` passed 8/8, and
  `git diff --check` is clean — and bounded documentation reconciliation was
  performed with its outcome recorded by this Track update; no ADR change is
  needed.)
- [x] M5) Complete full validation, the supplemental manual Turrican2-TITLE
  check under user approval, and completion evidence (application-level
  composition is delivered by the partial-S3 update recorded in Current
  inventory); the user's recorded check outcome is never automated
  acceptance and makes no broad or exact-audio claim. (CHECKED on the
  completion transition: application composition, the full build, CTest 8/8,
  the manual Turrican2-TITLE outcome, and the completion evidence are
  complete.)
- [x] M6) Perform bounded documentation reconciliation (PLAYER legacy
  local-code reference, Architecture, ASR evidence, README/Glossary/ADR
  review) and record the findings in this Track. The review is performed by
  this Track update and its outcome is recorded under Artifacts: the six
  timing docs — README, Architecture, ASR, Glossary,
  AUDIO_RENDERING_DESIGN, and TFMXLegacy PLAYER — were reconciled and
  updated for the delivered state, the four public/current docs — README,
  Architecture, ASR, and AUDIO_RENDERING_DESIGN — were further updated
  after the manual outcome, and ADR review found no ADR update or new ADR
  required. (CHECKED on the completion transition: the bounded documentation
  reconciliation is recorded and reviewed; no ADR update is required.)

Risks / decisions
- Risk: The mixer currently computes tick duration from its `eclocks`
  argument; changing the caller introduces a behavior change that must stay
  bounded to the private tick path and preserve exact-N arithmetic.
- Risk: `eClocks` is a legacy global; reading it after the interpreter tick
  is only valid on the private single-context path and must not become a
  reentrancy or callback-path claim.
- Risk: The CoreAudio callback is not causal, so no handoff may rely on
  callback-observed state; tick timing truth must remain on the private tick
  side.
- Risk (resolved by Decision Q1): A timing-control step may change `eClocks`
  mid-tick; the same-tick application is defined (interpreter update →
  duration calculation → mix) and is proven with an automated test.
- Decision (Q1): An `eClocks` update made during the bridge tick determines
  the same rendered tick; the existing/historical order (interpreter update →
  duration calculation → mix) applies, and the same-tick contract is proven
  with an automated test.
- Decision (Q2): The bridge tick returns the current `eClocks` together with
  that tick's snapshots; the bridge remains the sole private owner of direct
  legacy global access, the context forwards timing, and the mixer retains its
  explicit timing argument.
- Decision (Q3): Zero-frame behavior is preserved only for the CoreAudio
  device-request/no-playback-advance case; no promise remains that an accepted
  legacy timing control can produce a valid zero-frame music tick, which stays
  out of the accepted timing-control domain and remains unsupported.
- Decision (Q4): The supplemental Turrican2-TITLE check is a manual check the
  user performs after automated self-authored TDD/validation passes; only the
  explicit confirmed/not-confirmed/inconclusive outcome is recorded, never as
  automated acceptance or a broad/exact-audio claim.
- Decision (six-dimension impact): C API/ABI unchanged; module
  compatibility/extension changed as a bounded playback compatibility
  correction with no format/admission alteration and no broad claim;
  interpreter/timing/audio changed — same-tick post-interpreter `eClocks`
  affects the exact-N tick duration while remainder/partial-tick continuity is
  preserved and device-zero behavior is unchanged; persistent DAW
  format/versioning unchanged; platform/audio-output adapter unchanged;
  component/package boundaries changed as a private bridge-capture and
  context-forwarding refinement with no public, callback, reentrancy, or
  target-boundary change.
- Risk (S3 blocker, discovered and resolved): the former `src/player.c:773`
  speed-control expression `!(l[3]&0xF200)&&(x=(l[3]&0x1FF)>0xF)` assigned
  the boolean comparison result to `x`, so a qualifying speed control used
  divisor 1 (`eClocks = 0x1B51F8 / 1`) instead of the masked low-nine-bit
  divisor `l[3]&0x1FF`; the rendered tick was then ~1,790,456 clock units
  long instead of the documented `0x1B51F8 / low9` duration. The delivered
  S3 correction restores `eClocks = 0x1B51F8 / low9` in that one expression
  for high-mask pass and low9 16..511; the completed S2 handoff renders it
  same-tick, and high-mask/prescale behavior outside the expression is
  unchanged. This was separate from the S2 handoff and from the
  source/docs discrepancy.
- Decision (S3 corrective contract, user-approved, implemented): where the
  high mask passes (`(l[3]&0xF200) == 0`) and low9 (`l[3]&0x1FF`) is
  16..511, the speed control sets `eClocks = 0x1B51F8 / low9` during that
  bridge tick; the existing S2 handoff renders the same-tick exact-N
  duration. Prescale/mask behavior outside this expression is preserved.
- Decision (S3 placement): the speed-control expression correction belongs
  within Track 018 S3/A2, not a new Track; it changes no public API/ABI,
  persistence, platform-adapter, or component-boundary contract.
- Risk (capacity, assessed): valid small divisors (low9 near 16) yield large
  `eClocks` and therefore larger rendered ticks. The delivered capacity
  assessment shows every valid low9 16..511 speed divisor uses at most
  `eClocks = 111903` at divisor 16, safely bounded at no more than 6894
  frames/tick at 44.1 kHz or 7503 at 48 kHz including remainder — below the
  existing 65536 production application/tick capacity. This does not promise
  every module is compatible; the S3 test fixture uses the bounded divisor
  100.
- Decision (six-dimension impact, S3 speed correction): C API/ABI unchanged —
  the correction lives inside the private legacy module with no public
  interface change. Module compatibility/extension changed as a bounded
  correction — the speed-control timing expression is corrected so the
  documented low-nine-bit divisor contract applies, with no format/admission
  alteration and no broad compatibility claim. Interpreter/timing/audio
  changed as a bounded correction — a qualifying speed control now yields
  `eClocks = 0x1B51F8 / low9` on its bridge tick and the existing S2 handoff
  renders the same-tick exact-N duration, replacing the boolean-divisor
  behavior. Persistent DAW format/versioning unchanged. Platform/audio-output
  adapter unchanged — CoreAudio adapter and facade untouched. Component/
  package   boundaries unchanged — the correction stays within the existing
  private legacy module and Track 018's S3/A2 step, with no public, callback,
  reentrancy, or target-boundary change. This decision is implemented by the
  delivered S3 correction.

Open questions
- [x] Q1) RESOLVED: An `eClocks` update made during the bridge tick determines
  the same rendered tick; the existing/historical order (interpreter update →
  duration calculation → mix) means the updated value feeds the mixer before
  it computes the same tick's duration, and the same-tick contract is proven
  with an automated test.
- [x] Q2) RESOLVED: The bridge tick returns the current `eClocks` together
  with that tick's snapshots; the bridge remains the sole private owner of
  direct legacy global access, the private context forwards timing to the
  mixer, and the mixer retains its explicit timing argument.
- [x] Q3) RESOLVED: Zero-frame behavior is preserved only for the CoreAudio
  device-request/no-playback-advance case; every promise that an accepted
  legacy timing control could produce a valid zero-frame music tick is
  removed/reworded. A zero-frame legacy music tick is out of the accepted
  timing-control domain and remains unsupported (currently not a normal valid
  outcome).
- [x] Q4) RESOLVED: After automated self-authored TDD/validation passes, the
  user performs the supplemental manual Turrican2-TITLE check; only the
  explicit confirmed/not-confirmed/inconclusive outcome is recorded, with no
  external files, paths, module content, or raw controls retained. It is never
  automated acceptance and makes no broad or exact-audio claim.

Decision log
- Decision (status): Track 018 is COMPLETED. Implementation was authorized
  once the Move-to-ACTIVE plan step (S1) was checked and proceeded one
  declared TDD chunk at a time; the completion transition (S5) now records
  the move to COMPLETED and authorizes no further implementation.
- Decision (scope): The objective is a general private `eClocks` handoff so
  header-tempo, speed-control, and timeshare changes determine rendered tick
  duration; this is not a timing-model redesign or a compatibility promise.
- Decision (evidence): Self-authored automated tests are the acceptance
  evidence; any external-module direct check is bounded supplemental evidence
  only and records no module bytes, raw module values, or external file paths.
- Decision (Q1): An `eClocks` update made during the bridge tick determines
  the same rendered tick; the existing/historical order is interpreter update
  → duration calculation → mix.
- Decision (Q2): The bridge tick returns the current `eClocks` together with
  that tick's snapshots; the bridge remains the sole private owner of direct
  legacy global access, the context forwards timing, and the mixer retains
  its explicit timing argument.
- Decision (Q3): Zero-frame behavior is preserved only for the CoreAudio
  device-request/no-playback-advance case; every promise that an accepted
  legacy timing control could produce a valid zero-frame music tick is
  removed/reworded. A zero-frame legacy music tick is out of the accepted
  timing-control domain and remains unsupported (currently not a normal valid
  outcome).
- Decision (Q4): After automated self-authored TDD/validation passes, the
  user performs the supplemental manual Turrican2-TITLE check; only the
  explicit confirmed/not-confirmed/inconclusive outcome is recorded, with no
  external files, paths, module content, or raw controls retained, and it is
  never automated acceptance or a broad/exact-audio claim.
- Decision (status transition, user-approved): The user approved the
  Move-to-ACTIVE transition of Track 018 after confirming the recorded DRAFT
  decisions — the Q1–Q4 open-question resolutions and the six-dimension
  interface-impact decision. S1 is executed and checked; this transition
  claims no implementation, test, or validation work.
- Decision (six-dimension impact): C API/ABI unchanged — the handoff is
  entirely private with no public interface change. Module
  compatibility/extension changed — a bounded playback compatibility
  correction (timing controls now drive rendered duration), with no format or
  admission alteration and no broad compatibility claim. Interpreter/timing/
  audio changed — same-tick post-interpreter `eClocks` affects the exact-N
  tick duration, while remainder/partial-tick continuity is preserved and
  device-zero behavior is unchanged. Persistent DAW format/versioning
  unchanged — no format or version impact. Platform/audio-output adapter
  unchanged — CoreAudio callback admission, locking, and allocation behavior
  are untouched. Component/package boundaries changed — a private
  bridge-capture/context-forwarding refinement, with no public, callback,
  reentrancy, or target-boundary change.
- Decision (S3 blocker, discovered and resolved): the `src/player.c:773`
  speed-control expression assigned the boolean comparison result to `x`, so
  qualifying speed controls used divisor 1 instead of the masked
  low-nine-bit divisor; the delivered S3 correction restores
  `eClocks = 0x1B51F8 / low9` for high-mask pass and low9 16..511. This was
  separate from the completed S2 handoff and from the source/docs
  discrepancy.
- Decision (S3 corrective contract, user-approved): where the high mask passes
  and low9 is 16..511, the speed control sets `eClocks = 0x1B51F8 / low9`
  during that bridge tick; the existing S2 handoff renders the same-tick
  exact-N duration. Prescale/mask behavior outside this expression is
  preserved.
- Decision (S3 placement): the correction belongs within Track 018 S3/A2, not
  a new Track; no public API/ABI, persistence, platform-adapter, or
  component-boundary change is involved.
- Decision (S3 evidence): S3 requires a test-first self-authored speed
  fixture/contract (default then corrected speed duration and long-run
  remainder evidence), a separate self-authored header-tempo fixture/contract,
  existing timeshare evidence, appropriate fake-CoreAudio and application
  composition, full validation, and bounded documentation reconciliation
  (PLAYER legacy local-code reference, Architecture, ASR evidence,
  README/Glossary/ADR review). The speed fixture/contract, the speed
  expression correction, the capacity assessment, and the header-tempo
  source fixture/test are delivered by this partial-S3 Track update; the
  speed-source fake-CoreAudio workspace/HAL composition and its test-only
  lifecycle cleanup are also delivered by this update. Application-level
  composition, full validation, and documentation reconciliation remain. No
  documentation is claimed updated and no full-build/full-CTest claim is
  made by this update — local to this partial update only, not a statement
  about the final Track state.
- Decision (S3 capacity risk): valid small divisors can create larger ticks;
  the delivered capacity assessment bounds every valid low9 16..511 speed
  divisor at no more than 6894 frames/tick at 44.1 kHz or 7503 at 48 kHz
  including remainder (max `eClocks` 111903 at divisor 16), below the 65536
  production application/tick capacity, without promising every module is
  compatible; the test fixture uses the bounded divisor 100.
- Decision (six-dimension impact, S3 speed correction): C API/ABI unchanged —
  the correction lives inside the private legacy module with no public
  interface change. Module compatibility/extension changed as a bounded
  correction — the speed-control timing expression is corrected so the
  documented low-nine-bit divisor contract applies, with no format/admission
  alteration and no broad compatibility claim. Interpreter/timing/audio
  changed as a bounded correction — a qualifying speed control now yields
  `eClocks = 0x1B51F8 / low9` on its bridge tick and the existing S2 handoff
  renders the same-tick exact-N duration, replacing the boolean-divisor
  behavior. Persistent DAW format/versioning unchanged — no format or version
  impact. Platform/audio-output adapter unchanged — CoreAudio adapter and
  facade untouched. Component/package boundaries unchanged — the correction
  stays within the existing private legacy module and Track 018's S3/A2 step,
  with no public, callback, reentrancy, or target-boundary change.
  Implemented by the delivered S3 correction.

Plan (execution steps)
- [x] S1) Move Track 018 to ACTIVE (folder, filename, and title status) only
  after the recorded open-question resolutions (Q1–Q4) and the six-dimension
  interface-impact decision, supplied by this DRAFT revision, are confirmed
  by the user, and any remaining DRAFT gate is cleared. This step stays
  unchecked while DRAFT; ACTIVE implementation begins only when S1 is
  executed and checked.
- [x] S2) Add a focused failing automated test proving that an interpreter
  `eClocks` change made during the bridge tick alters the same rendered tick's
  frame count through the private tick path; implement the smallest private
  handoff that passes it; refactor and validate exact-N, remainder continuity,
  and the CoreAudio device-request zero-frame/no-advance behavior; update this
  Track. S2 is delivered; the completed evidence is recorded in Current
  inventory. S4 is the next unchecked plan step.
- [x] S3) Deliver the timing-control sources with test-first evidence:
  (a) a self-authored speed-control fixture/contract proving the default
  speed duration and then the corrected speed-control duration
  (`eClocks = 0x1B51F8 / low9` for high-mask pass and low9 16..511) with
  long-run remainder evidence, implemented as the smallest passing correction
  to the speed-control expression at `src/player.c:773` with prescale/mask
  behavior outside that expression preserved; the fixture uses a bounded
  divisor and broader-range capacity assessment precedes acceptance — (a) and
  the capacity assessment are DELIVERED (partial S3, recorded by this Track
  update): focused TDD red/green evidence and current focused results
  (playback 53/53, renderer 4/4, focused CTest 2/2, clean `git diff --check`);
  no full build/full CTest or docs reconciliation claim. (b) the separate
  self-authored header-tempo fixture/contract is DELIVERED (partial S3,
  recorded by this Track update): `mdat.header_tempo`/`smpl.header_tempo`
  with `header_tempo_layout.md` (self-authored, no copied content or external
  paths); header tempo 100 is established in StartSong before the first
  bridge tick and no speed/timeshare control exists in the fixture; the 44.1
  kHz component contract renders three active ticks `1102/1103/1103` with
  correct carried remainders, distinct from the fixed-default `881` behavior;
  test-first evidence is a temporary isolated fixed-context timing mutation
  (only the mixer timing input forced to 14318, never the workspace) giving
  the causal `881 != 1102` failure; the workspace is green with
  `test_playback_context` 53/53 and a clean `git diff --check`. (c) reuse of
  existing timeshare evidence (unchanged S2 evidence) applies; (d) the
  speed-source fake-CoreAudio workspace/HAL composition is DELIVERED (partial
  S3, recorded by this Track update): the dynamic workspace/HAL test drives
  the self-authored speed fixture at 44.1 kHz through the fake facade →
  adapter → coordinator → renderer path, the workspace zero request bypasses
  the renderer with no tick advance, and nonzero native-buffer requests
  consume the default 881 then two corrected speed 1103 ticks exactly, with
  active playback and zero adapter allocations proven; dynamic
  fake-CoreAudio test state is teardown-owned (stop/quiesce/dispose before
  playback destroy, observer reset) as test-only lifecycle cleanup — test
  reliability only, no product/CoreAudio production behavior change; and the
  application-level composition is DELIVERED (partial S3, recorded by this
  Track update): the self-authored finite header/speed fixture
  (`mdat.header_speed_finite`/`smpl.header_speed_finite` with
  `header_speed_finite_layout.md`) runs through the application → fake
  CoreAudio workspace/HAL route with application success, the rate-0
  startup request, negotiated 44.1 kHz preparation, and the clean
  open/configure/prepare/bind/start/stop/quiesce/dispose lifecycle; exact
  timing-source behavior remains component-owned — the application level
  records no per-tick timing claims;
  then refactor and run full validation (focused suites, full CTest, build,
  `git diff --check`). Bounded documentation reconciliation is performed by
  this Track update and its outcome recorded under Artifacts (the six
  timing docs reconciled and updated; the four public/current docs further
  updated after the manual outcome; ADR review found no ADR update or new
  ADR required). Full
  validation is complete (authoritative final evidence: `cmake --build
  build --parallel 2` passed, full `ctest --test-dir build
  --output-on-failure` passed 8/8, `git diff --check` clean). S3 is
  delivered; S4 is the next unchecked plan step.
- [x] S4) After automated self-authored TDD/validation passes, the user
  performs the supplemental manual Turrican2-TITLE check under user approval;
  record only the explicit confirmed/not-confirmed/inconclusive outcome (never
  automated acceptance or a broad/exact-audio claim), assess and record
  documentation impact, and complete only after acceptance. (CHECKED, this
  Track update: the user performed the check and the explicit outcome is
  recorded — the previously missing Turrican2-TITLE tempo transition is now
  audible, as supplemental user judgment following the completed automated
  evidence and not an automated criterion, an exact-audio comparison, or a
  broad compatibility claim. Documentation impact is assessed and recorded
  as of this update: the existing bounded Track 018 docs then stated the
  correction and the pending manual boundary, so no additional document
  mutation was performed by this manual outcome — a statement local to this
  update, not a description of the final Track state. Final Track
  completion, critique lifecycle action, roadmap reconciliation, and Git
  workflow remain separate pending steps.)
- [x] S5) Completion transition: on acceptance, move Track 018 to COMPLETED
  (folder, filename, and title status), update `Current path`, reconcile the
  earlier unchecked M1 box, check M5 and M6, record the completion evidence
  and the roadmap/critique reconciliation, and check this completion-
  transition step. No commit or push occurs; this transition edits the Track
  file only and authorizes no further implementation from this Track.

Current inventory
- `src/player.c` (legacy interpreter) updates `eClocks` on timing control
  steps: the speed step recomputes it from the step value, the timeshare step
  scales it, and song start and tempo handling set it. These updates are
  observable interpreter state, not rendered-tick evidence. S3 blocker
  (discovered and fixed by the delivered S3 correction): the former speed
  step expression at `src/player.c:773`
  `!(l[3]&0xF200)&&(x=(l[3]&0x1FF)>0xF)` assigned the boolean comparison
  result to `x`, so a qualifying speed control set `eClocks = 0x1B51F8 / 1`
  instead of `0x1B51F8 / (l[3]&0x1FF)`. The delivered one-expression
  correction restores `eClocks = 0x1B51F8 / low9` for high-mask pass and
  low9 16..511, so the user-approved corrective contract now holds: the speed
  control sets `eClocks = 0x1B51F8 / low9` during that bridge tick, the
  existing S2 private handoff renders the same-tick exact-N duration (audible
  same-tick), and high-mask/prescale behavior outside the expression is
  unchanged.
- `src/playback/playback_legacy_bridge.c` owns the legacy global reset and
  restores `eClocks = 14318` on reset; it remains the sole private owner of
  direct legacy global access. The bridge tick now captures the current
  `eClocks` after the interpreter tick and returns it together with that
  tick's snapshots; this is the delivered private handoff of S2.
- `src/playback/playback_context.c` receives the tick's `eClocks` from the
  bridge and forwards it to the existing `tfmx_playback_legacy_mixer_begin_tick`
  timing argument on each tick, replacing the previous fixed `14318`; the
  same-tick contract (interpreter update → duration calculation → mix) is
  verified by the focused same-tick test. The mixer's `eclocks` argument and
  its exact-N computation are unchanged.
- `src/playback/playback_legacy_mixer.c` converts the `eclocks` argument and
  output rate into an exact-N `pending_frames` count with a carried `remainder`
  (partial-tick continuity); it retains its explicit timing argument and its
  production code is unchanged by S2.
- `src/playback/playback_legacy_renderer.c` pulls ticks when the CoreAudio
  callback requests frames and treats a zero-frame tick as a stop condition;
  it is pull-driven and not causal with respect to CoreAudio. Per Q3 the
  preserved zero-frame behavior is only the CoreAudio device-request
  no-playback-advance case; a zero-frame legacy music tick under an accepted
  timing control is out of domain and remains unsupported.
- `src/audio_output/adapters/coreaudio_adapter.c` and
  `src/audio_output/adapters/coreaudio_facade.c` statically assert lock-free
  callback admission; the callback path uses a pre-allocated workspace and
  must not allocate or lock. The CoreAudio production path is unchanged by S2:
  the handoff lives entirely on the private tick side.
- Playback components are private, single-context, and non-reentrant; no
  public API/ABI, callback-path, or multi-context claim exists.
- The deferred Turrican2-TITLE tempo critique is tracked in project memory;
  per Q4 its resolution is a supplemental manual check performed by the user
  after automated self-authored TDD/validation passes, recording only the
  explicit confirmed/not-confirmed/inconclusive outcome with no external
  files, paths, module content, or raw controls retained; it is never
  automated acceptance or a broad/exact-audio claim.
- Self-authored fixture evidence (S2): the focused playback-context test
  drives `tests/fixtures/mdat.timeshare` and `smpl.timeshare` at 44.1 kHz and
  verifies the rendered frame sequence default → timeshare → active dynamic:
  tick frames `881`/`1764`/`1764` with carried remainders
  `353545`/`344725`/`335905`; the inert hold trackstep keeps tick 3 active
  (the engine is not complete after it). `tests/fixtures/timeshare_layout.md`
  declares the fixture self-authored with no copied module or sample content
  and a scope bounded to the private playback boundary.
- Dynamic fake CoreAudio workspace/HAL evidence (S2): the audio-output
  composition test drives the same fixture through the fake CoreAudio facade
  → adapter → coordinator → renderer path. A zero-frame device request is
  accepted with no renderer invocation and no tick advance; nonzero requests
  deliver into a native output buffer; exact-N delivery and partial-tick
  retention hold across the dynamic `881`/`1764`/`1764` ticks; the test
  asserts pre-allocated workspace and zero allocation attempts.
- TDD red evidence (S2, condensed): the initial missing-handoff test produced
  `881` frames where the dynamic timeshare duration was intended; the
  correction assertions failed with an immediate stop before dynamic tick
  advancement; a workspace-only generic nonzero request failed without a
  native output buffer; green was reached only after the bounded production
  and test changes.
- Validation evidence (S2): `git diff --check` is clean; the full build
  succeeded (all 8 targets); `test_playback_context` passed 51/51 and
  `test_legacy_exact_renderer` passed 3/3; the focused CTest run of those two
  targets passed 2/2. The full CTest suite was not run.
- Self-authored fixture evidence (S3 speed subchunk, partial): the focused
  playback-context test drives `tests/fixtures/mdat.speed` and
  `smpl.speed` at 44.1 kHz. `tests/fixtures/speed_layout.md` declares the
  fixture self-authored with no copied module or sample content and no
  external paths. Component contract: the default speed renders 881 frames
  per tick; after the corrected speed control, ticks render 1103 frames with
  a later 1102 remainder-carry tick, during active playback. Against the old
  divisor-1 expression the focused test failed with insufficient output
  capacity (the ~1,790,456-clock tick exceeded the renderer's output
  capacity), proving the red leg; it passed after the one-expression
  correction restored `eClocks = 0x1B51F8 / low9`.
- Self-authored fixture evidence (S3 header-tempo subchunk, delivered): the
  focused playback-context test drives `tests/fixtures/mdat.header_tempo` and
  `smpl.header_tempo` at 44.1 kHz. `tests/fixtures/header_tempo_layout.md`
  declares the fixture self-authored with no copied module or sample content
  and no external file paths. Header tempo 100 (`>= 0x10`) is established in
  StartSong (`eClocks = 0x1B51F8 / 100 = 17904`, prescale 0) before the first
  bridge tick; the fixture contains no speed or timeshare control. Component
  contract at 44.1 kHz: three active ticks render `1102`/`1103`/`1103` frames
  with correct carried remainders (`316790`/`275625`/`234460`), each tick
  active (engine not complete), distinctly above the fixed-default `14318`
  behavior of `881` frames/tick. Test-first evidence: a temporary isolated
  fixed-context timing mutation (only the mixer timing input forced to 14318,
  never the workspace) produced the causal `881 != 1102` failure; the
  workspace is green with `test_playback_context` 53/53 and a clean
  `git diff --check`. No full build, full CTest, or documentation
  reconciliation claim is made by this update.
- Capacity assessment (S3 speed subchunk): every valid low9 16..511 speed
  divisor uses at most `eClocks = 111903` at divisor 16; the rendered tick is
  safely bounded at no more than 6894 frames/tick at 44.1 kHz or 7503 at
  48 kHz including remainder, below the existing 65536 production
  application/tick capacity. This does not promise every module is
  compatible; the focused test uses the bounded divisor 100.
- Validation evidence (S3 speed, header-tempo, and speed workspace/HAL
  subchunks, current focused results): the focused playback test passed
  53/53 (52/52 for the speed subchunk plus the delivered header-tempo test),
  the focused renderer test passed 4/4 (the prior 3/3 plus the delivered
  speed-source workspace/HAL composition test), and the focused CTest run of
  those two targets passed 2/2; `git diff --check` is clean. No full build,
  full CTest, or documentation reconciliation claim is made by this
  partial-S3 update.
- Dynamic fake CoreAudio workspace/HAL evidence (S3 speed subchunk,
  delivered): the audio-output composition test reuses the self-authored
  `tests/fixtures/mdat.speed`/`smpl.speed` fixture at 44.1 kHz through the
  fake CoreAudio facade → adapter → coordinator → renderer path in
  workspace/HAL mode. The zero-frame device request is accepted with no
  renderer invocation and no tick advance; nonzero native-buffer requests
  consume the default 881 then two corrected speed 1103 ticks exactly
  (request trace 0/500/1024/460/1024/79 frames, tick advances 0/1/2/2/3/3,
  retained offsets 0/500/643/1103/1024/1103); the engine remains active (not
  complete) on the dynamic speed ticks, and the test asserts the
  pre-allocated workspace with zero allocation attempts.
- Test-only lifecycle cleanup (S3 dynamic workspace/HAL tests): dynamic
  fake-CoreAudio test state is teardown-owned; on assertion failure after
  start, the teardown best-effort stops/quiesces/disposes the adapter
  instance before destroying the playback context and resets the observers,
  avoiding cross-test contamination. This is test reliability only; no
  product or CoreAudio production behavior changes.
- Test-only fake request-trace repair evidence (delivered by this Track
  update): the audio-output test
  `coreaudio_request_trace_is_bounded_and_truncation_is_observable` drives
  distinct frame counts through the fake facade/adapter with an unbounded
  sink. The first eight callback requests (5, 2, 8, 1, 4, 7, 3, 6) are
  retained in order: `render_request_count` and `request_trace_count` both
  equal 8, `request_trace_truncated` is false, and the retained prefix
  matches the requested order. At the ninth request the trace remains capped
  at 8 (`request_trace_count` unchanged) while `render_request_count`
  increments to 9 and `request_trace_truncated` becomes true; the retained
  prefix is unchanged. Later requests keep incrementing the total (11) with
  the retained prefix and truncation flag unchanged, and the render and
  delivery call counts equal the total at every stage — no callback is
  skipped. Re-initializing the facade resets `render_request_count` to 0,
  `request_trace_count` to 0, and `request_trace_truncated` to false.
  `render_request_count` is the total admitted/attempted callback requests
  serviced by the fake, not a successful-delivery count; render/delivery
  counts are asserted separately. This is test-only fake-facade repair: no
  product behavior, fixture, or fixture layout is changed.
- Application-level composition evidence (S3 application subchunk,
  delivered): the application test
  `test_application_header_speed_finite_lifecycle` drives the self-authored
  finite fixture `tests/fixtures/mdat.header_speed_finite`/`smpl.header_speed_finite`
  (documented in `tests/fixtures/header_speed_finite_layout.md`, self-authored
  with no copied module or sample content and no external file paths)
  through the private application → fake CoreAudio workspace/HAL route.
  `application_run` succeeds; the startup selection requests rate 0, the fake
  reports 44.1 kHz, the preparation observer records the negotiated 44.1 kHz
  renderer rate with zero requests before preparation, and the clean
  open/configure/prepare/bind/start lifecycle is followed by
  stop/quiesce/dispose with the fake left inactive, quiescent, and disposed.
  The fixture's header tempo 100 and qualifying speed control (low9 divisor
  100) both derive `eClocks = 17904`, so the finite flow exercises both
  timing sources and then completes via the stop step. The application level
  asserts composition only — success, rate-0 request, negotiated 44.1 kHz
  preparation, clean lifecycle, and teardown — and records no per-tick
  timing claims; per-tick frame counts remain owned by the playback and
  renderer component tests.
- Validation evidence (S3 application composition and test-only trace
  repair subchunks, current focused results, delivered by this Track
  update): the application test passed 6/6 (including the delivered finite
  header/speed lifecycle test), the audio-output test passed 37/37
  (including the delivered bounded request-trace test), the playback test
  passed 53/53, and the legacy-renderer test passed 4/4; the focused CTest
  run of those four targets passed 4/4 and `git diff --check` is clean. No
  full build, full CTest, or documentation reconciliation claim is made by
  this partial-S3 update.
- Validation evidence (S3 full validation, final review GO, delivered by
  this Track update): the authoritative full validation passed — the full
  build `cmake --build build --parallel 2` succeeded, the full CTest suite
  `ctest --test-dir build --output-on-failure` passed 8/8, and
  `git diff --check` is clean. The final S3 review GO is recorded with no
  findings: S3 delivered the bounded private same-tick `eClocks` behavior
  with header-tempo, speed-control, and timeshare evidence; documentation
  reconciliation is complete (the six timing docs reconciled and updated,
  the four public/current docs further updated after the manual outcome,
  and ADR review found no ADR update or new ADR required). No broad
  compatibility, exact-audio, public API/ABI, or target-architecture claim
  is made. As of this update, the user's manual Turrican2-TITLE check
  outcome (S4) was still required before A5/M5/Track completion; roadmap
  reconciliation remained completion work.
- Deferred (remaining S3 work, timeshare S2 evidence unchanged): S2
  delivered timeshare-only evidence, which remains unchanged. The
  speed-control source subchunk of S3 is delivered (fixture, correction,
  capacity assessment, focused red/green evidence, current focused results),
  and the header-tempo source subchunk of S3 is delivered (fixture, layout
  doc, component contract, test-first evidence, current focused results),
  and the speed-source fake-CoreAudio workspace/HAL composition subchunk of
  S3 is delivered (dynamic workspace/HAL evidence and test-only lifecycle
  cleanup, current focused results), and the application-level composition
  subchunk of S3 is delivered (finite header/speed fixture through the
  application → fake workspace/HAL route; see the application-composition
  bullet above). Bounded documentation reconciliation is performed by this
  Track update and its outcome recorded under Artifacts: the six timing
  docs (`README.md`, `docs/ARCHITECTURE.md`, `docs/ASR.md`,
  `docs/GLOSSARY.md`, `docs/AUDIO_RENDERING_DESIGN.md`,
  `docs/TFMXLegacy/PLAYER.md`) were reconciled and updated for the
  delivered state — the private same-tick post-interpreter `eClocks` handoff,
  the local speed low9 correction, and self-authored primary evidence — with
  the supplemental manual Turrican2-TITLE check outcome recorded and no
  broad compatibility/exact-audio/public API/target-architecture claim; the
  four public/current docs (`README.md`, `docs/ARCHITECTURE.md`,
  `docs/ASR.md`, `docs/AUDIO_RENDERING_DESIGN.md`) were further updated
  after the manual outcome, and ADR review found no ADR update or new ADR
  required. Full validation is complete (authoritative final S3 evidence:
  `cmake --build build --parallel 2` passed, full `ctest --test-dir build
  --output-on-failure` passed 8/8, `git diff --check` clean). Exact
  timing-source behavior remains component-owned. S4 and A5 are checked by
  this Track update (the user's supplemental manual Turrican2-TITLE check
  outcome is recorded: the previously missing Turrican2-TITLE tempo
  transition is now audible, as supplemental user judgment following the
  completed automated evidence and not an automated criterion, an
  exact-audio comparison, or a broad compatibility claim; no external files,
  paths, module content, or raw controls are retained). M5 and Track
  completion remain pending as of this update (never automated acceptance
  and no broad or exact-audio claim); critique closure and full completion
  remain deferred, and roadmap reconciliation remained completion work.

Artifacts
- Roadmap-derived work: project memory `synthtracker/roadmaps`, canonical
  index "SynthTracker modernization roadmap" (revision 26) and "Phase 4 —
  Component extraction" (revision 19), Stage 4 Loader and Player extraction,
  which remains in progress with deferred playback-detail work outside
  Track 017. Roadmap reconciliation was completed on this transition: Stage 4
  is still in progress and no next Track is drafted.
- Critique: project memory `synthtracker/critiques`, "Turrican-TITLE tempo
  change is missing" — the deferred playback-detail observation this Track
  addressed; per Q4 the user's supplemental manual check outcome was recorded
  by S4, and on this completion transition the critique is archived at
  lifecycle revision 2.
- Historical context: `.backlog/COMPLETED/2026/TRACK_017_COMPLETED_selected_subsong_support.md`
  (Stage 4 predecessor) and
  `.backlog/COMPLETED/2026/TRACK_016_COMPLETED_bounded_structural_loader_admission.md`
  (deferred playback observations).
- Timing reference: `docs/TFMXLegacy/PLAYER.md` and `docs/TFMXLegacy/AUDIO.md`
  document the `eClocks` timing model; `docs/AUDIO_RENDERING_DESIGN.md`
  records the audio-rendering and Phase 4 reconciliation context.
  `docs/TFMXLegacy/PLAYER.md` line 65 documents the intended low-nine-bit
  divisor contract (`0x1B51F8 / (l[3] & 0x1FF)`), which the delivered
  `src/player.c:773` correction now implements. The docs discrepancy is
  resolved by the performed documentation reconciliation (recorded below):
  PLAYER remains consistent with the corrected local source.
- Documentation reconciliation (performed, outcome recorded): the six
  timing docs — `README.md`, `docs/ARCHITECTURE.md`, `docs/ASR.md`,
  `docs/GLOSSARY.md`, `docs/AUDIO_RENDERING_DESIGN.md`, and
  `docs/TFMXLegacy/PLAYER.md` — were reconciled and updated against the
  delivered Track 018 state and remain consistent with the bounded result:
  the private same-tick post-interpreter `eClocks` handoff, the local speed
  low9 correction, and self-authored primary evidence, with the supplemental
  manual Turrican2-TITLE check outcome recorded and no broad compatibility,
  exact-audio, public API/ABI, or target-architecture claim. The four
  public/current docs — `README.md`, `docs/ARCHITECTURE.md`, `docs/ASR.md`,
  and `docs/AUDIO_RENDERING_DESIGN.md` — were further updated after the
  manual outcome. ADR review found no ADR update and no new ADR required.

Completion notes
- Roadmap reconciliation (recorded on this transition): the living roadmap
  was reconciled — Phase 4 roadmap revision 19 and canonical index revision
  26, with Stage 4 still in progress and no next Track drafted; the critique
  is archived at lifecycle revision 2.
- The DRAFT revision recorded the user-approved planning decisions only: the
  Q1–Q4 resolutions and the six-dimension interface-impact decision. This
  Track update records the delivered S2 handoff and its focused evidence
  (same-tick post-interpreter `eClocks` → mixer timing argument; exact-N,
  remainder continuity, and device-request zero-frame behavior verified by
  self-authored tests). Application-level coverage, full CTest, documentation
  impact, and completion remain deferred; the Turrican2-TITLE check outcome
  and documentation notes are recorded only after the user performs the
  manual check.
- This Track update records S2 evidence only and changes no source, test,
  fixture, documentation, configuration, project memory, or Git history
  (local to this update, not a statement about the final Track state);
  it authorizes no further implementation beyond the stated ACTIVE step
  discipline.
- This Track update (user-approved, partial S3) records the delivered
  speed-control source subchunk of S3/A2: the one-expression `src/player.c:773`
  correction that restores `eClocks = 0x1B51F8 / low9` for high-mask pass and
  low9 16..511 (the former expression assigned the boolean comparison result
  to `x`, so qualifying speed controls used divisor 1), the self-authored
  `mdat.speed`/`smpl.speed` fixture with `speed_layout.md` (no copied content
  or external paths), the focused TDD red/green evidence (the test failed on
  the old expression via insufficient output capacity and passed after the
  correction), the capacity assessment (max `eClocks` 111903 at divisor 16;
  ≤6894 frames/tick at 44.1 kHz or 7503 at 48 kHz including remainder, below
  the 65536 production application/tick capacity; bounded test divisor 100),
  and the current focused results (playback 52/52, renderer 3/3, focused
  CTest 2/2, `git diff --check` clean). The docs discrepancy is resolved by
  the performed documentation reconciliation (recorded by the review-GO Track
  update below): PLAYER remains consistent with the corrected local source.
  No new box is
  checked: S3, A2, A4–A6, and M4–M6 remain unchecked; S3 remains the next
  unchecked implementation step with its remaining work (speed-source
  fake-CoreAudio and application composition, full validation,
  documentation reconciliation), and the timeshare S2 evidence
  remains unchanged. No full build/full CTest or documentation reconciliation
  claim is made, and no documentation is claimed updated by this update —
  local to this partial update only, not a statement about the final Track
  state.
- This Track update (user-approved, partial S3) records the delivered
  header-tempo source subchunk of S3/A2: the self-authored
  `mdat.header_tempo`/`smpl.header_tempo` fixture with
  `header_tempo_layout.md` (no copied content or external file paths), where
  header tempo 100 is established in StartSong before the first bridge tick
  and no speed/timeshare control exists in the fixture. The 44.1 kHz
  component contract renders three active ticks `1102`/`1103`/`1103` with
  correct carried remainders (`316790`/`275625`/`234460`), distinct from the
  fixed-default `14318` behavior of `881` frames/tick. Test-first evidence: a
  temporary isolated fixed-context timing mutation (only the mixer timing
  input forced to 14318, never the workspace) produced the causal
  `881 != 1102` failure; the workspace is green with `test_playback_context`
  53/53 and a clean `git diff --check`. The speed partial-S3 evidence and the
  S2 timeshare evidence are preserved unchanged. No new box is checked: S3,
  A2, A4–A6, and M4–M6 remain unchecked; the remaining S3 work is the
  speed-source fake-CoreAudio and application-level composition, full
  validation (full CTest, full build), and bounded documentation
  reconciliation (PLAYER legacy local-code reference, Architecture, ASR
  evidence, README/Glossary/ADR review), with S4 (the user's supplemental
  manual Turrican2-TITLE check) also pending. No full build/full CTest or
  documentation reconciliation claim is made, and no documentation is claimed
  updated by this update — local to this partial update only, not a
  statement about the final Track state.
- This Track update (user-approved, partial S3) records the delivered
  speed-source fake-CoreAudio workspace/HAL composition subchunk of S3/A2 and
  its test-only lifecycle cleanup: the audio-output composition test reuses
  the self-authored `mdat.speed`/`smpl.speed` fixture at 44.1 kHz through the
  fake CoreAudio facade → adapter → coordinator → renderer path; the
  workspace zero request bypasses the renderer with no tick advance, and
  nonzero native-buffer requests consume the default 881 then two corrected
  speed 1103 ticks exactly, with the engine active (not complete) on the
  dynamic speed ticks and zero adapter allocations proven. The dynamic
  fake-CoreAudio test state is teardown-owned: on assertion failure after
  start, the teardown stop/quiesces/disposes the adapter before destroying
  the playback context and resets the observers, avoiding cross-test
  contamination — test reliability only, with no product/CoreAudio
  production behavior change. Evidence current to this point: playback 53/53,
  renderer 4/4, focused CTest 2/2, clean `git diff --check`; no full
  build/full CTest or documentation claim. The S2 timeshare evidence and the
  partial-S3 speed/header component records are preserved unchanged. No new
  box is checked: S3, A2, A4–A6, and M4–M6 remain unchecked; S3 remains the
  next unchecked implementation step with only application-level composition,
  full validation (full CTest, full build), and bounded documentation
  reconciliation remaining, and S4 remains the user's supplemental manual
  Turrican2-TITLE check. No documentation is claimed updated by this update —
  local to this partial update only, not a statement about the final Track
  state.
- This Track update (user-approved, partial S3) records the delivered
  test-only fake request-trace repair and the delivered application-level
  composition subchunk of S3/A2. The test-only repair: the fake-CoreAudio
  facade's bounded request trace retains the ordered first eight callback
  requests; at the ninth request the trace remains capped and truncation
  becomes observable (`request_trace_truncated` true) while later callbacks
  still execute and deliver (render and delivery counts track the total at
  every stage); facade re-initialization resets the total, retained, and
  truncation state; `render_request_count` is the total admitted/attempted
  callback requests, not a successful-delivery count. The application
  composition: the self-authored finite header/speed fixture
  (`mdat.header_speed_finite`/`smpl.header_speed_finite` with
  `header_speed_finite_layout.md`, no copied content or external paths) runs
  through the application → fake CoreAudio workspace/HAL route with
  application success, the rate-0 startup request, negotiated 44.1 kHz
  preparation, and the clean
  open/configure/prepare/bind/start/stop/quiesce/dispose lifecycle; the
  application level records no per-tick timing claims — exact timing-source
  behavior remains component-owned by the playback and renderer tests.
  Current focused validation: application 6/6, audio-output 37/37, playback
  53/53, legacy-renderer 4/4, focused CTest four targets 4/4, clean
  `git diff --check`; no full build/full CTest or documentation claim. The
  S2 timeshare evidence and the partial-S3 speed, header-tempo, and
  speed-source fake-CoreAudio workspace/HAL records are preserved
  unchanged. No new box is checked: S3, A2, A4–A6, and M4–M6 remain
  unchecked; S3 remains the next unchecked implementation step with only
  full validation (full CTest, full build) and bounded documentation
  reconciliation remaining, and S4 remains the user's supplemental manual
  Turrican2-TITLE check. No documentation is claimed updated by this update —
  local to this partial update only, not a statement about the final Track
  state.
- This Track update (user-approved, review GO) records the performed bounded
  documentation reconciliation for S3/A6/M6 only: the six already-approved
  docs — `README.md`, `docs/ARCHITECTURE.md`, `docs/ASR.md`,
  `docs/GLOSSARY.md`, `docs/AUDIO_RENDERING_DESIGN.md`, and
  `docs/TFMXLegacy/PLAYER.md` — were reviewed against the delivered Track 018
  state and remain consistent with the bounded result: the private same-tick
  post-interpreter `eClocks` handoff, the local speed low9 correction, and
  self-authored primary evidence, with the supplemental manual
  Turrican2-TITLE check pending as of this update and no broad
  compatibility/exact-audio/public API/target-architecture claim. ADR
  review found no ADR update or new ADR required. No documentation file was
  edited, and no source, test, fixture, configuration, project memory, or
  Git history was changed; this update edits the Track file only — local to
  this update, not a statement about the final Track state. Remaining
  requirements are preserved: full
  build/full CTest validation, application/Track acceptance reconciliation
  as applicable, the user's manual Turrican2-TITLE check in S4, and roadmap
  reconciliation at completion. No new box is checked: S3, A2, A4–A6, M4–M6,
  and S4 remain unchecked; S3 remains the next unchecked plan step, and all
  prior S2/S3 partial evidence is preserved unchanged.
- This Track update (user-approved, final S3 review GO) records the final
  S3 review GO with no findings and checks S3, A2, A4, A6, and M4. The
  authoritative validation evidence is recorded: the full build
  `cmake --build build --parallel 2` passed, the full CTest suite
  `ctest --test-dir build --output-on-failure` passed 8/8, and
  `git diff --check` is clean. S3 is delivered: the bounded private
  same-tick `eClocks` behavior with header, speed, and timeshare evidence
  is complete, and bounded documentation reconciliation is complete with no
  ADR change needed. As of this update, S4 is the next unchecked plan step:
  the user's supplemental manual Turrican2-TITLE check outcome was required
  before A5, M5, and Track completion (never automated acceptance and no
  broad or exact-audio claim); roadmap reconciliation remained completion
  work. No
  box beyond S3/A2/A4/A6/M4 is newly checked — A1, A3, S1, S2, M2, M3, and
  Q1–Q4 remain checked from prior updates, and A5, M1, M5, M6, and S4 remain
  unchecked. No commit or push occurred, and this update changes no source,
  test, fixture, documentation, configuration, project memory, or Git
  history — the Track file only (local to this update, not a statement about
  the final Track state).
- This Track update (user-approved, S4 manual outcome) records the user's
  supplemental manual Turrican2-TITLE check outcome and checks S4 and A5
  only. The explicit user outcome: the previously missing Turrican2-TITLE
  tempo transition is now audible. This is supplemental user judgment
  following the completed automated evidence (full build passed, full CTest
  8/8, clean `git diff --check`); it is not an automated criterion, an
  exact-audio comparison, or a broad compatibility claim, and no external
  files, paths, module content, or raw controls are retained. Documentation
  impact is assessed and recorded as of this update: the existing bounded
  Track 018 docs then stated the correction and the pending manual boundary,
  so no additional document mutation was performed by this manual outcome —
  a statement local to this update, not a description of the final Track
  state. Final
  Track completion, critique lifecycle action, roadmap reconciliation, and
  Git workflow remain separate pending steps. No box beyond S4 and A5 is
  newly checked — A1–A4, A6, S1–S3, M2–M4, and Q1–Q4 remain checked from
  prior updates, and M1, M5, M6 remain unchecked; the Track is not moved,
  renamed, or completed. This update changes no source, test, fixture,
  documentation, configuration, project memory, or Git history — the Track
  file only (local to this update, not a statement about the final Track
  state).
- This Track update (user-approved, completion transition) moves Track 018 to
  COMPLETED and checks S5, M1, M5, and M6; A1–A6 and S1–S4 remain checked
  with no pending Track execution steps. Completion evidence recorded: the
  user confirmed the previously missing Turrican2-TITLE tempo transition is
  now audible (supplemental judgment only, never automated acceptance or a
  broad/exact-audio claim); the full build passed; CTest passed 8/8; `git
  diff --check` is clean; roadmap reconciliation is recorded (Phase 4 roadmap
  revision 19, canonical index revision 26, Stage 4 still in progress, no
  next Track drafted); the critique is archived at lifecycle revision 2; four
  manual-outcome docs were reconciled while the prior six timing docs remain
  reconciled, and ADR review found no ADR update or new ADR required. No
  commit or push occurred; the unrelated AGENTS.md remains outside Track
  scope and unmodified by this transition. This update edits the Track file
  only.
