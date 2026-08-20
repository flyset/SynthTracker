# Architecture

This is the concise current-system overview and architecture entrypoint. It
does not replace the decision and requirement registers:
[`ADR.md`](ADR.md) indexes Architectural Decision Records and
[`ASR.md`](ASR.md) records Architecturally Significant Requirements.

## Current system

SynthTracker is currently a transitional legacy CLI with SDL-backed audio. TFMX
names the legacy format, modules, and semantics; it is not the product name. The
legacy implementation is compiled under the C23 baseline as a single
SDL-linked executable:

- `src/main.c` is the minimal process entrypoint: it performs the root-user
  guard, invokes the application, and propagates its status.
- `src/application.c` coordinates CLI option parsing, loading calls,
  debug/export dispatch, and the playback lifecycle.
- `src/tfmx.c` retains the legacy TFMX format detection, loading, module-data
  logic, and associated global state.
- `src/player.c` owns the interpreter, including trackstep → pattern → macro
  sequencing, macro execution, and effects.
- `src/audio.c` owns mixing, filtering, stereo blending, ring-buffer handling,
  SDL audio callbacks, and pthread synchronization.

This is a source-level structural extraction only. The `main.c`/`application.c`
seam does not provide the approved target `Model`, `Playback Engine`, `Mixer`,
or `Audio Output` architecture, and it does not establish encapsulated reusable
components or a public API.

### Header placement

The four legacy headers (`player.h`, `audio.h`, `tfmx.h`, and `tfmxsong.h`) now
live under `src/`. The extracted private playback headers and `application.h`
are co-located with their owning source folders. Every project-owned production
and test header now lives in the same owning source or test folder as its
owning C source. This is folder co-location, not a one-to-one source/header
basename rule.

