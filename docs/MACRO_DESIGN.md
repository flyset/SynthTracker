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
the local code actually does. Any future TFMX-owned macro implementation and
its tests remain C23; no C++ port is planned.

## Why macros matter

- **Compatibility floor.** The porting decision in `docs/ARCHITECTURE.md`
  makes "loads and plays existing TFMX modules correctly" the floor.
  Interpreter semantics — the meaning of macro opcodes and their control flow
  — are one of the four preserved semantic layers (loops, gosub, waits,
  key-up).
- **They carry the musical character.** Everything that makes an instrument
  sound like itself — sample selection and looping, envelopes, vibrato,
  portamento, volume splits, key-up behavior — is expressed in macros (see
  `docs/TFMXLegacy/MACROS.md`).
- **They are the inner level of the sequencer.** The legacy interpreter drives
  trackstep → pattern → macro (`docs/ARCHITECTURE.md`); per-channel state and
  per-tick effects hang off macro execution (`DoMacro`, `DoEffects`,
  `src/player.c`).
- **They are a future editing target.** The TUI DAW will compose and edit TFMX
  modules (per `docs/VISION.md`), so macros will be first-class objects in the
  product, not just opaque data the player walks.

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
- **State.** All of this lives in `struct Channel` (`include/player.h`): the
  macro pointer/step/number, flow flags, sample addressing, and effect state.
- **Known discrepancy.** The opcode count is recorded three ways (42-name
  debug table / 44 `case` labels / 47 as stated in `docs/ARCHITECTURE.md`);
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

- **Representation: raw versus decoded.** Whether macros remain arrays of
  32-bit words with the opcode in the high byte, or are decoded into a typed
  model at the loader seam, is explicitly **not decided** by this document. It
  is the same open seam question recorded in `docs/ARCHITECTURE.md` ("The
  exact loader-to-player seam contract").
- How the new engine executes macros: interpret the word stream as the legacy
  code does, or transform it into an internal representation first.
- How the unsupported and stub opcodes are handled in a future editor (SID
  commands `0x22`–`0x29`, random play/limit `0x1B`/`0x1E`, `0x30`).
- How old-style versus new-style flow, key-up waits, and DMA-synchronized
  waits survive a decoded representation, if one is chosen.
- How macro editing interacts with the timing model (VBI-based waits, eClocks)
  in the TUI.
- Whether the unresolved opcode-count discrepancy matters for new-engine
  modeling, or is purely a documentation artifact.

## Relationship to other documents

- `docs/TFMXLegacy/MACROS.md` — the code-verified macro semantics of the
  legacy implementation; the authoritative evidence base for the behavior
  summarized here.
- `docs/TFMXLegacy/PROVENANCE.md` — provenance and citation policy, and the
  discrepancy registry.
- `docs/ARCHITECTURE.md` — the porting approach and the four preserved
  semantic layers; this document feeds the interpreter-semantics layer.
- `docs/GLOSSARY.md` — canonical terminology, including "macro (soundmacro)".
