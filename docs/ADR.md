# Architectural Decision Records

This document is the index and governance entrypoint for Architectural Decision
Records (ADRs). An ADR records a durable architectural choice, its context,
rationale, consequences, and status.

Product direction, scope, priorities, roadmaps, and their rationale are
product-management decisions and belong in project memory, not in ADRs. An ADR
must not be used to turn a product preference or roadmap item into an
architectural contract. Architecturally Significant Requirements are recorded
in [`ASR.md`](ASR.md); an ASR states what the architecture must satisfy, while
an ADR records why an architectural choice was made.

## Record convention

Future records use:

```text
docs/adr/ADR-NNN-<slug>.md
```

`NNN` is a zero-padded, immutable sequence number. A record has one of these
statuses:

- **Proposed** — under review; not an accepted architectural commitment.
- **Accepted** — approved and currently authoritative.
- **Superseded** — replaced by a later ADR; retained for history.

Accepted ADRs should identify affected ASRs and the relevant overview or
implementation evidence. Superseding an ADR requires a link from the older
record to its replacement. Product-management decisions remain in the
`tfmx` project-memory decisions collection.

## Records

- [ADR-001 — New Engine Not Line-by-Line Legacy Port](adr/ADR-001-new-engine-not-line-by-line-port.md) — **Accepted**, 2026-08-15.
- [ADR-002 — Private SDL-Free Playback Evidence Seam](adr/ADR-002-private-sdl-free-playback-evidence-seam.md) — **Accepted**, 2026-08-16.
- [ADR-003 — Private Seam Placement](adr/ADR-003-private-seam-placement.md) — **Accepted**, 2026-08-17.
- [ADR-004 — Tests Mirror Source Architecture](adr/ADR-004-component-first-test-organization.md) — **Accepted**, 2026-08-16.
- [ADR-005 — Target DAW Component Foundation](adr/ADR-005-target-daw-component-foundation.md) — **Accepted**, 2026-08-19.
- [ADR-006 — Private Header Co-location and Include Retirement](adr/ADR-006-private-header-colocation-and-include-retirement.md) — **Accepted**, 2026-08-19.
- [ADR-007 — Audio Frame Block Boundary and Fixed First Format](adr/ADR-007-audio-frame-block-boundary-and-fixed-first-format.md) — **Superseded** by [ADR-008](adr/ADR-008-audio-frame-block-mixed-value-boundary.md), 2026-08-20.
- [ADR-008 — Audio Frame Block Mixed-Value Boundary](adr/ADR-008-audio-frame-block-mixed-value-boundary.md) — **Accepted**, 2026-08-22.

## Related documentation

- [`ARCHITECTURE.md`](ARCHITECTURE.md) — concise current-system overview and
  architecture entrypoint.
- [`ASR.md`](ASR.md) — architecturally significant requirement register.
- [`VISION.md`](VISION.md) — product intent and future direction.
- [`../MEMORY.md`](../MEMORY.md) — product-management decisions and roadmap
  context.
