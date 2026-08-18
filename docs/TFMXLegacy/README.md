# TFMX Legacy Reference

This directory is the self-contained reference for the original TFMX format
and player mechanics, as implemented by the legacy C player code in this
repository. It is written from direct reading of the local code; it is not
copied from, and contains no quotations from, any external documentation,
module, or sample. See `PROVENANCE.md` for the citation and copyright policy
before writing or quoting legacy-format content.

## Scope

This reference answers "how does the TFMX format work, and how does the
legacy player interpret it?", using the repository's own code as the evidence
base:

- `src/tfmx.c` — command-line front end, format sniffing, and module loading
  (`load_tfmx`, `tfmxtest`).
- `src/player.c` — the interpreter: `RunMacro`, `DoTrack`, `DoEffects`,
  `NotePort`, `tfmxIrqIn`.
- `src/audio.c` — mixing, filter, stereo blend, ring buffer, SDL audio path.
- `include/tfmx.h`, `include/tfmxsong.h`, `include/player.h`,
  `include/audio.h` — data structures and type definitions.
- `README_LEGACY` — historical notes from the original player authors.

Every factual statement in these documents is traceable to one of those
files. Where the code leaves a question open, the claim is explicitly marked
(see the status markers below). The documents describe the format as the
legacy player implements it; they do not assert that the legacy player's
behavior is the universal, authoritative definition of the TFMX format.

## The Chris Hülsbeck target distinction

TFMX is the Amiga module format created by Chris Hülsbeck and used for his
game soundtracks, including titles named in `README_LEGACY` (Turrican
II/III, R-Type, Z-Out, Gem'X, MasterBlazer, and others). This reference
targets that format: it documents the file layout and player mechanics that
Hülsbeck-style TFMX modules rely on. Three distinct things must not be
conflated:

1. **The TFMX format and its modules** — created by Chris Hülsbeck and other
   composers. The module files are their copyrighted works; this repository
   contains none of them, and these documents contain no module content.
   Attribution of the format to Hülsbeck is external knowledge; the local
   code itself never names him. [external]
2. **The legacy player** (`tfmx-play`) — an independent GPL implementation
   by Jonathan H. Pickard and David Banz (© 1996/2000), later maintained by
   Neochrome and others (`README_LEGACY`). The code in `src/` and `include/`
   descends from it.
3. **This repository's implementation** — a further independent port/refactor
   of that player under GPLv3 (`LICENSE`).

Nothing in these documents is a product of, endorsed by, or an official
publication of Chris Hülsbeck or any rights holder.

## Document map

| File | Content |
|------|---------|
| `FORMAT.md` | Module file layout: header, pointer tables, tracksteps, patterns, macros, samples; dual-file vs. single-file vs. DOS-extension variants; format detection strings. |
| `PATTERNS.md` | Pattern data semantics: note events, wait and portamento variants, the 16 pattern commands (`0xF0`–`0xFF`), transpose handling. |
| `MACROS.md` | Macro (instrument/soundmacro) data semantics: the macro opcode set, `NotePort` special codes, the note/period table, and the unresolved opcode-count discrepancy. |
| `PLAYER.md` | The interpreter and sequencer: trackstep table, `0xEFFE` control steps, timing model (`eClocks`, tempo, prescale), multimode, song start, per-song hacks, SFX lock. |
| `AUDIO.md` | The audio path: 14-bit fixed-point mixing, delta computation, sample looping, oversampling, stereo blend, low-pass filter, ring buffer, SDL integration. |
| `PROVENANCE.md` | Citation and copyright policy, evidence sources, and the registry of unresolved discrepancies. |

## Reading order

Start with `FORMAT.md` to understand the file layout, then `PATTERNS.md` and
`MACROS.md` for the two levels of event data, then `PLAYER.md` for how the
sequencer drives them, and `AUDIO.md` for how voices become samples. Read
`PROVENANCE.md` before quoting any legacy-format content elsewhere.

## Status markers

Claims are marked with a trailing tag where they are not directly observable
in the local code:

- **[observed]** — directly evidenced by the local code (the default; tags
  are omitted where the statement is plainly code-observable).
- **[inferred]** — a reasonable reading of the code, but the intended
  semantics are not spelled out anywhere locally.
- **[unverified]** — not confirmed against any authoritative external source.
- **[unsupported]** — acknowledged but not implemented in the local code
  (typically a `TODO` stub).
- **[external]** — outside knowledge not present in the local code.
- **[discrepancy]** — the local code (or this repo's other documents)
  disagrees with itself; see `PROVENANCE.md`'s discrepancy registry.

Line references in these documents refer to the snapshot of the repository
as read (August 2026) and are optional; symbol and path references are the
authoritative citations.

## Relationship to other repository documents

`docs/ARCHITECTURE.md` is the concise current-system overview and architecture
entrypoint. [`ADR-001`](../adr/ADR-001-new-engine-not-line-by-line-port.md)
records the new-engine approach, and [`ASR-002`](../ASR.md) records the
compatibility requirement. `docs/VISION.md` states product intent. This
reference is the detailed, evidence-cited counterpart to the current legacy
system overview. Where repository documents or local source disagree (for
example, the macro opcode count), the discrepancy is recorded in
`PROVENANCE.md` and marked here as **[discrepancy]**.

## Known limitations

- The documents describe only what the local legacy code does; they do not
  claim to be an authoritative TFMX specification.
- Some features exist only as `TODO` stubs (random play/limit, SID
  commands); they are marked **[unsupported]**.
- The single-file ("TFHD") loading path is incomplete in the local snapshot;
  see `FORMAT.md`.
- No module files are required to read these documents, and none are
  reproduced anywhere in the repository.
