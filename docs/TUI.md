# TUI

## Status and intent

Decision: the TUI is the future fourth component of the architecture (see
`docs/ARCHITECTURE.md`): a custom-built, tracker-style TUI DAW for playing,
composing, and editing TFMX modules, per `docs/VISION.md`. No design work
has been done yet; this document records only the decisions made so far.

## Terminal platform boundary

Decision: the TUI targets current terminal emulators on macOS, Linux, and
Windows. Capabilities specific to macOS or to iTerm2 are excluded from the
baseline; the TUI must not depend on them.

## Capability policy

Decision: the TUI declares the cross-platform terminal capabilities it
requires rather than probing the terminal at runtime to enable or disable
features. The required capability list is open; no final list is recorded
yet.

## Engine boundary

Decision: the TUI consumes the engine's UI-agnostic control surface
(play/stop/seek/subsong/editing) and observability surface (position,
active channels, per-voice state), per `docs/ARCHITECTURE.md`. The engine
adds no terminal knowledge or dependencies because of the TUI.

## Open questions

- The exact baseline: which terminal emulators are supported and what
  minimum capability set the TUI requires.
- The rendering model: full-frame redraw versus incremental updates.
- The input model: keyboard mapping and event handling.
- The terminal-I/O substrate: full custom handling of raw terminal bytes
  versus custom UI on a thin substrate (raw mode, escape parsing, resize
  events), per the open items in `docs/ARCHITECTURE.md`.
