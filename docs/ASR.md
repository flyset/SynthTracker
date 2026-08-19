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

**Current Phase 4 policy:** Phase 3 is delivered; Phase 4 is next. Preserve
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
- **Verification:** Source/build/test layout review and component tests.
- **Related ADRs:** [ADR-003](adr/ADR-003-private-seam-placement.md), [ADR-004](adr/ADR-004-component-first-test-organization.md), [ADR-002](adr/ADR-002-private-sdl-free-playback-evidence-seam.md) (private test-boundary evidence only; does not fulfill the target requirement), [ADR-005](adr/ADR-005-target-daw-component-foundation.md).

### ASR-006 — Isolated future audio output

- **Requirement:** Device-specific audio output must be provided through
  platform-specific adapters behind a device-independent Audio Output Port.
- **Status:** Target; the output port and platform-specific adapters are not
  implemented.
- **Verification:** Boundary/API review and adapter tests when implemented.
- **Related ADRs:** [ADR-005](adr/ADR-005-target-daw-component-foundation.md).

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
