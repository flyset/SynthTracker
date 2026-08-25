# TDD and Governance Analysis

This document identifies where SynthTracker's TDD method, testing strategy, and
implementation governance are documented. Track files under
`.backlog/**/TRACK_*.md` are intentionally excluded; those Tracks are created
from the rules identified here.

## Summary

SynthTracker explicitly documents a focused red-green-refactor TDD process. It
does not explicitly name Chicago/classic, London/mockist, outside-in,
inside-out, BDD, or ATDD.

The project has two related but distinct policy layers:

1. **Testing strategy** — what evidence tests should provide and where tests
   belong.
2. **Governance** — when implementation may begin, what approvals are needed,
   how work is divided, and what evidence must be recorded.

The testing burden is produced by both layers. Governance makes the testing
strategy mandatory for each implementation chunk and adds approval,
traceability, validation, and record-keeping requirements.

## The problem: a modest TDD method multiplied by heavy governance

The TDD method identified in this repository is not inherently exhaustive. It
is the ordinary focused **red-green-refactor** loop:

1. write a failing test for intended observable behavior;
2. make the smallest implementation that passes it; and
3. refactor and validate.

The closest named style is **classic/Chicago TDD**, because the tests generally
exercise real components and assert observable state, results, output, and
rejection behavior. That classification is inferred from the tests; the project
does not explicitly choose the Chicago/classic label. The project does
explicitly choose component-owned tests, behavior-focused evidence, and
automated coverage for behavior changes.

The feeling of exhaustiveness comes from multiplying that small loop by several
independent governance requirements. A single change may be required to have:

- a declared implementation chunk;
- an approved ACTIVE Track;
- a focused test that demonstrably fails for the intended reason;
- the smallest passing implementation;
- a refactoring step;
- focused validation and broader-suite validation;
- component, application, executable, or compatibility evidence depending on
  the affected boundary;
- compatibility-impact and architectural-impact decisions;
- an inventory and validation record in the Track; and
- completion/status updates before the chunk is considered done.

These requirements are not all the same kind of testing. Some prove behavior;
others prove process compliance, architectural intent, compatibility awareness,
or traceability. The result is a multiplicative burden:

```text
implementation change
  × TDD red/green/refactor loop
  × evidence level(s)
  × approval and scope gates
  × validation commands
  × durable record updates
```

This distinction matters for refinement. Reducing the burden does not require
abandoning TDD. It can mean narrowing the evidence and governance obligations
according to risk—for example, retaining the full loop for new behavior and
public/audio/compatibility boundaries while using lighter validation for
low-risk refactoring or already-characterized legacy behavior.

## TDD and testing-strategy documentation

### Canonical sources

#### `docs/TESTING.md`

This is the canonical detailed testing strategy.

- `1-16` — evidence levels:
  component, application, build/link/executable integration, and bounded
  compatibility evidence.
- `18-24` — TDD loop:
  failing automated test, smallest passing change, refactor, and validation;
  every behavior change requires automated coverage.
- `26-34` — application and executable coverage boundaries.
- `36-54` — component ownership, test placement, and structural evidence limits.
- `56-63` — fixture and compatibility-evidence rules.
- `65-78` — current CMake/CMocka validation commands.

This is the primary document to revise when changing the testing strategy.

#### `docs/AGENT_WORKFLOW.md`

- `36-45` — restates the TDD loop, coverage requirement, evidence levels, and
  the rule that structural inspection cannot substitute for behavioral evidence.
- `47-51` — test placement and links to the canonical testing rules.

This is both a testing-policy reference and the main implementation-workflow
document.

#### `AGENTS.md`

- `98-104` — before-editing rules and the default TDD sequence.
- `106-126` — project-wide testing rules, evidence boundaries, and mandatory
  automated coverage.

This is the project-wide instruction layer and repeats key rules from
`docs/TESTING.md` and `docs/AGENT_WORKFLOW.md`.

### Scoped and supporting sources

