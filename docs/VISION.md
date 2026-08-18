# VISION

## Purpose

TFMX.cpp is one system containing one application container: a modern,
maintainable reimplementation of the legacy TFMX music engine, originally built
for SDL 1.1.7. The current transitional product is one legacy CLI application
with SDL-backed audio. The future target is that same system's GUI-first TFMX
digital audio workstation (DAW) using SDL, while keeping compatibility with
existing TFMX modules (MasterBlazer, Turrican II/III, Z-Out, and others).

## What it is

- A current transitional CLI that plays TFMX modules, with SDL-backed audio.
- A narrow internal and emerging playback seam being refactored out of the
  legacy C player; it is not yet a completed reusable playback core.
- A future modern C GUI-first DAW foundation. All TFMX-owned production and test
  source must use C23 or a later ISO C standard; C++ is not a project
  direction. Third-party dependency implementation languages remain separately
  evaluated.
- A future DAW target whose native format is TFMX.

## What it is not (non-goals)

- Not a general-purpose tracker: only the TFMX format is supported.
- Not an application that supports simultaneous independent playback contexts: TFMX has one active playback session at a time. Multiple TFMX channels or voices within that session remain supported.
- Not a sample editor or synthesizer.
- Not a cross-platform application: macOS is the current and only platform
  scope.
- Not a replacement for the legacy format: TFMX modules remain the native
  format.

## Guiding principles

- Legacy compatibility first: existing modules keep playing as the engine
  evolves.
- Existing TFMX modules must load and play with correct musical behavior;
  bit-identical rendered audio is not required.
- The future GUI is a layer on top of a reusable playback core; the current
  implementation has only a narrow internal and emerging playback seam.
- TFMX is the native format; the project extends it over time rather than
  replacing it.

## Open questions

- Q1) How the format is extended: which added features, how extended
  modules are saved, and how legacy players handle them. Unknown — the
  direction (extend, don't replace) is settled.

## Future direction

- Layered roadmap: establish the reusable playback core, then the GUI DAW (pattern editing,
  composing, mixing). Format extension comes after the DAW exists.
- Phase 5 is **Modern C product readiness**, targeting a reusable C playback
  core and a modern C GUI/DAW foundation.
- The intended macOS Audio Output Adapter is CoreAudio. Detailed GUI design
  and CoreAudio integration remain open.
- Detailed sequencing is deferred until the open questions resolve.
