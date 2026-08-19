# AGENTS.md

## Project Intent

SynthTracker is the product and repository identity: a modern, maintainable C
product with a transitional legacy CLI and a future GUI-first DAW using SDL.
TFMX denotes the legacy format, modules, semantics, and temporary compatibility
lineage; it does not name the DAW product.

## Principles

- Read before changing, especially `docs/AGENT_WORKFLOW.md`.
- Prefer small, explicit MCP tools over broad access.
- Do not add shell-execution or unrestricted filesystem features.
- Do not store secrets, tokens, private keys, or sensitive personal data.
- Keep memory visible, consent-based, and easy to delete.
- Favor simple filesystem-backed schemas before infrastructure complexity.
- Keep all SynthTracker-owned production and test source, including the future GUI/DAW,
  in C23 or a later ISO C standard. C++ is not a project direction;
  implementation languages for third-party dependencies are evaluated
  separately.

## Guardrails

- ALWAYS read `docs/AGENT_WORKFLOW.md`

## Analysis Style

- Analysis is a joint, step-by-step exercise with the user; the agent does not deliver a
  finished analysis on its own.
- After each short step, present the concrete evidence and stop, so the user can course-correct
  before the agent draws conclusions.
- Do not write large standalone analysis dumps; keep each step small and reversible so the
  user can steer the investigation as it unfolds.

## Current Scope

- Refactoring the legacy C engine (src/, include/) into a reusable modern C
  playback core under the modern C product boundary. C23 remains the current
  build and validation baseline.
- During Phase 4, preserve current TFMX behavior where practical only as a
  temporary development scaffold. Every Phase 4 Track must assess compatibility
  impact and retain appropriate evidence; this is not a SynthTracker v1
  compatibility promise.
- Preserve the current SDL-era audio features exposed through the SDL 1.2-era
  API surface (stereo blending, low-pass filter); SDL 1.1.7 remains historical
  context.
- Designing the future modern C GUI DAW layer on top of the engine; no GUI or
  editing functionality is implemented yet.
- Phase 3 is delivered. Phase 4 is next: component extraction under the
  SynthTracker product boundary.
- Consult `.backlog/` for the current Track status; implementation requires an ACTIVE Track.

## Project Memory

- Read `MEMORY.md` before using Mnemosyne or assuming that prior project context
  is absent.
- Use the available Mnemosyne tools as the primary durable project record store.
- Follow every instruction in `MEMORY.md`; its project-local rules are
  non-negotiable.

## Subagents

Always try to use subagents rather than doing the work directly.

- Use `@explore` for read-only repository discovery, analysis, and review; it
  must not edit files or run state-changing commands.
- Use `@general` for distinct, bounded, multi-step work with explicit scope and
  verification requirements.
- Use `@test` for independent test review and automated verification. It may
  modify tests only within an explicitly approved TDD chunk and must not change
  production code.
- Use `@build` for implementing approved TDD chunks (failing focused test,
  smallest passing change, refactor, validate); it never changes Tracks, durable
  memory, configuration, or Git history.
- Use `@plan` for read-only architecture analysis and implementation-plan
  design; it never edits files or runs state-changing commands.
- Use `@investigate` for read-only investigation and debugging of complex issues only;
  it never edits files or runs state-changing commands.
- Use `@repo` only for read-only Git status, diff, history, and repository
  structure inspection; it must not modify files or implement behavior, tests,
  or documentation.
- Use `@docs` for explicitly approved documentation authoring and review. It may
  modify documentation only and must not change production code, tests,
  configuration, Tracks, or durable memory.
- Do not duplicate work already delegated to another subagent.
- Before delegating state-changing work, obtain the user's approval and
  explicitly grant permission for the approved scope.
- Subagents must follow applicable repository guidance, ACTIVE Track gates,
  declared TDD steps, and automated verification requirements.
- The primary agent remains responsible for reviewing results, integrating
  decisions, validating changes, and reporting evidence.

## Before Editing

- Inspect `README.md`, `docs/VISION.md`, `docs/ARCHITECTURE.md`, and the affected package files.
- For terminology or public-contract work, read `docs/GLOSSARY.md` first.
- For backlog work, follow `.backlog/README.md`; implementation requires an ACTIVE Track and its implementation gates.
- Implementation follows TDD by default: write a failing focused test, make it pass with the smallest implementation, then refactor and validate.
- Every behavior change requires automated test coverage; direct TFMX checks complement automated tests and do not replace them.

## TFMX Testing

TBD

## Project Documentation

- The `docs/` folder is the home for durable project documentation beyond the README.
- Use `README.md` for user-facing setup, status, and quick orientation.
- Use `VISION.md` for product intent, boundaries, non-goals, and future direction.
- Use `docs/ARCHITECTURE.md` for the concise current-system overview and architecture entrypoint.
- Use `docs/ADR.md` as the index and governance for individual architectural decision records under `docs/adr/`.
- Use `docs/ASR.md` as the register of architecturally significant requirements.
- Keep product direction, scope, priorities, roadmaps, and their rationale in project memory, not ADRs or ASRs.
- Use `docs/AGENT_WORKFLOW.md` for contribution gates and verification expectations.
- Use `docs/GLOSSARY.md` for canonical product and protocol terminology.
- Use `docs/TFMXLegacy/` as the self-contained reference for the original
  TFMX format and player mechanics; start at `docs/TFMXLegacy/README.md`.
  Before writing or quoting legacy-format content, read and follow the
  citation and copyright policy in `docs/TFMXLegacy/PROVENANCE.md`.
- Add new focused docs under `docs/` when a topic becomes too detailed for the README.
- Keep `README.md`, `docs/ARCHITECTURE.md`, `docs/ASR.md`, relevant ADRs, and `docs/GLOSSARY.md` updated when changing public MCP behavior, endpoints, package layout, architectural boundaries, or MCP structure.
- Put implementation-specific rules in the nearest scoped `AGENTS.md`; keep this root file focused on project-wide constraints.
- Use `.backlog/README.md` for local Track governance and `.backlog/PORE.md` for problem-oriented requirements.