#### `tests/AGENTS.md`

- `1-16` — test contribution rules: component ownership, application-level
  tests, executable checks, compatibility fixtures, and mandatory coverage.

#### `tests/README.md`

- `1-5` — points to the canonical testing strategy.
- `7-31` — describes current test areas, evidence boundaries, and integration
  coverage.

#### `docs/adr/ADR-004-component-first-test-organization.md`

- `14-25` — accepted decision that tests co-evolve with source component
  boundaries and that shared fixtures remain separate.

This ADR documents **component-first test organization**, not a named TDD
variant or a required inside-out/outside-in development direction.

#### `README.md`

- `63-76` — documents CMocka, CMake, and CTest as the testing infrastructure.
- `107-115` — points readers to the workflow and architecture documentation.

## Governance documentation

### Core implementation governance

#### `docs/AGENT_WORKFLOW.md`

- `3-10` — read-only versus state-changing actions and approval gates.
- `12-34` — terminology, architecture, compatibility, and public-contract
  review requirements.
- `36-57` — TDD, validation, documentation, and evidence requirements.
- `59-66` — Track activation, execution one chunk at a time, and recording
  evidence in the Track.

This is the main governance control point outside the Track files.

#### `AGENTS.md`

- `28-30` — mandatory reading of the workflow rules.
- `64-96` — subagent roles, delegation boundaries, approval requirements, and
  implementation responsibilities.
- `98-104` — pre-edit gates.
- `128-146` — documentation ownership and update requirements.

#### `.backlog/README.md`

- `38-40` — Track non-negotiable: focused failing test, smallest passing
  implementation, refactor, and validation.
- `87-106` — Track lifecycle and execution workflow.
- `108-117` — implementation gates, mandatory TDD, validation, and completion.

This is the canonical backlog/Track governance source, even though individual
Track files are excluded from this analysis.

#### `.backlog/AGENTS.md`

- `1-9` — scoped backlog rules, including Track structure, ACTIVE status,
  one-TDD-chunk execution, automated tests, and evidence requirements.

#### `.backlog/PORE.md`

- `1-8` — problem-oriented requirements and the requirement that every
  objective and acceptance criterion trace to a stated problem.
- `15-22` — root causes, success conditions, scope boundaries, and acceptance
  evidence.

PORE is requirements and traceability governance rather than TDD policy, but it
contributes to the perceived process burden.

### OpenCode operational enforcement

These files translate repository governance into agent behavior.

#### `.opencode/agents/synthtracker.md`

- `20-24` — collaborative, approval-gated, stepwise execution.
- `26-34` — mandatory startup gates.
- `35-45` — project rules, memory, subagent use, and concise execution.

#### `.opencode/agents/build.md`

- `14-25` — implementation only for approved TDD chunks under an ACTIVE Track.
- `28-31` — red, green, refactor, focused validation, and broader-suite
  validation.

#### `.opencode/agents/test.md`

- `13-29` — independent test-review role, approval boundaries, and restrictions.
- `31-37` — red-phase evidence and focused-then-broader validation.

#### `.opencode/skills/synthtracker-start-work/SKILL.md`

- `8-31` — required readiness review and governing-document review.
- `33-50` — subagent roles and implementation boundaries.
- `51-64` — readiness report, approval requirements, and stop conditions.

#### `.opencode/skills/synthtracker-finish-work/SKILL.md`

- `6-13` — post-completion status, diff, roadmap, and record review.

#### `opencode.json`

- `3-13` — global permission and approval configuration.
- `31-44` — SynthTracker agent permission configuration.

This file enforces permissions mechanically; it does not define the TDD
method itself.

## Secondary governance and evidence sources

These documents impose or describe domain-specific evidence expectations but do
not define the general TDD method:

- `docs/ADR.md:1-32` — ADR authority, status, approval, and evidence rules.
- `docs/ASR.md:1-18,56-68` — architectural requirements and verification
  evidence; ASR-004 defines independently testable component boundaries.
