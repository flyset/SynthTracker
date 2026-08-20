# Glossary

Canonical product and protocol terminology. Legacy definitions below are
code-verified against the implementation; the evidence-cited reference is
`TFMXLegacy/` (start at `TFMXLegacy/README.md`). Target-only terms are
decision-defined, explicitly marked, and are not claims about implementation.

- **SynthTracker** — the product and repository identity. **TFMX** denotes the
  legacy format, modules, semantics, and temporary compatibility lineage; it
  does not name the DAW product.

- **Macro (soundmacro)** — the instrument-level event stream in a TFMX
  module. A channel runs one macro per note, selected by the note event's
  macro number; macros control sample playback, volume, pitch effects, waits,
  loops, and sub-macro calls. Code-verified semantics: `TFMXLegacy/MACROS.md`.
  New-engine design work: `MACRO_DESIGN.md`.
- **Macro word / opcode** — a macro is a sequence of 32-bit words; the high
  byte (`b0`) is the opcode and `b1`–`b3`/`w1` are its arguments. Words are
  stored big-endian and converted with `ntohl` at use time
  (`src/player.c:141`).
- **Note event** — a pattern word that triggers a note: it records the note
  number, velocity, and finetune, and starts the macro selected by its macro
  number (`src/player.c:66`–`85`). See `TFMXLegacy/PATTERNS.md`.
- **Trackstep** — the sequencer row that selects a pattern block; subsongs
  are ranges of tracksteps (`start`/`end` per subsong in the header). See
  `TFMXLegacy/PLAYER.md`.
- **Pattern** — the per-channel event list carrying note events and pattern
  commands; the middle level of the trackstep → pattern → macro hierarchy.
  See `TFMXLegacy/PATTERNS.md`.
- **Channel** — the per-voice interpreter state (`struct Channel`,
  `src/player.h`): active macro pointer/step/number, flow flags, sample
  addressing, and effect state.
- **Playback context** — a playback-session object. Its current private
  bridge-backed execution is single-global and non-reentrant. TFMX supports at
  most one simultaneously active playback context; multiple channels or voices
  within one context are not independent playback contexts.
- **Effects** — per-tick channel modifiers applied after macro stepping
  (`DoEffects`, `src/player.c:504`): AddBegin sample-offset slide, vibrato,
  portamento, envelope; plus a global master-volume fade.
- **Macro flow (old-style / new-style)** — the two execution cadences
  distinguished by `NewStyleMacro`: old-style advances at most one opcode per
  tick; new-style runs non-blocking steps in the same tick
  (`src/player.c:113`; see `TFMXLegacy/MACROS.md`).
- **Raw versus decoded representation** — **open**. Whether the new engine
  keeps macros as arrays of 32-bit words or decodes them into typed
  structures is unresolved; see `MACRO_DESIGN.md`.
- **Module Domain Model** — future/proposed shared editable/playable model for
  TFMX module data. Its loader, writer, playback, ownership, lifetime,
  validation, and raw-versus-decoded contracts are **open**; it is not
  implemented by the current transitional CLI.
- **GUI-first DAW** — the future SynthTracker product direction: a modern C DAW
  using SDL on macOS.
- **Phase 4 compatibility scaffold** — temporary development policy to preserve
  current TFMX behavior where practical. Phase 3 is delivered; Phase 4 is next.
  Compatibility impact is assessed per Phase 4 Track and appropriate evidence
  retained. It is not a SynthTracker v1 compatibility promise.
- **Audio Output Port** — future/proposed device-independent playback-output
  boundary. **CoreAudio Adapter** is the intended macOS implementation; the
  integration and API are **open**.
- **Sequencing / Synthesis** — future/proposed distinct responsibilities that
  share a control vocabulary: Sequencing schedules musical structure, while
  Synthesis interprets voice and sound behavior. Neither target responsibility
  is implemented as a separate boundary.

## Target-only terminology (not implemented)

- **Main** — target process entrypoint owning one `Application` and only
  process start, run, stop, and status.
- **Application** — target coordinator for top-level lifecycles, configuration,
  and UI-request dispatch; it performs no domain work and carries no real-time
  musical routes.
- **UI** — target presentation boundary that reads/observes `Model` and sends
  mutations through `Application`.
- **Editor** — target owner of edit commands and undo/redo; it applies edits to
  `Model`.
- **Model** — target authoritative DAW project data, including persistent
  in-memory `File Information` metadata/reference; this metadata/reference is
  not serialized persistent project-format data. It has no filesystem
  authority and is distinct from a possible future `Module Domain Model`.
- **Filesystem** — target bounded directory browse/list and file-deletion-only
  boundary that produces `File Information` and never reads or writes file
  content.
- **File I/O** — target bounded content reader/writer and translator between
  file content and `Model` data, using `Model`-provided `File Information` for
  import/load, save/export, and rendered-audio export. This replaces the
  provisional target labels `Loader` and `Writer`.
- **Input** — target musical performance input only; ordinary UI input remains
  with `UI`.
- **Tracker** — target TFMX song sequencing/recording component that reads
  `Model`, produces timed musical events, and routes recording through
  `Editor`; it is not a general-purpose tracker product.
- **Synthesizer** — target component that reads configured active engine
  instances from `Model`, renders multiple independent instances, and produces
  one stream per instance. Instances are neither threads nor legacy `Channel`s.
  This is not the rejected product category named “synthesizer” in the Vision.
- **Mixer** — target component that receives synthesizer streams, reads `Model`
  mix data, and produces `Audio Frame Blocks`.
- **Audio Output** — target device-independent output port that consumes `Audio
  Frame Blocks`. A CoreAudio adapter or future platform adapter is an
  implementation of this port, not the port itself.
- **Audio Frame Block** — target artifact: a finite, ordered block of
  rendered audio frames produced by `Mixer` and consumed by `Audio Output` or
  `File I/O` for rendered-audio export. See [`ARTIFACTS.md`](ARTIFACTS.md),
  [`ADR-007`](adr/ADR-007-audio-frame-block-boundary-and-fixed-first-format.md),
  and [`ASR-009`](ASR.md#asr-009--audio-frame-block-boundary-invariants) for
  the artifact decision and contract invariants.
- **File Information** — target metadata/reference produced by `Filesystem` and
  retained persistently in memory by `Model`, including at least path and
  filename; it is not serialized persistent project-format data or file
  content, and is used by `File I/O`.
- **Synthesizer engine instance** — target independent renderable instance
  configured by `Model` and targetable by `Tracker`/`Input`; multiple instances
  render within one playback context. It is neither a `Channel` nor a thread.
- **Playback Engine** — target emergent subsystem consisting of `Tracker`,
  `Synthesizer`, and `Mixer`; it is not another component or an independent
  playback context.
