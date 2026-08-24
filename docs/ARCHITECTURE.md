# Architecture

This is the concise current-system overview and architecture entrypoint. It
does not replace the decision and requirement registers:
[`ADR.md`](ADR.md) indexes Architectural Decision Records and
[`ASR.md`](ASR.md) records Architecturally Significant Requirements.

## Current system

SynthTracker is currently a transitional legacy CLI with a private device-driven
CoreAudio live route on macOS. TFMX names the legacy format, modules, and
semantics; it is not the product name. The legacy implementation is compiled
under the C23 baseline as a single executable with no SDL dependency:

- `src/main.c` is the minimal process entrypoint: it performs the root-user
  guard, invokes the application, and propagates its status.
- `src/application.c` coordinates CLI option parsing, loading calls, and the
  private live-output lifecycle: it prepares a private legacy exact-N renderer
  and a private CoreAudio adapter instance, starts the HAL Output Audio Unit
  route, and stops it on completion or interrupt. The removed `-b`, `-8`, `-f`,
  `-o`, `-w`, and `-v` options are rejected as unknown options.
- `src/playback/tfmx_loader.c` is the private bounded structural loader
  (Track 016): it checks TFMX magic and minimum size, subsong-0
  `start[0]`/inclusive `end[0]` bounds, and table-pointer alignment/in-bounds
  safety, resolves zero raw
  `trackstart`/`pattstart`/`macrostart` header pointers to the documented
  defaults (0x800/0x400/0x600), scans the on-disk pattern and macro pointer
  tables independently for up to 128 aligned, readable entries each
  (requiring at least one valid entry per table), normalizes accepted offsets
  into its metadata arrays, and treats raw SMPL as opaque bytes with a
  two-byte minimum (leading zeros admitted; no macro-derived sample-range
  inference). Its evidence is automated self-authored structural contracts
  plus bounded supplemental manual corpus evidence; general real-module
  loader compatibility remains deferred after the historical Track 015 XOut2
  rejection and is not promised.
- `src/player.c` owns the interpreter, including trackstep → pattern → macro
  sequencing, macro execution, and effects.
- `src/playback/playback_legacy_renderer.c` is the private exact-N legacy
  renderer: it retains the current TFMX tick's remaining frames, advances the
  interpreter exactly once when the retained tick is exhausted, and mixes
  across ticks to fill each exact device-requested frame count.
- `src/audio_output/` owns the private Audio Frame Block boundary, the
  synchronous exact-N device-demand coordinator, the private CoreAudio adapter,
  and the private CoreAudio system-call facade (see below).

This is a source-level structural extraction only. The `main.c`/`application.c`
seam does not provide the approved target `Model`, `Playback Engine`, `Mixer`,
or `Audio Output` architecture, and it does not establish encapsulated reusable
components or a public API.

### Header placement

The three remaining legacy headers (`player.h`, `tfmx.h`, and `tfmxsong.h`)
live under `src/`; the legacy `audio.h` was retired with the SDL-era audio
path. The extracted private playback headers and `application.h` are
co-located with their owning source folders. Every project-owned production
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

The private copied legacy bridge (`src/playback/playback_legacy_bridge.c`)
owns capacity-128 normalized pattern/macro arrays (Track 016): counts are
constrained to 1..128, every loader metadata index is validated non-negative
and within the complete copied MDAT word range, and the validated entries are
copied before the interpreter's `patterns`/`macros` globals bind to the
bridge-owned arrays — never aliasing the copied on-disk table region inside
`editbuf`. Bounded trackstep conversion over the resolved
`[trackstart, first_pattern)` range is preserved, and reset clears the
arrays. It remains single-global and non-reentrant. The private `src/playback`
seam provides a fixed-eight voice snapshot and is SDL-free, single-global,
non-reentrant, and not a public API or MCP surface.

