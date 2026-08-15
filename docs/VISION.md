# VISION

## Purpose

TFMX.cpp is a modern, maintainable reimplementation of the legacy TFMX
music engine, originally built for SDL 1.1.7. It ports the original C
engine to C++ while keeping compatibility with existing TFMX modules
(MasterBlazer, Turrican II/III, Z-Out, and others), and builds a terminal
UI on top of it. The product is a TUI digital audio workstation (DAW):
a tracker-style environment for playing, composing, and editing TFMX
modules.

## What it is

- A terminal application that plays, composes, and edits TFMX modules.
- A modern C++ playback core refactored out of the legacy C player.
- A DAW whose native format is TFMX.

## What it is not (non-goals)

- Not a general-purpose tracker: only the TFMX format is supported.
- Not a sample editor or synthesizer.
- Not a GUI application: Qt, ImGui, and other windowed toolkits are out of
  scope; the interface is a TUI.
- Not a replacement for the legacy format: TFMX modules remain the native
  format.

## Guiding principles

- Legacy compatibility first: existing modules keep playing as the engine
  evolves.
- The engine is a reusable playback core; the TUI is a layer on top.
- TFMX is the native format; the project extends it over time rather than
  replacing it.

## Open questions

- Q1) Compatibility floor: bit-identical playback with the original, or
  "loads and plays existing modules correctly"? Unknown.
- Q2) How the format is extended: which added features, how extended
  modules are saved, and how legacy players handle them. Unknown — the
  direction (extend, don't replace) is settled.

## Future direction

- Layered roadmap: playback core, then the TUI DAW (pattern editing,
  composing, mixing). Format extension comes after the DAW exists.
- Detailed sequencing is deferred until the open questions resolve.