- `docs/ARCHITECTURE.md` — current architecture and validation boundaries.
- `docs/ARTIFACTS.md` — artifact authority and evidence direction.
- `docs/AUDIO_RENDERING_DESIGN.md:371-398` — audio-specific evidence strategy.
- `docs/MACRO_DESIGN.md:22-27` — compatibility-evidence expectations.
- `docs/TFMXLegacy/PROVENANCE.md` — legacy evidence, citation, and discrepancy
  rules.
- `MEMORY.md:64-85` — roadmap structure and the role of TDD chunks in the
  execution model. It is not the canonical source for the TDD method.

Individual accepted ADRs under `docs/adr/` may add evidence requirements for a
particular architectural decision. They should be reviewed when changing the
relevant boundary, but they are not general TDD policy sources.

## Recommended refinement order

The refinement should begin with **deduplication and authority**, not with
changing individual requirements. Otherwise a relaxed rule in one document may
be reintroduced by an instruction, scoped rule, skill, or Track template.

Use one source of truth for each closely related policy domain:

- `docs/TESTING.md` — authoritative source for the TDD method, test strategy,
  evidence levels, and validation expectations.
- `docs/AGENT_WORKFLOW.md` — authoritative source for implementation governance,
  approval gates, scope control, and required workflow records.

This is preferable to putting every rule into one large document: testing
policy answers **what evidence is needed**, while governance answers **when and
how work may proceed**. The two sources should cross-link, but neither should
silently redefine the other.

Recommended sequence:

1. **Inventory all normative statements.** Collect every TDD, testing,
   evidence, approval, validation, scope, and record-keeping rule from the
   sources listed above. Mark each statement as canonical, repeated,
   specialized, or merely descriptive.
2. **Identify redundancies and conflicts.** Look for repeated versions of the
   red/green/refactor loop, mandatory coverage, focused validation, broader
   validation, Track gates, approval gates, and evidence-recording rules. Also
   identify rules that are stricter in an operational file than in the stated
   project policy.
3. **Establish authority.** Keep the TDD/testing rules in `docs/TESTING.md`
   and the governance rules in `docs/AGENT_WORKFLOW.md`. Decide explicitly
   which rules belong in each source and remove policy wording from documents
   that should only link to them.
4. **Define the refined policy.** Only after the inventory and authority map
   are stable, decide which obligations remain universal and which become
   risk-tiered, optional, or boundary-specific.
5. **Align project and scoped instructions.** Update `AGENTS.md`,
   `.backlog/README.md`, `.backlog/AGENTS.md`, and `tests/AGENTS.md` to point to
   the authoritative wording rather than restating independent variants.
6. **Align operational enforcement.** Update `.opencode/agents/build.md`,
   `.opencode/agents/test.md`, `.opencode/agents/synthtracker.md`, and related
   skills so agents enforce only the refined policy.
7. **Update derived documentation.** Revise `tests/README.md`, `README.md`,
   and domain-specific design documents only where they currently impose or
   imply a policy that has changed.
8. **Validate the migration.** Search for the old wording, confirm that every
   remaining reference points to an authority, and ensure new Tracks inherit
   the refined rules without requiring changes to existing Track history.

The two primary control points are:

- `docs/TESTING.md` — what testing evidence is required.
- `docs/AGENT_WORKFLOW.md` — when and how that evidence must be produced.

## Proposed structure

Does this make sense? As a future structure, the project could be organized as follows:

SynthTracker/
├── src/
│   ├── component_a/ <-- PURE C (Chicago Style: Waveforms, mixing, logic)
│   └── component_b/ <-- PURE C (Chicago Style: Waveforms, mixing, logic)
└── tests/
    ├── integration/  <-- Integration tests executing full audio graph paths
    ├── fixtures/     <-- Shared fixtures for audio, compatibility, and memory
    └── units/        <-- Fast CMocka tests checking structural math and leaks
