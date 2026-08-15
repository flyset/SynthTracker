# Architecture

## Status

This document is a living snapshot of the agreed architecture decisions and
of the current legacy implementation. Open and deferred items are listed at
the end and are not resolved here.

## Current legacy state

Observation: the existing code is roughly 2,674 lines of C99 in three files,
compiled as a single SDL-linked executable (see `CMakeLists.txt`):

- `src/tfmx.c` — `main()`, argument parsing, file loading (`load_tfmx`), and
  byte-sniffing format detection (`tfmxtest()`).
- `src/player.c` — the interpreter: `RunMacro` (47 macro opcodes), `DoTrack`,
  `DoEffects`, `NotePort`, `tfmxIrqIn`. Sequencing runs trackstep → pattern →
  macro.
- `src/audio.c` — mixing with 14-bit fixed-point sample addressing, low-pass
  filter, stereo blend, ring buffer, SDL audio callback, and pthread
  synchronization.

State is global and shared by name across the files: `editbuf`, `smplbuf`,
`macros`, `patterns`, `audioData[8]`, `channelData[16]`, `trackManager`,
`patternBlockData`, `idb`.

Module data is mutated in place on load: `ntohl`/`ntohs` results are written
back into `editbuf`, and file pointers are converted to array indices.

Per-song behavior hacks exist as globals: `gemx`, `dangerFreakHack`,
`oopsUpHack`, `monkeyHack`.

The audio path is SDL-bound (`SDL_OpenAudio`, `SDL_MixAudio`) through the SDL
1.2-era API surface currently used by the engine. SDL 1.1.7 is historical
legacy context, not an asserted current build dependency.

## Phase 1 validation boundary

The approved Phase 1 target is a C23 baseline validated on macOS with Clang
and Linux with GCC, while preserving the SDL 1.2-era API surface currently
used by the legacy engine. The current CMake configuration still declares
C99; the C23 baseline is a validation target rather than a claim that the
configuration has already been updated or validated.

## Porting approach

Decision: build a new engine rather than a line-by-line port of the legacy C.
The ideas and logic of the legacy interpreter carry over; the code shape does
not.

Compatibility floor — resolves the direction of VISION.md open question Q1:
the engine must load and play existing TFMX modules correctly. Bit-identical
playback with the original is not a requirement.

## Preserved semantic layers

Decision: the "logic" that must be preserved spans four layers:

1. File format semantics — which files load, big-endian word layout,
   pointer/index interpretation.
2. Interpreter semantics — meaning of macro opcodes and pattern commands;
   control flow (loops, gosub, waits, key-up). Macro-layer design for the new
   engine is tracked in `docs/MACRO_DESIGN.md`, which is intentionally
   pre-design and leaves the open questions unresolved.
3. Timing model — VBI counting, eClocks, prescale/speed, tempo, multimode.
4. Audio model — period-to-frequency math, fixed-point mixing,
   envelope/vibrato/portamento, sample-loop handling.

## Top-level decomposition

Decision (agreed skeleton): three components now, with a fourth in the
future:

- Module loader — reads a TFMX module file into a typed, host-endian model.
  The boundary between raw bytes and the rest of the engine. Deferred detail:
  the exact shape of the loader output (for example, whether patterns/macros
  remain arrays of 32-bit words with the opcode in the high byte, versus
  deeper decoding).
- Player core — the interpreter: tracksteps → patterns → macros, per-channel
  state, effects, timing. Pure computation; no I/O.
- Mixer / output — takes per-voice state and produces samples; owns filter,
  stereo blend, and format conversion; can render to a device or to
  memory/file.
- TUI (future) — a fourth component, custom-built (see below).

Each box maps to a legacy file — `tfmx.c`, `player.c`, `audio.c` — which
serves as a reference implementation.

## TUI

Decision: the interface is a TUI (per VISION.md, Qt/ImGui and other windowed
toolkits are out of scope). The TUI will be custom-built as a first-class
product component — deliberately not an off-the-shelf TUI library
integration — with the intent of pushing what is possible in a terminal
(aesthetic identity, redefining terminal UI potential, following the
precedent of trackers like FastTracker II / Schism Tracker and tools like
opencode's OpenTUI).

Consequence: the engine must be UI-agnostic, exposing a defined control
surface (play/stop/seek/subsong/editing) and an observability surface
(position, active channels, per-voice state), so the TUI can be built
independently.

The detailed TUI architecture record is `docs/TUI.md`. Design work remains
deferred.

## Open / deferred

These items are not resolved here:

- How per-song hacks are handled in the new engine (kept as data/flags versus
  starting clean).
- The exact loader-to-player seam contract (the typed model shape).
- The macro-layer design: how macros are modeled in the new engine (for
  example, arrays of 32-bit words with the opcode in the high byte, versus a
  decoded/typed representation). Deliberately left open in
  `docs/MACRO_DESIGN.md`.
- The TUI substrate: full custom from raw terminal bytes versus custom UI on
  a thin terminal-I/O substrate (raw mode, escape parsing, resize events).
- The TUI itself: no design work yet.
