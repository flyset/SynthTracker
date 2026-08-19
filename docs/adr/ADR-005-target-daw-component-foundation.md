# ADR-005: Target DAW Component Foundation

Status: Accepted

Decision date: 2026-08-19.

## Context

The current product is a transitional legacy CLI with a private playback seam.
The GUI-first DAW target needs an explicit vocabulary and high-level boundary
model before any implementation or extraction work. This record is target-only;
it does not describe the current implementation.

## Decision

Approve the target component foundation of `Main`, `Application`, `UI`,
`Editor`, `Model`, `Filesystem`, `File I/O`, `Input`, `Tracker`, `Synthesizer`,
`Mixer`, and `Audio Output`. `Playback Engine` names the emergent
`Tracker`/`Synthesizer`/`Mixer` subsystem and is not an additional component.

`Main` owns one `Application` and only process start/run/stop/status.
`Application` coordinates top-level lifecycles, configuration, and UI-request
dispatch, while delegating domain work and carrying no real-time musical routes.
`UI` reads/observes `Model`; mutations flow through `Application` and `Editor`
to `Model`. `Editor` owns edit commands and undo/redo. `Model` owns
authoritative DAW project data and persistent in-memory `File Information`
metadata/reference, including at least path and filename. This
metadata/reference is not serialized persistent project-format data, and
`Model` has no filesystem authority.

`Filesystem` is limited to bounded directory browse/list operations and file
deletion only; it produces `File Information` and never reads or writes
content. `File I/O` directly reads and writes content only using `File
Information` provided by `Model`, and translates
content to/from `Model` data for bounded format import/load, save/export, and
rendered-audio export. This preserves the no-shell and no-unrestricted-
filesystem guardrail.

`Input` is musical performance input only. `Tracker` reads `Model` song data,
produces timed musical events, and routes recording through `Editor`.
`Synthesizer` reads `Model` configurations for active engine instances and
renders multiple independent instances simultaneously, one stream per
instance, targetable by `Tracker`/`Input` events. Instances are neither threads
nor legacy `Channel`s. `Mixer` combines streams using `Model` mix data and
produces frames. `Audio Output` is a device-independent port; CoreAudio and
future platform adapters implement it.

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

## Principal flows

The approved target flows are: browse/list/file-delete `UI` → `Application` →
`Filesystem`; load metadata `UI` → `Application` → `Filesystem` → `Application`
→ `Model` for `File Information`, followed by `Application` retrieving it from
`Model` and invoking `File I/O` to load content into `Model`; edit `UI` →
`Application` → `Editor` → `Model`; audition `Input` → `Synthesizer`; record `Input` →
`Tracker` → `Editor` → `Model`; and
playback/export `Model` → `Tracker` → targeted `Synthesizer` instance → streams
→ `Mixer` → frames → `Audio Output` or `File I/O`.

Multiple synthesizer instances remain inside one playback context and never
create independent playback contexts.

## Consequences

- Target responsibilities and least-authority relationships are explicit.
- Playback remains UI-independent and audio output remains device-independent.
- Existing TFMX module compatibility remains a requirement, not a behavior
  change or format decision.

## Explicit deferrals and non-decisions

This target decision does not define APIs, concrete data types, lifetime or
allocation mechanics, raw/decoded representation, validation rules, threading
or scheduling, exact file contracts, or extraction mechanics. It does not
authorize implementation or extraction. It does not revise implemented private
playback-seam decisions, public C API/ABI, module compatibility, interpreter,
timing, audio behavior, persistent format, platform adapter behavior, or current
package boundaries. `Synthesizer` is not the rejected product category from the
Vision; `Tracker` is not a general-purpose tracker product.

## Related ASRs

- ASR-003 — UI-agnostic playback core.
- ASR-004 — Explicit, independently testable component boundaries.
- ASR-006 — Isolated future audio output.
- ASR-007 — Explicit filesystem and shell boundaries.

## Evidence

- [`ARCHITECTURE.md`](../ARCHITECTURE.md) — approved target architecture,
  principal flows, and current-versus-target boundary.
- [`GLOSSARY.md`](../GLOSSARY.md) — target-only terminology.
- [`TRACK_007_COMPLETED_target_architectural_foundation.md`](../../.backlog/COMPLETED/2026/TRACK_007_COMPLETED_target_architectural_foundation.md)
  — approved Track scope and decision log.
