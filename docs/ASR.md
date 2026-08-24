# Architecturally Significant Requirements

This register records requirements that materially constrain the architecture.
Each entry has an immutable ID, a normative statement, status, verification
evidence, and related ADRs. Requirements contain no product rationale; product
direction, scope, priorities, and roadmaps are held in project memory.

## Fields

- **ID** — immutable `ASR-NNN` identifier.
- **Requirement** — the normative architectural statement.
- **Status** — `Current` for the implemented/current boundary, `Target` for an
  approved future requirement that is not implemented, or `Retired (historical)`
  for a former requirement retained as historical evidence.
- **Verification** — current evidence or the validation expected when the
  target requirement is implemented.
- **Related ADRs** — accepted or proposed decisions that satisfy or constrain
  the requirement.

## Register

### ASR-001 — Modern ISO C source boundary

- **Requirement:** All SynthTracker-owned production and test source must use C23 or a
  later ISO C standard.
- **Status:** Current
- **Verification:** CMake language standard and repository source/test review.
- **Related ADRs:** None yet.

### ASR-002 — Historical compatibility evidence record

- **Requirement:** Retired historical requirement: existing TFMX modules were
  required to load and play with correct musical behavior; bit-identical
  rendered audio was not required.
- **Status:** Retired (historical)
- **Verification:** Historical bounded self-authored fixture coverage,
  automated component/integration tests, and direct legacy checks remain
  recorded evidence. This evidence does not constitute format-wide proof or a
  SynthTracker v1 compatibility promise.
- **Related ADRs:** [ADR-001](adr/ADR-001-new-engine-not-line-by-line-port.md), [ADR-002](adr/ADR-002-private-sdl-free-playback-evidence-seam.md).

**Current Phase 4 policy:** Phase 3 is delivered; Phase 4 is in progress. Preserve
current TFMX behavior where practical only as a temporary development scaffold.
Every Phase 4 Track must assess compatibility impact and retain appropriate
evidence. This policy is not a SynthTracker v1 compatibility requirement.

### ASR-003 — UI-agnostic playback core

- **Requirement:** Playback responsibilities remain usable independently of GUI
  control and observability concerns.
- **Status:** Target; the current private seam is not a completed public core.
- **Verification:** Component-boundary review and playback tests when the
  reusable core is implemented.
- **Related ADRs:** [ADR-005](adr/ADR-005-target-daw-component-foundation.md).

### ASR-004 — Explicit, independently testable component boundaries

- **Requirement:** TFMX components have small, explicit boundaries that can be
  tested independently.
- **Status:** Target; the current legacy implementation remains largely
  co-located and global.
- **Verification:** Source/build/layout review is supporting evidence only.
  Observable component behavior is covered by component tests, application
  workflows and composition by application-level tests, and executable
  composition by build/link/integration checks, as each boundary is
  implemented. See [`TESTING.md`](TESTING.md); compatibility fixtures and
  direct checks remain supplemental.
- **Related ADRs:** [ADR-003](adr/ADR-003-private-seam-placement.md), [ADR-004](adr/ADR-004-component-first-test-organization.md), [ADR-002](adr/ADR-002-private-sdl-free-playback-evidence-seam.md) (private test-boundary evidence only; does not fulfill the target requirement), [ADR-005](adr/ADR-005-target-daw-component-foundation.md).

### ASR-006 — Isolated future audio output

- **Requirement:** Device-specific audio output must be provided through
  platform-specific adapters behind a device-independent Audio Output Port.
- **Status:** Target; the device-independent Audio Output Port and adapters
  behind it are not implemented. A private device-driven CoreAudio adapter
  exists (Track 015) but is not the port's implementation.
- **Verification:** Boundary/API review and adapter tests when implemented.
- **Related ADRs:** [ADR-005](adr/ADR-005-target-daw-component-foundation.md), [ADR-009](adr/ADR-009-callback-driven-audio-rendering.md).

### ASR-007 — Explicit filesystem and shell boundaries

- **Requirement:** TFMX components must not expose general shell-execution or
  unrestricted filesystem-access interfaces. In the target allocation,
  `Filesystem` is limited to bounded directory browse/list operations and file
  deletion only; it produces `File Information` and does not read or write
  content. `File I/O` directly reads
  and writes content only through `File Information` provided by `Model` for
  its bounded format duties. `Model` may retain `File Information` but has no
  filesystem authority.
- **Status:** Target; the no-shell and no-unrestricted-filesystem guardrail is
  current, while this target allocation is not implemented.
- **Verification:** Current review confirms no general process-launching or
  unrestricted filesystem interface. Target verification is a boundary/API
  review confirming the stated least-authority split when implemented.
- **Related ADRs:** [ADR-005](adr/ADR-005-target-daw-component-foundation.md).

### ASR-008 — Co-located project-owned headers and `include/` retirement

