# Architecture

## Status

This document is a living snapshot of the agreed architecture decisions and
of the current legacy implementation. Open and deferred items are listed at
the end and are not resolved here.

## Current legacy state

Observation: the existing code is roughly 2,674 lines of C99-era source in
three files, compiled as a single SDL-linked executable under the C23 baseline
(see `CMakeLists.txt`):

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

The approved Phase 1 target is a C23 baseline validated on macOS with Clang,
while preserving the SDL 1.2-era API surface currently used by the legacy
engine. macOS is the current and only platform scope; other-platform
compatibility and validation are outside the current project scope. Support
for another platform requires a new explicit roadmap decision. The current
CMake configuration uses C23 and the macOS/Clang baseline has been validated.

## Porting approach

Decision: build a new C engine rather than a line-by-line port of the legacy C.
The ideas and logic of the legacy interpreter carry over; the code shape does
not. All TFMX-owned production and test source remains C23, including the
future GUI/DAW; no C++ port is planned. Third-party dependency implementation
languages are evaluated separately.

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

## System decomposition (target)

The following responsibilities are future/proposed and are **not implemented**
as this decomposition. TFMX.cpp remains one system; the current transitional
legacy CLI and SDL-backed audio are the implemented shape.

- **Editor** — future GUI-first editing and composition experience.
- **Loader/Writer** — future TFMX-native loading and writing boundary.
- **Module Domain Model** — a shared editable/playable model used by the Editor,
  Loader/Writer, and Playback Engine.
- **Sequencing** — future trackstep/pattern scheduling, sharing control
  vocabulary with Synthesis but remaining a distinct responsibility.
- **Synthesis** — future macro/voice/effect interpretation, sharing that control
  vocabulary but remaining distinct from Sequencing.
- **Playback Engine** — future orchestration of sequencing and synthesis.
- **Audio Mixer** — future sample rendering, filtering, stereo blending, and
  format conversion.
- **Audio Output Port** — future device-independent output boundary.
- **CoreAudio Adapter** — intended future macOS implementation of the Audio
  Output Port; integration details remain open.

The current legacy files (`tfmx.c`, `player.c`, and `audio.c`) are the reference
implementation for these responsibilities, not a claim that the target seams
already exist.

## Test layout

Decision: automated tests use a component-first layout that mirrors the
planned engine decomposition:

- `tests/playback/` — playback-core behavior, beginning with
  `tests/playback/test_playback_context.c`.
- `tests/loader/` — module loading and representation behavior.
- `tests/mixer/` — rendering and audio-mixing behavior.
- `tests/editor/` — future editing behavior.
- `tests/gui/` — reserved future GUI behavior; the directory is not created yet.
- `tests/integration/` — cross-component and end-to-end behavior.
- `tests/fixtures/` — shared self-authored fixtures used by the component and
  integration tests.

The component-first layout is the agreed structure for test work. The initial
implemented paths are `tests/playback/test_playback_context.c` and the
self-authored fixtures under `tests/fixtures/`; the remaining component paths
are reserved for future coverage.

## GUI direction

The target interface is GUI-first and uses SDL. Detailed GUI design remains
open. The engine must remain UI-agnostic so the future Editor can be developed
independently; no GUI control or observability API is asserted here.

Phase 5 is **C23 product readiness**: a reusable C playback core and a C-based
GUI/DAW foundation.

## Open / deferred

These items are not resolved here:

- How per-song hacks are handled in the new engine (kept as data/flags versus
  starting clean).
- The loader → Module Domain Model → Playback Engine contract, including
  ownership and lifetime, validation responsibilities, and whether the model
  carries raw or decoded representations.
- The macro-layer design: how macros are modeled in the new engine (for
  example, arrays of 32-bit words with the opcode in the high byte, versus a
  decoded/typed representation). Deliberately left open in
  `docs/MACRO_DESIGN.md`.
- Detailed GUI design and SDL integration details.
- CoreAudio Adapter integration details and the exact Audio Output Port seam.
