# Glossary

Canonical product and protocol terminology. Definitions are code-verified
against the legacy implementation in this repository; the evidence-cited
reference is `TFMXLegacy/` (start at `TFMXLegacy/README.md`). Design questions
are marked **open** and are not defined here.

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
  `include/player.h`): active macro pointer/step/number, flow flags, sample
  addressing, and effect state.
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
