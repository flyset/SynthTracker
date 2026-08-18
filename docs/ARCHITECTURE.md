# Architecture

This is the concise current-system overview and architecture entrypoint. It
does not replace the decision and requirement registers:
[`ADR.md`](ADR.md) indexes Architectural Decision Records and
[`ASR.md`](ASR.md) records Architecturally Significant Requirements.

## Current system

TFMX.cpp is currently a transitional legacy CLI with SDL-backed audio. The
legacy implementation is compiled under the C23 baseline as a single
SDL-linked executable:

- `src/tfmx.c` owns `main()`, argument parsing, file loading, and byte-sniffing
  format detection.
- `src/player.c` owns the interpreter, including trackstep → pattern → macro
  sequencing, macro execution, and effects.
- `src/audio.c` owns mixing, filtering, stereo blending, ring-buffer handling,
  SDL audio callbacks, and pthread synchronization.

The runtime state is global and shared across these files. Module data is
mutated in place on load: network-order values are converted in `editbuf`, and
file pointers become array indices. Per-song behavior hacks remain global.

The private copied legacy bridge resets bridge-owned runtime state for a fresh
start but remains single-global and non-reentrant. The private `src/playback`
seam provides a fixed-eight voice snapshot and is SDL-free, single-global,
non-reentrant, and not a public API or MCP surface.

Audio remains bound to the SDL 1.2-era API surface used by the engine. SDL 1.1.7
is historical legacy context, not an asserted current build dependency.

## Current validation boundary

The current baseline is C23 validated on macOS with Clang. macOS is the current
and only platform scope. Other-platform support requires an explicit product
decision recorded in project memory. The current SDL-era audio boundary remains
the implemented output path.

## Target-level decomposition

The following remains future/proposed and unresolved at the implementation
boundary: Editor, Loader/Writer, Module Domain Model, distinct Sequencing and
Synthesis responsibilities, Playback Engine, Audio Mixer, Audio Output Port,
and the intended macOS CoreAudio Adapter. No target seam or GUI/editing
functionality is claimed to be implemented here.

The future playback core must remain UI-agnostic and future component
boundaries must be explicit and independently testable. The exact loader →
domain-model → playback contract, ownership, lifetime, representation, GUI
design, and CoreAudio integration remain open. TFMX will never support
simultaneous independent playback contexts; reentrancy or concurrency for that
purpose is not a target requirement.

## Navigation

- [`ADR.md`](ADR.md) — ADR index and governance.
- [`ASR.md`](ASR.md) — architecturally significant requirements.
- [`VISION.md`](VISION.md) — product intent, boundaries, and future direction.
- [`GLOSSARY.md`](GLOSSARY.md) — canonical terminology.
- [`MACRO_DESIGN.md`](MACRO_DESIGN.md) — pre-design macro-layer questions.
- [`TFMXLegacy/README.md`](TFMXLegacy/README.md) — legacy format and player
  reference.
- [`../MEMORY.md`](../MEMORY.md) — product-management decisions and roadmap
  context.
- [`../.backlog/README.md`](../.backlog/README.md) — Track governance and
  execution records.
