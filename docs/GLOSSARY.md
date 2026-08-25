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
- **post-interpreter `eClocks`** — the legacy interpreter's current tick-clock
  value captured by the private bridge after `tfmxIrqIn()` and alongside that
  tick's voice snapshots. The private playback context passes it to the
  existing mixer timing argument for that same rendered tick; mixer exact-N and
  remainder arithmetic are unchanged. This is a private Phase 4 handoff, not a
  public timing contract or compatibility promise.
- **`eClocks`** — the legacy interpreter tick-clock state. Local timing sources
  include song-start tempo, speed control, and timeshare control in
  `src/player.c`; a qualifying speed control (high mask passes; low9 is
  16..511) sets `eClocks = 0x1B51F8 / low9`. See **post-interpreter
  `eClocks`** for its implemented private same-tick use.
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
  current TFMX behavior where practical. Phase 3 is delivered; Phase 4 is in
  progress. Compatibility impact is assessed per Phase 4 Track and appropriate
  evidence retained. It is not a SynthTracker v1 compatibility promise.
- **Audio Output Port** — future/proposed device-independent playback-output
  boundary. **CoreAudio Adapter** is the intended macOS implementation; the
  integration and API are **open**. The private Phase 4 CoreAudio adapter below
  is a bounded Stage 3 step toward it, not the port itself. The private
  `audio_output` demand coordinator (see the entry below) is
  likewise not this port; see
  [ADR-010](adr/ADR-010-native-adapter-ownership-and-private-demand-coordination.md).
- **CoreAudio adapter (private, Phase 4)** — the implemented private
  macOS-only adapter at `src/audio_output/adapters/coreaudio_adapter.c` (with
  its private co-located header). It receives signed-32 Audio Frame Blocks from
  the private legacy exact-N renderer through the private synchronous exact-N
  coordinator and privately converts each block to its adapter-private
  interleaved Float32 representation (`float32 = int32 / 2147483648.0f`,
  INT32_MIN → -1.0f, INT32_MAX → +1.0f) in a validated borrowed workspace,
  copying the result into the separate native output buffer on the HAL route.
  Its request entry runs a lock-free OPEN/IN_FLIGHT admission gate, and the
  production default facade is a real HAL Output Audio Unit on macOS with a
  strict 44.1/48 kHz negotiated-rate gate, control-side quiescence, and no
  resampler. The application requests the private rate-0 startup-selection
  sentinel; after device open the facade accepts only an actual 44.1/48 kHz
  nominal rate and reports the verified configured rate, and the control side
  prepares the private legacy exact-N renderer at that rate before bind/start.
  A zero-frame request on the workspace/HAL route is accepted at the admission
  gate without invoking the renderer or the exact-N coordinator.
  The adapter's generic conversion remains signed-32/2^31; the private legacy
  producer (`playback_legacy_mixer_render_frames`) first computes blended
  int32 lanes from the mixed voices using the legacy fixed L/R blend, then
  explicitly reconstructs each blended lane's low-16 historical PCM-domain
  value and multiplies it by 65536 to produce the signed-32 Frame Block values.
  The adapter is not the
  target `Audio Output` port or an implementation of it.
