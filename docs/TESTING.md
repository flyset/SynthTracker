# Testing Strategy

This is the canonical detailed testing strategy for SynthTracker. It applies
to the current transitional CLI and to progressively extracted components.

## Evidence levels

1. **Component tests** exercise observable component contracts.
2. **Application-level tests** exercise observable workflows and composition.
3. **Build/link/executable integration checks** cover `Main` and executable
   composition.
4. **Compatibility fixtures and direct checks** provide bounded supplemental
   evidence for legacy TFMX behavior; they do not replace automated coverage.

Source-text, symbol-placement, and layout inspection can support review of a
boundary, but they are not behavioral evidence.

## TDD and behavior coverage

Before behavior or component work, add a failing automated test for the
intended observable behavior. Make the smallest change that passes it, then
refactor and run the relevant validation. Every behavior change requires
automated coverage. Direct TFMX checks and compatibility evidence remain
bounded supplements.

`main.c` remains minimal. Cover it through compile, link, and executable
integration behavior, not a source-text existence or placement test. Put
application behavior in application-level tests. The current source-level
`Application` boundary is exercised by `tests/application/test_application.c`.
Its coverage is deliberately bounded and non-root: it calls `application_run`
directly to verify missing program arguments and an invalid option both return
status `2` and write `Usage:` to standard error. The process-level root-user
guard and executable composition remain compile/link/executable integration
concerns for `Main`, not source-text placement tests.

## Ownership, layout, and naming

Tests co-evolve with progressively extracted production components and mirror
component ownership. Do not organize production tests by the technique used to
locate source text. Name test files and fixtures for the component and contract
they exercise, and place them in the corresponding component-owned tree.
Shared fixtures may have a separate fixture area when ownership is shared. Test
trees evolve as components are extracted; current directories are not a fixed
future component list.

The current private `src/playback/` subtree and `tests/playback/` evidence are
temporary compatibility scaffolding. They do not declare target product
architecture. The source-level `Application` seam and its
`tests/application/test_application.c` tests now exist, but only cover the
bounded non-root usage/error status and output behavior described above.
Workflow/composition coverage will expand as implemented behavior expands;
`Main` remains covered by compile/link/executable integration checks rather than
source-text placement tests. The future target architecture remains
unimplemented beyond this structural seam.

## Current fixtures and compatibility evidence

Fixtures under `tests/fixtures/` are self-authored, bounded inputs and
documented malformed variants used by private playback evidence. Tests assert
observable loading, playback, rendering, completion, or rejection behavior;
fixture layout is not itself a product contract. Direct checks may supplement
these tests when assessing Phase 4 compatibility impact, but are not a
SynthTracker v1 compatibility promise.

## Current validation commands

The current CMake/CMocka validation uses a Homebrew CMocka installation on
macOS. From the repository root:

```bash
cmake -S . -B build -DCMAKE_PREFIX_PATH="$(brew --prefix cmocka)"
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

The README also documents the equivalent configure/build flow using `cmake ..`
followed by `make`. Focused CMocka targets are registered with CTest; use the
current CMake target names when a focused check is required.
