# Macro (Soundmacro) Design

Status: **Pre-design / open**. This document records why the macro layer
matters, what the code-verified behavior is, and the open design questions. It
resolves nothing: in particular, it does not decide how macros are represented
in the new engine.

## Purpose

Macros are the instrument-level event streams of a TFMX module. They are the
per-note voice programs that tell a channel which sample to play, at which
address and length, with which volume and pitch effects, and when to wait,
loop, or call another macro. This document gives the reimplementation a
shared, evidence-based picture of that layer before design work starts, and
keeps external context about the format's origins clearly separate from what
the local code actually does. Any future SynthTracker-owned macro implementation and
its tests must use C23 or a later ISO C standard; C++ is not a project
direction.

## Why macros matter

- **Temporary Phase 4 compatibility scaffold.** Phase 3 is delivered; Phase 4
  is next. During Phase 4, preserve current TFMX behavior where practical,
  including macro, timing, interpreter, and audio behavior. Every Phase 4 Track
  must assess compatibility impact and retain appropriate evidence. This is not a
  SynthTracker v1 compatibility promise.
  [`ADR-001`](adr/ADR-001-new-engine-not-line-by-line-port.md) records the
  new-engine approach informed by legacy ideas and semantics.
- **They carry the musical character.** Everything that makes an instrument
  sound like itself — sample selection and looping, envelopes, vibrato,
  portamento, volume splits, key-up behavior — is expressed in macros (see
  `docs/TFMXLegacy/MACROS.md`).
- **They are the inner level of the sequencer.** The legacy interpreter drives
  trackstep → pattern → macro (`docs/ARCHITECTURE.md`); per-channel state and
  per-tick effects hang off macro execution (`DoMacro`, `DoEffects`,
  `src/player.c`).
- **They inform future editing concepts.** TFMX macro semantics inform
  SynthTracker’s future editing concepts; the editable model and format remain
  open.

## Code-verified behavior (summary)

The following is a concise summary; the full evidence-cited opcode table and
flow details are in `docs/TFMXLegacy/MACROS.md`, with the discrepancy registry
in `docs/TFMXLegacy/PROVENANCE.md`.

- **Data.** Macros are arrays of 32-bit words with the opcode in the high
  byte; parameter bytes `b1`–`b3` and the low word `w1` carry arguments. Words
  are stored big-endian and converted with `ntohl` at use time
  (`src/player.c:141` in `RunMacro`); the loader does not decode them up front
  (`src/tfmx.c`). Macro pointers (`macros[]`) are resolved from the module's
  macro pointer table during load.
- **Selection.** A note event selects the macro by number (`b1` of the pattern
  word); `NotePort` sets `MacroPtr = macros[MacroNum]` and resets the flow
  state (`src/player.c:66`–`85`).
- **Execution.** Each channel steps its macro each tick through `DoMacro`
  (`src/player.c:600`), which counts down `MacroWait` and calls `RunMacro`
  (`src/player.c:134`). `RunMacro` advances `MacroStep` through the words and
  loops internally until an opcode yields or blocks. The `MAYBEWAIT` helper
  (`src/player.c:113`) distinguishes old-style flow (one opcode per tick) from
  new-style flow (non-blocking steps run in the same tick).
- **Effects.** After stepping, `DoEffects` (`src/player.c:504`) applies the
  per-tick modifiers: AddBegin sample-offset slide, vibrato, portamento, and
  envelope; a global fade runs on the master volume.
- **Sequencing.** `tfmxIrqIn` (`src/player.c:1065`) calls `DoAllMacros()`
  before `DoTracks()`, so macro and effect stepping precedes pattern and
  trackstep processing each tick.
- **State.** All of this lives in `struct Channel` (`src/player.h`): the
  macro pointer/step/number, flow flags, sample addressing, and effect state.
- **Known discrepancy.** The opcode count is recorded three ways (42-name
  debug table / 44 `case` labels / 47 in historical repository documentation);
  the conflict is registered in `docs/TFMXLegacy/PROVENANCE.md` and is
  **unresolved**. This document does not pick a side.

## External context

TFMX is the Amiga module format created by Chris Hülsbeck for his game
soundtracks, and the soundmacro concept originates in that creator ecosystem
[external]. Modules that use these macros are the composers' copyrighted works
[external]. None of that is observable in the local code, and this document
does not rely on it; the statements above are code-verified from the source in
this repository. The provenance and citation policy in
`docs/TFMXLegacy/PROVENANCE.md` governs how external material may be handled.

## Open design questions (pre-design)

These are open; none are resolved here:

- **Target contract:** The exact `File I/O → Model → Playback Engine` contract
  remains open, and the relationship of a possible Module Domain Model remains
  open.
- How the new engine executes macros: interpret the word stream as the legacy
  code does, or transform it into an internal representation first.
- How the unsupported and stub opcodes are handled in a future editor (SID
  commands `0x22`–`0x29`, random play/limit `0x1B`/`0x1E`, `0x30`).
- How old-style versus new-style flow, key-up waits, and DMA-synchronized
  waits survive a decoded representation, if one is chosen.
- How macro editing interacts with the timing model (VBI-based waits, eClocks)
  in the GUI.
- Whether the unresolved opcode-count discrepancy matters for new-engine
  modeling, or is purely a documentation artifact.

## Relationship to other documents

- `docs/TFMXLegacy/MACROS.md` — the code-verified macro semantics of the
  legacy implementation; the authoritative evidence base for the behavior
  summarized here.
- `docs/TFMXLegacy/PROVENANCE.md` — provenance and citation policy, and the
  discrepancy registry.
- `docs/ARCHITECTURE.md` — the concise current-system overview and architecture
  entrypoint.
- `docs/ASR.md` — historical ASR-002 evidence and the current Phase 4 policy.
- `docs/adr/ADR-001-new-engine-not-line-by-line-port.md` — the accepted
  new-engine approach informed by legacy ideas and semantics.
- `docs/GLOSSARY.md` — canonical terminology, including "macro (soundmacro)".
