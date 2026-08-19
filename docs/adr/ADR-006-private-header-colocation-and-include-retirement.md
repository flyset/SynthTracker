# ADR-006: Private Header Co-location and Include Retirement

Status: Accepted

Decision date: 2026-08-19.

## Context

The transitional source tree uses a top-level `include/` directory for
project-owned headers, while extracted private headers already live beside
their owning source files. The project needs one ownership-oriented layout for
production and test headers as extraction continues.

## Decision

All project-owned production and test headers must live in the same owning
source or test folder as the owning C source. This is folder co-location only;
it does not require a one-to-one basename pair between a C source file and a
header.

`include/` is deprecated for project-owned headers. No new project-owned header
may be added there, and the directory is to be removed after migration of its
project-owned contents. Third-party, generated, and platform SDK headers are
outside this decision.

## Consequences

- Header ownership is visible from the source or test folder that owns it.
- New extraction work does not expand the transitional `include/` layout.
- Existing project-owned headers require a migration before `include/` can be
  removed.

## Explicit non-decisions

This ADR does not create a public C API or establish a library-header model.
It does not change C23 requirements, runtime behavior, package boundaries,
public ABI, third-party dependencies, generated artifacts, or platform SDK
usage.

## Related ASRs

- ASR-008 — Co-located project-owned headers and `include/` retirement.

## Evidence

- [`ARCHITECTURE.md`](../ARCHITECTURE.md) — records the current transitional
  header layout and approved private-header end state.
- [`ASR.md`](../ASR.md) — records the migration and end-state requirement.
