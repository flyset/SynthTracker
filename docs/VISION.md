# VISION

## Purpose

SynthTracker is one system containing one application container: a modern,
maintainable C product and repository. The current transitional product is one
legacy CLI application with SDL-backed audio. The future target is that same
system's GUI-first digital audio workstation (DAW) using SDL. TFMX denotes the
legacy format, modules, semantics, and temporary compatibility lineage.

## What it is

- A current transitional CLI that plays TFMX modules (including MasterBlazer,
  Turrican II/III, Z-Out, and others), with SDL-backed audio.
- A narrow internal and emerging playback seam being refactored out of the
  legacy C player; it is not yet a completed reusable playback core.
- A future modern C GUI-first DAW foundation. All SynthTracker-owned production and test
  source must use C23 or a later ISO C standard; C++ is not a project
  direction. Third-party dependency implementation languages remain separately
  evaluated.
- A future DAW target that continues the TFMX legacy format lineage; this does
  not make TFMX the product identity or establish a v1 compatibility promise.

## What it is not (non-goals)

- Not a general-purpose tracker: only the TFMX format is supported.
- Not an application that supports simultaneous independent playback contexts: TFMX has one active playback session at a time. Multiple TFMX channels or voices within that session remain supported.
- Not a sample editor or synthesizer.
- Not a cross-platform application: macOS is the current and only platform
  scope.
- Not a replacement for the legacy format: TFMX modules remain part of the
  legacy format lineage.

## Guiding principles

- During Phase 4, preserve current TFMX behavior where practical only as a
  temporary development scaffold. Every Phase 4 Track must assess compatibility
  impact and retain appropriate evidence. This is not a SynthTracker v1
  compatibility promise.
- The future GUI is a layer on top of a reusable playback core; the current
  implementation has only a narrow internal and emerging playback seam.
- TFMX remains the legacy format and semantic lineage; future format decisions
  remain subject to approved project direction.

## Open questions

- Q1) How the format is extended: which added features, how extended
  modules are saved, and how legacy players handle them. Unknown — the
  direction (extend, don't replace) is settled.

## Future direction

- Layered roadmap: establish the reusable playback core, then the GUI DAW (pattern editing,
  composing, mixing). Format extension comes after the DAW exists.
- Phase 3 is delivered. Phase 4 is next: component extraction.
- The intended macOS Audio Output Adapter is CoreAudio. Detailed GUI design
  and CoreAudio integration remain open.
- Detailed sequencing is deferred until the open questions resolve.