- **Requirement:** All project-owned production and test headers must live in
  the same owning source or test folder as the owning C source. This requires
  folder co-location only and does not require a one-to-one basename pair.
  `include/` must contain no project-owned headers at the completed end state;
  no new project-owned header may be added there during migration. Third-party,
  generated, and platform SDK headers are outside this requirement.
- **Status:** Current; completed.
- **Verification:** Completed layout review confirms that the three remaining
  legacy headers (`player.h`, `tfmx.h`, and `tfmxsong.h`) are under `src/`
  (the legacy `audio.h` was retired with the SDL-era audio path) and that
  project-owned production and test headers are co-located with their
  owning source or test folders. Removal verification confirms that `include/`
  is retired and contains no project-owned headers. Build/include-resolution
  validation provides supporting evidence.
- **Related ADRs:** [ADR-006](adr/ADR-006-private-header-colocation-and-include-retirement.md).

### ASR-009 — Audio Frame Block Boundary Invariants

- **Requirement:** An Audio Frame Block is an ordered sequence of zero or more
  frames. Every frame contains a signed-32 left mix value and a signed-32 right
  mix value emitted by the `Mixer`; the precise internal creation point inside
  the `Mixer` is not fixed by this requirement. The block contains no
  device-native or serialized PCM representation. Zero-frame blocks are valid.
- **Status:** Target
- **Verification:** Focused component tests when the Audio Output boundary is
  implemented. The concrete C API/layout, validation-result and error
  representation, ownership/lifetime, numerical range, and clipping or overflow
  behavior are not fixed by this requirement and remain deferred to later work.
- **Related ADRs:** [ADR-008](adr/ADR-008-audio-frame-block-mixed-value-boundary.md), [ADR-009](adr/ADR-009-callback-driven-audio-rendering.md), [ADR-005](adr/ADR-005-target-daw-component-foundation.md).

### ASR-010 — Callback-driven real-time audio rendering

- **Requirement:** Live audio output must be driven by the platform device
  (CoreAudio on macOS) through a private render boundary: for every
  device-requested Audio Frame Block the boundary must answer on demand from
  current and future playback state and time-ordered events — never from
  pre-rendered audio. Each nonzero request is answered with exactly the
  requested frame count; on the implemented workspace/HAL route a zero-frame
  request is accepted without invoking the renderer or the exact-N
  coordinator. The approved broader direction covers zero-frame requests as
  answered on-demand blocks; see ADR-009. The callback path must use
  lifecycle-preallocated storage
  and perform no allocation, locking, file I/O, UI work, or other unbounded
  work. Adapter-private conversion remains separate, and no public Audio Output
  Port, C API, header, or export is introduced. During pre-start preparation,
  the private route obtains the active output device's negotiated sample rate
  and configures the callback-driven TFMX renderer at that rate for the active
  run; it does not force a device to 44.1 kHz and Track 015 introduces no
  resampler. Audio Frame Blocks remain rate-free signed-32 stereo mixed values:
  the rate is private lifecycle/configuration context for the run, not Frame
  Block metadata. The route presents an output configuration of exactly two
  channels to the adapter, whose private client format is interleaved Float32;
  each Audio Frame Block's left/right values map directly to those two channels
  through the existing adapter-private conversion, and a selected
  device/configuration that cannot provide this strict stereo setup is
  rejected. No mono fold-down, surround/upmix mapping, channel remapping,
  device-native representation, or Audio Frame Block metadata is introduced.
  The application/control thread prepares the module and the
  callback context before audio starts; while the route is active, the render
  callback has exclusive ownership of the legacy playback, timing, voice, and
  mix state. Control requests a stop at a
  render-block boundary, and the callback becomes quiescent before the control
  thread stops or disposes audio resources or tears down module state. The
  callback path contains no locks: exclusivity is enforced by the lifecycle
  protocol, not by callback-path synchronization.
- **Status:** Current for the private device-driven route; the public Audio
  Output Port remains target-only. Track 015 S5 implemented and
  component-tested the device-driven route: the real legacy exact-N renderer,
  the private CoreAudio adapter lifecycle/callback with control-side
  quiescence and lock-free admission gate,   workspace-only conversion, the HAL
  workspace-copy route, and the real HAL Output Audio Unit facade with a strict
  44.1/48 kHz negotiated-rate gate (a zero-frame workspace/HAL request is
  accepted at the admission gate without renderer or coordinator invocation);
  the application cut over to this route and
  the SDL live-audio path, the `-o` file-output path, and the `-b`, `-8`, `-f`,
  `-o`, `-w`, and `-v` options are removed. Track 015 S6.2–S6.4b delivered the
  private rate-0 startup-selection sentinel (the application requests rate 0;
  after device open the facade accepts only an actual 44.1/48 kHz nominal rate
  and reports the verified configured rate for control-side renderer
  preparation before bind/start) and the private legacy producer's
  blend-then-widen normalization (blended int32 lanes reconstructed to their
  low-16 historical PCM-domain value and multiplied by 65536 into the existing
  signed-32 Audio Frame Blocks),
  while the generic adapter conversion remains signed-32/2^31; `voices_01` is
  the self-authored supplemental audible-smoke fixture, with a
  listener-confirmed real 48 kHz playback check. The earlier device-free
  S4/S5.1–S5.5 coordination, bound-instance, deferred-stop-to-quiescence,
  borrowed workspace, and workspace-mode startup-rejection evidence remains as
  the deterministic test foundation for this route. Track 015 S6.5 (final
  validation/documentation) is complete: the full CTest suite passes 8/8, the
  required documentation was reconciled, and the Phase 4 roadmap was revised
  (Phase 4 revision 14; canonical index revision 21). S7 (completion) is
  checked: Track 015 is completed at
  `.backlog/COMPLETED/2026/TRACK_015_COMPLETED_coreaudio_live_route_and_sdl_retirement.md`
  while Stage 3 remains in progress pending its wider acceptance/merge. No public
  API/port, target `Mixer`, non-macOS adapter, live input, rendered-file
  export, device-rate-change restart policy, workspace release/close, invalid
  storage-length proof, resampler, or loader-expansion work was added; general
  real-module loader compatibility remains deferred after the recorded XOut2
  rejection with no format-wide promise.