- **Private `audio_output` demand coordinator** — the private synchronous
  exact-N device-demand coordinator role of `src/audio_output/` assigned by
  [ADR-010](adr/ADR-010-native-adapter-ownership-and-private-demand-coordination.md).
  For a request of exactly N frames it
  calls its renderer with exactly N, rejects a returned block whose frame count
  differs from N or a nonzero block with NULL frame storage — without delivery,
  padding, or truncation — and otherwise delivers the same borrowed block once
  and returns the delivery result.   Track 015 S5 completed the device-driven
  route around this coordinator: the real legacy exact-N renderer in
  `src/playback/playback_legacy_renderer.c`, the
  private adapter-owned bound instance with its lock-free admission gate and
  control-side quiescence, the borrowed preallocated conversion workspace and
  HAL workspace-copy delivery, and the real HAL Output Audio Unit facade; the
  application cuts over to this route and the SDL live-audio, `-o` file-output,
  and removed-option paths are deleted. The production default facade is a real
  HAL Output Audio Unit on macOS (strict 44.1/48 kHz negotiated-rate gate, no
  resampler) and `UNAVAILABLE` elsewhere; on the workspace/HAL route a
  zero-frame request is accepted at the adapter's admission gate before this
  coordinator is invoked. Track 015 S6.2–S6.4b delivered the
  private rate-0 startup-selection sentinel with a verified 44.1/48 kHz
  configured rate and control-side renderer preparation before bind/start, and
  the private legacy producer's blend-then-widen normalization (blended int32
  lanes reconstructed to their low-16 historical PCM-domain value and
  multiplied by 65536 into the existing signed-32 Frame Blocks); `voices_01`
  is the self-authored
  supplemental audible-smoke fixture, with a listener-confirmed real 48 kHz
  playback check. Track 015 S6.5 (final validation/documentation) is complete
  (full CTest 8/8, required documentation reconciled, Phase 4 roadmap revised
  to revision 14 with canonical index revision 21); S7 (completion) is
  checked and Track 015 is completed at
  `.backlog/COMPLETED/2026/TRACK_015_COMPLETED_coreaudio_live_route_and_sdl_retirement.md`,
  while Stage 3 remains in progress pending its wider acceptance/merge. The coordinator function
  (`audio_output_coordinate_frame_request`) contains no software queue or timer
  and is device-independent, and it is not the public `Audio Output` Port; the
  private `audio_output` translation unit still includes
  `audio_output_dispatch_submit`, which calls the CoreAudio adapter dispatch on
  Apple platforms — removing that dispatch is an outstanding private
  refactor/deferral. The public port,
  target `Mixer`, invalid storage-length proof, workspace release/close,
  device-rate-change restart policy, general real-module loader expansion, and
  other deferred mechanics remain
  deferred.
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
  implementation of this port, not the port itself; the private Phase 4
  CoreAudio adapter is not this port's implementation. Callback-driven
  rendering is the approved live-route direction; see the target-only terms
  below and [ADR-009](adr/ADR-009-callback-driven-audio-rendering.md). The
  private `audio_output` demand coordinator is distinct from this port; see
  [ADR-010](adr/ADR-010-native-adapter-ownership-and-private-demand-coordination.md).
- **Callback-driven audio rendering** — approved target direction for live
  audio output: the platform device (CoreAudio on macOS) owns render timing and
  pulls each requested `Audio Frame Block` from a private render boundary.
  Blocks are rendered on demand from current/future playback state and
  time-ordered events, never from pre-rendered audio. See
  [`ADR-009`](adr/ADR-009-callback-driven-audio-rendering.md) and
  [`AUDIO_RENDERING_DESIGN.md`](AUDIO_RENDERING_DESIGN.md);
  [ADR-010](adr/ADR-010-native-adapter-ownership-and-private-demand-coordination.md)
  refines the private responsibility allocation. Implemented as the private
  Track 015 live route (device-driven HAL Output Audio Unit on macOS), where
  each nonzero request is answered exact-N through the private render boundary
  and a zero-frame workspace/HAL request is accepted at the adapter without
  invoking the renderer or the exact-N coordinator;   Track
  015 S6.2–S6.4b delivered the negotiated-rate startup selection and
  control-side preparation handoff and the private legacy blend-then-widen
  normalization (blended int32 lanes reconstructed to their low-16 historical
  PCM-domain value and multiplied by 65536); S6.5 is complete and S7
  (completion) is checked: Track 015 is completed (Stage 3 remains in
  progress pending its wider acceptance/merge). The
  public `Audio Output` Port and
  target `Mixer` are not implemented.
- **Audio Render Boundary** — approved target private boundary that produces
  every device-requested `Audio Frame Block`, including variable-size and
  zero-frame requests, under the real-time contract: lifecycle-preallocated
  storage with no allocation, locking, file I/O, UI work, or other unbounded
  work on the callback path. Adapter-private conversion remains outside this
  boundary, and the boundary introduces no public Audio Output Port, C API,
  header, or export. See [ADR-009](adr/ADR-009-callback-driven-audio-rendering.md)
  and [ADR-010](adr/ADR-010-native-adapter-ownership-and-private-demand-coordination.md).
  The private Track 015 implementation is the legacy exact-N renderer at
  `src/playback/playback_legacy_renderer.c`; on the implemented workspace/HAL
  route a zero-frame request is accepted at the adapter without invoking the
  renderer or the exact-N coordinator, while each nonzero request is produced
  on demand with exactly the requested frame count. The broader "every request
  rendered" vocabulary remains the approved target direction; the target
  vocabulary and public
  port remain not implemented.
- **Audio Frame Block** — target artifact: a finite, ordered block of
  rendered audio frames produced by `Mixer` and consumed by `Audio Output` or
  `File I/O` for rendered-audio export. See [`ARTIFACTS.md`](ARTIFACTS.md),
  [`ADR-008`](adr/ADR-008-audio-frame-block-mixed-value-boundary.md), and
  [`ASR-009`](ASR.md#asr-009--audio-frame-block-boundary-invariants) for
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