`include/` is retired and contains no project-owned headers. Third-party,
generated, and platform SDK headers are outside this layout decision. This end
state does not create a public C API or a library-header model. See
[ADR-006](adr/ADR-006-private-header-colocation-and-include-retirement.md) and
[ASR-008](ASR.md#asr-008--co-located-project-owned-headers-and-include-retirement).

The runtime state is global and shared across these files. Module data is
mutated in place on load: network-order values are converted in `editbuf`, and
file pointers become array indices. Per-song behavior hacks remain global.

The private copied legacy bridge resets bridge-owned runtime state for a fresh
start but remains single-global and non-reentrant. The private `src/playback`
seam provides a fixed-eight voice snapshot and is SDL-free, single-global,
non-reentrant, and not a public API or MCP surface.

Audio remains bound to the SDL 1.2-era API surface used by the engine. SDL 1.1.7
is historical legacy context, not an asserted current build dependency.

### Phase 4 compatibility policy

Phase 3 is delivered and Phase 4 is next. During Phase 4, preserving current
TFMX behavior where practical is a temporary development scaffold only. Every
Phase 4 Track must assess compatibility impact on relevant TFMX modules,
trackstep, pattern, macro, timing, interpreter, and audio semantics, and retain
appropriate evidence. This is not a SynthTracker v1 compatibility promise.

## Current validation boundary

The current baseline is C23 validated on macOS with Clang. macOS is the current
and only platform scope. Other-platform support requires an explicit product
decision recorded in project memory. The current SDL-era audio boundary remains
the implemented output path.

### Validation by boundary

Validation follows ownership rather than source-text placement: component
tests exercise observable component contracts, application-level tests exercise
observable workflows and composition, and build/link/executable integration
checks cover `Main` and executable composition. Compatibility fixtures and
direct checks provide bounded supplemental evidence. `main.c` remains minimal
and is not validated by source-text existence or placement tests. The private
`src/playback/` subtree is temporary compatibility evidence, not target
application architecture. See [`TESTING.md`](TESTING.md).

## Approved target architecture (not implemented; structural seam only)

The following remains an approved target foundation only. Its target components,
flows, and boundaries are not implemented; the current source-level
`main.c`/`application.c` seam is structural and does not implement the target
`Main` or `Application` contracts. This section does not define APIs, concrete
data types, extraction mechanics, or implementation contracts.

- `Main` owns one `Application` and performs only process start, run, stop, and
  process-status handling.
- `Application` owns and coordinates top-level lifecycles, configuration, and
  UI-request dispatch. It does not perform domain work or carry real-time
  musical routes.
- `UI` observes and reads `Model`; mutations flow `UI` → `Application` →
  `Editor` → `Model`.
- `Editor` owns edit commands and undo/redo.
- `Model` owns authoritative DAW project data and persistent in-memory `File
  Information` metadata/reference (at least path and filename). This metadata/
  reference is not serialized persistent project-format data, and `Model` has
  no filesystem authority.
- `Filesystem` performs bounded directory browse/list operations and file
  deletion only, and produces `File Information`; it never reads or writes
  file content.
- `File I/O` uses `Model`-provided `File Information` to directly read/write
  content and translate `Model` data for bounded format import/load, save/export,
  and rendered-audio export.
- `Input` is musical performance input only.
- `Tracker` reads song data from `Model`, produces timed musical events, and
  routes recording through `Editor`.
- `Synthesizer` reads `Model` configurations defining active engine instances;
  it renders multiple independent instances simultaneously, with one stream
  per instance. Tracker/Input events can target instances. Instances are
  neither threads nor legacy `Channel`s; scheduling is deferred.
- `Mixer` receives synthesizer streams and `Model` mix data and produces `Audio
  Frame Blocks`.
- `Audio Output` consumes `Audio Frame Blocks` through a device-independent
  port. CoreAudio and future adapters are implementations of that boundary.
- `Playback Engine` is the emergent `Tracker` + `Synthesizer` + `Mixer`
  subsystem, not another component.

### Principal permitted relationships (target only)

The following table names principal permitted relationships. It is not an
exhaustive dependency matrix; detailed contracts and additional relationships
remain deferred to later Tracks.

| From | To | Permitted responsibility |
| --- | --- | --- |
| `Main` | `Application` | Process lifecycle: start, run, stop, and status. |
| `UI` | `Model` | Read and observe project data. |
| `UI` | `Application` | Route UI commands and mutations. |
| `Application` | `Editor` | Coordinate edit requests. |
| `Application` | `Model` | Store and retrieve `File Information`; coordinate top-level lifecycle and configuration. |
| `Application` | `Filesystem` | Request bounded browse, list, and file-delete operations. |
| `Filesystem` | `Application` | Return `File Information`; it has no `Model` dependency. |
| `Application` | `File I/O` | Invoke bounded content load, save, and export operations. |
| `File I/O` | `Model` | Translate loaded or saved content to or from `Model`, using `Model`-provided `File Information`; it has no `Filesystem` dependency. |
| `Application` | `Tracker`, `Synthesizer`, `Mixer` | Configure real-time routes; it does not carry musical events. |
| `Editor` | `Model` | Apply edits. |
| `Input` | `Synthesizer` | Send audition events. |
| `Input` | `Tracker` | Send capture input. |
| `Tracker` | `Model` | Read song data. |
| `Tracker` | `Editor` | Send recorded edits. |
| `Tracker` | `Synthesizer` | Send timed musical events. |
| `Synthesizer` | `Model` | Read active-instance configuration. |
| `Synthesizer` | `Mixer` | Send per-instance streams. |
| `Mixer` | `Model` | Read mix data. |
| `Mixer` | `Audio Output` | Send audible frames. |
| `Mixer` | `File I/O` | Send export frames. |

### Principal target flows (not implemented)

- Browse/list/file-delete: `UI` → `Application` → `Filesystem`.
- Load metadata: `UI` → `Application` → `Filesystem` → `Application` → `Model`
  for `File Information`; `Application` then retrieves it from `Model` and
  invokes `File I/O`, which loads content into `Model`.
- Edit: `UI` → `Application` → `Editor` → `Model`.
- Audition: `Input` → `Synthesizer`.
- Record: `Input` → `Tracker` → `Editor` → `Model`.
- Playback/export: `Model` → `Tracker` → targeted `Synthesizer` instance →
  streams → `Mixer` → frames → `Audio Output` or `File I/O`.

Multiple `Synthesizer` instances remain inside one playback context and never
create independent playback contexts. Lifetime/allocation mechanics,
raw/decoded representation, validation, APIs, threading/scheduling, exact file
contracts, and extraction are explicitly deferred.

## Navigation

- [`ADR.md`](ADR.md) — ADR index and governance.
- [`ASR.md`](ASR.md) — architecturally significant requirements.
- [`VISION.md`](VISION.md) — product intent, boundaries, and future direction.
- [`GLOSSARY.md`](GLOSSARY.md) — canonical product and protocol terminology.
- [`ARTIFACTS.md`](ARTIFACTS.md) — target component-boundary artifacts and open
  contracts.
- [`MACRO_DESIGN.md`](MACRO_DESIGN.md) — pre-design macro-layer questions.
- [`TFMXLegacy/README.md`](TFMXLegacy/README.md) — legacy format and player
  reference.
- [`../MEMORY.md`](../MEMORY.md) — product-management decisions and roadmap
  context.
- [`../.backlog/README.md`](../.backlog/README.md) — Track governance and
  execution records.