The SDL 1.2-era audio API surface is retired: the legacy SDL live-audio path,
SDL linkage, SDL test scaffolding, and the `-o` file-output path are removed.
SDL 1.1.7 is historical legacy context, not an asserted current build
dependency; SDL remains only a future GUI decision.

### Bounded structural loader admission (Track 016)

Track 016 replaced the finite-fixture content recognizer with bounded
structural load/start admission in the private loader/bridge. This is a
private, non-promissory Phase 4 scaffold — not a general TFMX format
validator, a loader redesign, or a SynthTracker v1 compatibility promise.

- `src/playback/tfmx_loader.c` admits a candidate by structural rules only:
  TFMX magic and minimum size, subsong-0 `start[0]`/inclusive
  `end[0]` bounds, table-pointer alignment and in-bounds checks, and
  candidate-metadata safety. Zero raw
  `trackstart`/`pattstart`/`macrostart` header pointers resolve to the
  documented defaults 0x800/0x400/0x600 (`docs/TFMXLegacy/FORMAT.md:65-74`).
  The on-disk pattern and macro pointer tables are scanned independently for
  up to 128 readable, aligned raw targets each — stopping before an
  unreadable cell and on zero, below-0x200, unaligned, or out-of-bounds
  entries — with at least one valid entry per table; accepted offsets are
  normalized into the existing metadata arrays, and `first_pattern` derives
  from the first normalized pattern. Primary load admission additionally
  requires `first_pattern` to be strictly after `trackstart`, and a subsong-0
  inclusive `end` that fits all `end + 1` complete 16-byte tracksteps within
  `[trackstart, first_pattern)`. The exact trackstep/pattern/macro
  content comparisons and first-macro sample-range inference are removed.
- Raw SMPL is opaque byte data with no header: a two-byte minimum is
  retained, leading zeros are admitted, and no loader sample-range inference
  is performed from macro content.
- `src/playback/playback_legacy_bridge.c` owns the private capacity-128
  normalized pattern/macro arrays described above (validation, copy-before-
  bind, reset, and bounded trackstep conversion). Before legacy state
  binding/start, the bridge defensively repeats both load/start checks —
  `first_pattern` strictly after `trackstart`, and the subsong-0 inclusive
  `end` requiring all `end + 1` complete 16-byte tracksteps within
  `[trackstart, first_pattern)`.
- This is a restrictive private structural-admission correction only: it
  changes no public API/ABI, artifact contract, timing/interpreter/audio
  behavior for accepted modules, persistence, adapter, or compatibility
  promise.
- Evidence is automated self-authored structural contract tests
  (`tests/playback/`, deterministic and independent of the external corpus)
  plus bounded supplemental manual corpus evidence only: the four selected
  directories (`Turrican1`, `Turrican2`, `R-type`, `Apprentice`) with 21
  paired modules across 11 structural families and 14 default-pointer
  layouts, and the two user-confirmed smoke cases (`Turrican2-LVL1`,
  `Turrican1-LVL1`). No format-wide compatibility, exact audio, or resolved
  timing/effects/loop claim is made. The recorded XOut2 rejection remains a
  historical Track 015 record; Track 016 later delivered bounded admission,
  and general real-module loader compatibility and the loader redesign remain
  deferred.

### Private audio-output live route (Phase 4)

The private live route is now device-driven and audible: the application
composes a private legacy exact-N renderer with a private CoreAudio adapter
instance, and the macOS HAL Output Audio Unit render callback drives the
private render boundary for each device-requested nonzero frame count (a
zero-frame workspace/HAL request is accepted at the adapter's admission gate
without invoking the renderer or the exact-N coordinator):

- `src/audio_output/` is a private signed-32 Audio Frame Block implementation:
  a block carries a frame count and borrowed interleaved signed-32
  `{ left, right }` frames (never serialized PCM, never device-native data).
  The private synchronous exact-N coordinator
  `audio_output_coordinate_frame_request` requests exactly N frames from the
  renderer, rejects a returned block whose frame count differs from N or a
  nonzero block with NULL frame storage without delivery, padding, or
  truncation, and otherwise delivers the same borrowed block once.
