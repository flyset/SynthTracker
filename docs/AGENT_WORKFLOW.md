# Agent Workflow

## Hard Gates

- Read-only actions may proceed without approval: inspect, list, search, read, and review Git status, diff, or log.
- Before any state-changing action, read the relevant scoped `AGENTS.md`, propose the exact commands and files that would change, and wait for an explicit **yes**.
- Treat an unclear action as state-changing.
- State-changing actions include edits, generated files, dependency installs, networked commands, environment changes, and Git history changes.

## Protocol and Contract Work

- Read `docs/GLOSSARY.md` before changing or discussing terminology or public contracts.
- Read `docs/ARCHITECTURE.md` before changing public C APIs, package boundaries,
  TFMX module or file-format semantics, playback or audio behavior, future
  persistent DAW project formats, or audio-output boundaries.
- Review the relevant entries in `docs/ASR.md` and applicable ADRs indexed by
  `docs/ADR.md` before making architectural or contract changes. Record new
  architectural decisions as ADRs only after approval; keep product-management
  decisions in project memory.
- Explicitly assess and document the intended compatibility impact on existing
  TFMX modules and the documented trackstep, pattern, macro, timing, and audio
  semantics; compatibility need not be perpetual.
- Keep TFMX component boundaries small, explicit, and independently testable.
  Loader/Writer components may define explicit filesystem behavior, but do not
  introduce general shell-execution or unrestricted filesystem-access
  interfaces outside explicit component boundaries.
- Before implementing an interface change, record an explicit impact
  decision in the active Track Decision log. Assess each relevant dimension:
  C API/ABI, module compatibility/extension, interpreter/timing/audio
  behavior, persistent DAW format/versioning, platform/audio-output adapter,
  and component/package boundaries. Include reasons for every dimension judged
  unchanged.

## Verification and Documentation

- Implementation follows TDD: write a failing focused test, implement the smallest change that passes it, then refactor and run the relevant validation.
- Every behavior change requires automated test coverage. Direct checks complement automated tests; they do not replace them.
- Do not create ad-hoc test scripts unless explicitly requested.
- When public C APIs or package boundaries, TFMX module or file-format
  semantics or compatibility, playback or audio behavior, persistent DAW
  project formats or versioning, or audio-output boundaries change, update the
  applicable `README.md`, `docs/ARCHITECTURE.md`, `docs/ASR.md`, relevant ADRs,
  and `docs/GLOSSARY.md` in the same change.

## Backlog Work

- Use `.backlog/README.md` as the canonical local Track workflow and `.backlog/PORE.md` for problem statements.
- New Tracks begin in DRAFT; implementation begins only after the Track is ACTIVE and its Move-to-ACTIVE plan step is checked.
- Execute ACTIVE work one declared plan step or coherent TDD chunk at a time, then record inventory and validation evidence in the Track.
- When a completed Track has been committed and pushed, record it in the changelog.
