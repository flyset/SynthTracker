# ADR-001: New Engine Not Line-by-Line Legacy Port

Status: Accepted

This ADR was retrospectively recorded on 2026-08-18 from the original decision
evidence. The original decision date is 2026-08-15.

## Context

The legacy implementation provides behavior and semantic guidance, but its
source shape and ownership are not the required shape for the new engine.

## Decision

Build a new engine informed by legacy ideas and semantics rather than porting
the legacy source line by line.

## Consequences

- Legacy behavior remains a compatibility reference rather than a required
  source layout.
- New implementation boundaries and representations may differ from the
  legacy source.
- Compatibility behavior must be supported by focused evidence during
  refactoring.

## Explicit non-decisions

This ADR does not decide the C23/C++ policy, typed host-endian or
raw-versus-decoded representation, GUI direction, macOS scope, CoreAudio,
Audio Output Port, loader/domain ownership, or reentrancy.

## Related ASRs

- ASR-002 — Preservation of documented compatibility semantics.

## Evidence

- No originating Track exists for this decision.
- `da20c5f9eda211cec750ae095976e92fe5e26164` (`da20c5f`) — the first
  committed `docs/ARCHITECTURE.md` record establishing the new-engine-not-
  line-by-line-port decision.