- `src/playback/playback_legacy_renderer.c` fulfills exact device requests from
  the legacy engine: it retains the current TFMX tick's remaining frames,
  advances the interpreter exactly once when the retained tick is exhausted,
  and mixes across ticks as needed to fill each exact request, using
  lifecycle-preallocated storage with no allocation on the callback path. The
  private legacy producer (`playback_legacy_mixer_render_frames`) first
  computes blended int32 lanes from the mixed voices using the legacy fixed
  L/R blend, then explicitly reconstructs each blended lane's low-16
  historical PCM-domain value and multiplies by 65536 to produce the
  signed-32 Audio Frame Block values; the generic adapter conversion stays
  signed-32/2^31.
- `src/audio_output/adapters/coreaudio_adapter.c/.h` is a private macOS-only
  CoreAudio adapter (composed on Apple platforms only). Its request entry runs
  a lock-free OPEN/IN_FLIGHT atomic admission gate, converts signed-32
  `{ left, right }` blocks to interleaved Float32
  (`(float)sample / 2147483648.0f`, INT32_MIN → -1.0f, INT32_MAX → +1.0f) into
  a validated borrowed workspace, and copies the converted samples into the
  separate native output buffer on the HAL route; the direct delivery mode
  remains valid only for the fake/test route.
- `src/audio_output/adapters/coreaudio_facade.c/.h` is the private CoreAudio
  system-call facade. On Apple platforms the production default facade is a
  real HAL Output Audio Unit: it opens the default output device, negotiates
  the device's nominal sample rate (a requested rate of 0 is the private
  startup-selection sentinel; after device open only an actual nominal 44.1 or
  48 kHz rate is accepted and reported back as the verified configured rate,
  explicit 44.1/48 requests retain equality checking, and any other nonzero
  request is preflight-invalid), requires strict interleaved Float32 stereo
  at 44.1 or 48 kHz, rejects every other negotiated rate before activation,
  and drives the render callback with a lock-free admission gate and
  control-side quiescence; it performs no allocation, locking, file I/O, or UI
  work on the callback path, and no resampler is introduced. Elsewhere the
  production default facade returns `UNAVAILABLE`.
- `src/application.c` performs the cutover: it loads the module, requests the
  private rate-0 startup-selection sentinel in workspace delivery mode,
  prepares the renderer in the adapter's control-side preparation callback at
  the facade-verified 44.1/48 kHz configured rate, starts the route, waits for
  playback completion or an interrupt, and stops through control-side
  quiescence.
- Retired: the legacy SDL live-audio callback, device lifecycle, conversion,
  ring queue, throttle, drain, SDL teardown, and pthread synchronization; the
  `-o` file-output path; the strict temporary live profile; and the removed
  `-b`, `-8`, `-f`, `-o`, `-w`, and `-v` options (each rejected as an unknown
  option with usage and a non-zero exit status).
- Composition is direct and private: no library, public API, public header, or
  public target `Mixer` is introduced. On Apple platforms, `SynthTracker` and
  the affected test targets compose the private CoreAudio adapter and facade
  directly; the test-only fake facade drives lifecycle, negotiation, and
  zero/variable frame requests deterministically without hardware. Other
  application/audio tests use target-local doubles or the fake facade as
  applicable.

### Callback-driven render direction (private route implemented)