- **Verification:** Deterministic automated component tests drive the private
  render boundary with synthetic frame-count requests (variable and zero) and
  lifecycle contracts — including start/stop ownership and callback quiescence
  before teardown — without a physical device; component tests also confirm
  Audio Frame Blocks remain rate-free (carry no rate metadata), that the
  renderer is configured at the active rate for the run, and the strict stereo
  channel/format policy (exactly two channels presented to the adapter, direct
  left/right mapping through the adapter-private interleaved Float32
  conversion, and rejection of a selected device/configuration that cannot
  provide strict stereo, with no channel elaboration or block metadata).
  Fixture-driven legacy exact-N renderer tests cover the four approved fixtures
  at 44.1 and 48 kHz (8 cases) with exact-N fulfillment, zero-frame no-advance,
  partial-tick retention, exactly-once tick advance, non-silent stereo Float32
  output, and zero adapter allocation. Preparation-ordering tests prove
  configure → prepare → bind/start with no request before preparation and
  rollback to INACTIVE on configure, unsupported-format, or preparation
  failure; the rate-0 wildcard tests prove a reported 44.1/48 kHz configured
  format prepares/binds/starts, explicit 44.1/48 requests retain equality
  checking, and other nonzero rates are preflight-invalid. S6.4b proves the
  private legacy blend-then-widen normalization (blended int32 lanes
  reconstructed to their low-16 historical PCM-domain value and multiplied by
  65536) with the exact
  `voices_01` Float32 oracle (L=1680/32768, R=1392/32768) at both rates.
  Application-level tests exercise the cutover and removed-option rejection,
  and prove the workspace-only fake default 48 kHz route requests rate 0 and
  follows exactly open→configure→prepare→bind→start→stop→quiesce→dispose; the
  full CTest suite passes 8/8. Real-time-path audit confirms no allocation,
  locking, file I/O, UI work, or other unbounded work on the callback path;
  bounded on-device checks (including a listener-confirmed real 48 kHz
  `voices_01` smoke) provide supplemental evidence only. CoreAudio API/unit
  choice is decided (HAL Output Audio Unit); the concrete
  reconfiguration/restart policy for a device sample-rate change, lifecycle
  state-machine details beyond the implemented control-side quiescence, event
  handoff/clock/overflow semantics, threads/locks beyond the lock-free
  admission gate, invalid storage-length proof, workspace release/close, and
  general real-module loader expansion remain deferred. ADR-010 (Accepted,
  2026-08-23) assigns native adapter ownership of device callbacks, lifecycle,
  and conversion, and the private `audio_output` synchronous device-demand
  coordinator role; Track 015 S5 implemented and component-tested that
  coordinator's exact-N policy — it requests exactly N frames from the
  renderer, rejects a returned block whose frame count differs from N or a
  nonzero block with NULL frame storage without delivery, padding, or
  truncation, and otherwise delivers the same borrowed block once — plus the
  device-free bound-instance, deferred-stop-to-quiescence, borrowed-workspace,
  and workspace-mode startup-rejection evidence, the real legacy exact-N
  renderer, the lock-free admission gate, the HAL workspace-copy route, the
  real HAL Output Audio Unit lifecycle/callback with control-side quiescence,
  and the application cutover with SDL/`-o`/CLI deletion, while the public
  Audio Output Port, real device/framework stop/close details beyond the
  implemented control-side quiescence, device-rate-change restart, and the
  remaining deferred mechanics stay deferred.
- **Related ADRs:** [ADR-010](adr/ADR-010-native-adapter-ownership-and-private-demand-coordination.md), [ADR-009](adr/ADR-009-callback-driven-audio-rendering.md), [ADR-008](adr/ADR-008-audio-frame-block-mixed-value-boundary.md), [ADR-005](adr/ADR-005-target-daw-component-foundation.md).