ADR-009 formalizes the approved live-route rendering direction for the private
Track 015 CoreAudio route: CoreAudio owns render timing and drives a
private render boundary that produces every requested Audio Frame Block —
variable-size and zero-frame requests included — from current/future playback
state and time-ordered events, never from pre-rendered audio. On the
implemented workspace/HAL route, each nonzero request is answered exact-N
through that boundary, while a zero-frame request is accepted at the adapter
without invoking the renderer or the exact-N coordinator; the broader
"every request rendered" phrasing remains the approved direction. The
callback path
uses lifecycle-preallocated storage and performs no allocation, locking, file
I/O, UI work, or other unbounded work; adapter-private conversion remains
separate; and no public Audio Output Port, C API, header, or export is
introduced. ADR-010 (Accepted, 2026-08-23) refines the private responsibility
allocation: native audio adapters own their device callbacks, device lifecycle,
and device-format conversion, while the private `audio_output` component owns
the synchronous device-demand coordinator — the coordinator function
(`audio_output_coordinate_frame_request`) is device-independent and, for each
adapter request of exactly N frames, requests exactly N Audio Frame Block
frames from the private renderer and immediately routes the result to the
requesting adapter, containing no software queue or timer. The private
`audio_output` translation unit still includes `audio_output_dispatch_submit`,
which calls the CoreAudio adapter dispatch on Apple platforms; removing that
dispatch remains an outstanding private refactor/deferral.

Track 015 S5 implemented the private device-driven route under this direction
and completed the application cutover: the real legacy exact-N renderer, the
private CoreAudio adapter lifecycle/callback with control-side quiescence and
lock-free admission gate, workspace-only conversion, the real HAL Output Audio
Unit facade with a strict 44.1/48 kHz negotiated-rate gate, and the deletion of
the SDL live-audio path, the `-o` file-output path, and the `-b`, `-8`, `-f`,
`-o`, `-w`, and `-v` options. The S4/S5.1–S5.5 device-free coordination,
bound-instance, quiescence, workspace, and admission evidence remains in the
Track as the deterministic test foundation for this route. Track 015
S6.2–S6.4b delivered the private rate-0 startup-selection sentinel with a
verified 44.1/48 kHz configured rate and control-side renderer preparation
before bind/start, and the private legacy producer's blend-then-widen
normalization (blended int32 lanes reconstructed to their low-16 historical
PCM-domain value and multiplied by 65536 into the existing signed-32 Audio
Frame Blocks); the generic
adapter conversion remains signed-32/2^31. `voices_01` is a self-authored
supplemental audible-smoke fixture, and a listener confirmed real 48 kHz
`voices_01` playback. Track 015 S6.5 (final validation/documentation) is
complete: the full CTest suite passes 8/8, the required documentation was
reconciled, and the Phase 4 roadmap was revised (Phase 4 revision 14; canonical
index revision 21). S7 (completion) is checked: Track 015 is completed at
`.backlog/COMPLETED/2026/TRACK_015_COMPLETED_coreaudio_live_route_and_sdl_retirement.md`.
Stage 3 remains in progress pending its wider acceptance/merge.

This direction remains target-only at the public boundary: the public Audio
Output Port, public C API, target `Mixer`, non-macOS adapters, the future GUI,
live input, rendered-file export, the device-rate-change restart policy,
workspace release/close, invalid storage-length proof, and general real-module
loader expansion remain deferred. General real-module loader compatibility in
particular remains deferred: the recorded XOut2 rejection is historical Track
015 evidence, and Track 016 later delivered bounded structural load/start
admission (self-authored structural contracts plus bounded supplemental manual
corpus evidence, no format-wide promise), while the loader redesign and
format-wide compatibility stay deferred — the design does not promise
current behavior or compatibility. See
[`AUDIO_RENDERING_DESIGN.md`](AUDIO_RENDERING_DESIGN.md),
[ADR-009](adr/ADR-009-callback-driven-audio-rendering.md), and
[ADR-010](adr/ADR-010-native-adapter-ownership-and-private-demand-coordination.md).

### Phase 4 compatibility policy

Phase 3 is delivered and Phase 4 is in progress. During Phase 4, preserving
current TFMX behavior where practical is a temporary development scaffold only.
Every Phase 4 Track must assess compatibility impact on relevant TFMX modules,
trackstep, pattern, macro, timing, interpreter, and audio semantics, and retain
appropriate evidence. This is not a SynthTracker v1 compatibility promise.
Track 016 assessed its impact as a bounded private module-admission change
with no intended interpreter/timing/audio semantic change beyond admissibility
and reachability; observed playback differences are recorded, not resolved.

## Current validation boundary

The current baseline is C23 validated on macOS with Clang. macOS is the current
and only platform scope. Other-platform support requires an explicit product
decision recorded in project memory. The SDL 1.2-era audio API surface and the
legacy `-o` output path are retired; live playback is the private device-driven
CoreAudio route described above (HAL Output Audio Unit facade on macOS,
unavailable production facade elsewhere), and the removed CLI options are
rejected as unknown options.

### Validation by boundary

Validation follows ownership rather than source-text placement: component
tests exercise observable component contracts, application-level tests exercise
observable workflows and composition, and build/link/executable integration
checks cover `Main` and executable composition. Compatibility fixtures and
direct checks provide bounded supplemental evidence. Track 016's loader and
bridge contracts are covered by deterministic self-authored structural
contract tests in `tests/playback/`; the external selected corpus is
supplemental manual evidence only and is not a repository automated acceptance
criterion. `main.c` remains minimal
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
  Callback-driven rendering — the device owns render timing and the private
  render boundary produces every requested block — is the approved live-route
  direction per [ADR-009](adr/ADR-009-callback-driven-audio-rendering.md) and
  is implemented privately by Track 015 (device-driven HAL Output Audio Unit
  route, where a zero-frame workspace/HAL request is accepted at the adapter
  without invoking the renderer or the exact-N coordinator), while this public
  port remains target-only. The private
  `audio_output` demand coordinator is a Track
  015 private role distinct from this public port; ownership is decided by
  [ADR-010](adr/ADR-010-native-adapter-ownership-and-private-demand-coordination.md).
  Track 015 S5 implemented the device-free exact-N coordinator policy, the
  device-free private adapter-owned bound instance, the device-free
  deferred-stop-to-quiescence, the device-free borrowed preallocated conversion
  workspace, the device-free workspace-mode startup rejection of invalid
  preparation (NULL samples, zero capacity, and capacity
  greater than `SIZE_MAX / 2` rejected before facade
  lifecycle/bind/request/render/observer/allocation work, with the instance
  INACTIVE and the direct bound-instance route valid), the real legacy exact-N
  renderer, the lock-free admission gate, the HAL workspace-copy route, the
  real HAL Output Audio Unit lifecycle/callback with control-side quiescence,
  and the application cutover; the device-driven wiring is implemented and this
  public port, `Mixer`, and the remaining deferred mechanics stay deferred.
  Track 015 S6.2–S6.4b delivered the private rate-0 startup-selection sentinel
  with control-side renderer preparation at the facade-verified 44.1/48 kHz
  configured rate and the private legacy producer's blend-then-widen
  normalization (blended int32 lanes reconstructed to their low-16 historical
  PCM-domain value and multiplied by 65536 into the existing signed-32 Audio
  Frame Blocks); S6.5 (final
  validation/documentation) is complete (full CTest 8/8, required
  documentation reconciled, Phase 4 roadmap revised to revision 14 with
  canonical index revision 21) and S7 (completion) is checked: Track 015 is
  completed (`.backlog/COMPLETED/2026/TRACK_015_COMPLETED_coreaudio_live_route_and_sdl_retirement.md`)
  while Stage 3 remains in progress pending its wider acceptance/merge.
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
- [`AUDIO_RENDERING_DESIGN.md`](AUDIO_RENDERING_DESIGN.md) — callback-driven
  audio rendering design direction.
- [`MACRO_DESIGN.md`](MACRO_DESIGN.md) — pre-design macro-layer questions.
- [`TFMXLegacy/README.md`](TFMXLegacy/README.md) — legacy format and player
  reference.
- [`../MEMORY.md`](../MEMORY.md) — product-management decisions and roadmap
  context.
- [`../.backlog/README.md`](../.backlog/README.md) — Track governance and
  execution records.
