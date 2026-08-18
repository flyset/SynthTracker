# Provenance, Citation, and Copyright Policy

This file is the policy for how the `docs/TFMXLegacy/` reference may be
written, quoted, and extended. It must be read before writing or quoting
legacy-format content, per the repository's root `AGENTS.md`.

## What this reference is

`docs/TFMXLegacy/` is a **self-contained, evidence-cited** description of the
original TFMX format and player mechanics, written entirely from direct
reading of the code in this repository. It contains:

- no external URLs, downloads, or web references;
- no quotations from external documentation (including the unofficial TFMX
  notes by JHP that are only *mentioned by name* in code comments such as
  `src/player.c:433`);
- no module files, sample data, or excerpts of any song;
- no claims presented as authoritative beyond what the local code shows.

All prose is original factual summary or paraphrase of the local code.

## Evidence sources

| Path | What it evidences |
|------|-------------------|
| `src/tfmx.c` | Front end, format sniffing (`tfmxtest`), loader (`load_tfmx`), debug dumps, option handling. |
| `src/player.c` | Interpreter: `RunMacro`, `DoTrack`, `DoEffects`, `NotePort`, `DoMacro`, `DoTracks`, `GetTrackStep`, `StartSong`, `tfmxIrqIn`. |
| `src/audio.c` | Mixing, filter, blend, ring buffer, SDL audio path, tick-to-sample conversion. |
| `include/tfmx.h`, `include/tfmxsong.h`, `include/player.h`, `include/audio.h` | Types and structures referenced throughout. |
| `README_LEGACY` | Historical notes, credits, and tested-module list of the original player. |
| `README.md`, `docs/ARCHITECTURE.md`, `docs/VISION.md` | Repository-level context; cross-checked for consistency. |

Citations in the reference name file paths and symbols (for example,
`RunMacro`, `src/player.c:134`). Line numbers refer to the snapshot as read
(August 2026) and are optional; symbol and path references are the
authoritative citations.

## Ownership and licensing

- The legacy player `tfmx-play` is "(C) 1996/2000 by Jonathan H. Pickard
  and David Banz", released under the GPL, per `README_LEGACY`
  ("COPYRIGHT" section). Later maintainers are credited in `README_LEGACY`.
- This repository is released under GPLv3 (`LICENSE`), and its license file
  carries a legal notice stating that the project is an independent
  implementation of the TFMX format and does not include or redistribute the
  original TFMX software or copyrighted modules.
- The reference documents summarize behavior of GPL code located in this
  repository. Summaries and citations of local code are permitted; the
  reference must never reproduce large verbatim portions of any file or
  quote third-party material.

## The Chris Hülsbeck target distinction

TFMX is the Amiga module format created by Chris Hülsbeck and used for his
game soundtracks (Turrican II/III, R-Type, Z-Out, Gem'X, MasterBlazer, and
others named in `README_LEGACY`). The distinction the reference maintains:

1. **Format and modules** — Hülsbeck's creation; module files are the
   composers' copyrighted works. The repository contains none, and the
   reference describes only format mechanics, never module content.
2. **Legacy player** — an independent GPL implementation (Pickard/Banz and
   maintainers).
3. **This repository's implementation** — an independent port/refactor under
   GPLv3.

Nothing in `docs/TFMXLegacy/` is presented as a product of, endorsement by,
or an official publication of Chris Hülsbeck or any rights holder.
Hülsbeck's authorship is external knowledge: the local code never names him,
so statements about him carry the **[external]** marker (see
`docs/TFMXLegacy/README.md`'s status markers).

## Citation rules for this reference

1. Cite local paths and symbols; use line numbers only when accurate for the
   current snapshot.
2. Do not add external URLs or link out; the reference must stay
   self-contained.
3. Do not quote, transcribe, or embed any external copyrighted source —
   including documentation, forums, or manuals about TFMX.
4. Do not include module or sample content of any kind.
5. Distinguish implementation behavior (this codebase) from format claims;
   never assert that the legacy player's behavior is universal original-
   format behavior.
6. Mark every claim that is not directly code-observable with the tags
   defined in `docs/TFMXLegacy/README.md`: **[inferred]**, **[unverified]**,
   **[unsupported]**, **[external]**, **[discrepancy]**.
7. Record new contradictions in the discrepancy registry below.

## Discrepancy registry (unresolved)

Each entry records a place where the repository's code or documents disagree
with themselves. None are resolved here; they are marked **[discrepancy]** at
their occurrence sites.

1. **Macro opcode count (42 / 44 / 47).**
   - `macrocmds[]` (the debug-name table) has 42 entries — `src/tfmx.c:425`,
     `src/player.c:839`.
   - `RunMacro` has 44 `case` labels by direct count — `src/player.c:134`–`502`.
   - A historical repository-documentation statement records "RunMacro (47
     macro opcodes)" [discrepancy]; this is not a source-derived count.
   Which number is canonical is unresolved; see `MACROS.md`.
2. **`0xFD` pattern command label vs. behavior.** Both `pattcmds[]` tables
   label index 13 "No entry", but the interpreter implements `0xFD` as the
   Cue command (`idb.Cue[b1 & 3] = w1`, `src/player.c:1026`–`1029`).
   See `PATTERNS.md`.
3. **SDL version and build identification.** `README.md` and
   `docs/ARCHITECTURE.md` identify SDL 1.1.7 as historical context; the code
   uses the SDL 1.2-era API (`SDL_OpenAudio`, `SDL_MixAudio`). `CMakeLists.txt`
   labels the setup SDL2 yet uses an unversioned `sdl-config` and has no
   explicit SDL-version verification. The provider/version relationship
   remains unresolved. See `AUDIO.md`.
4. **Single-file (TFHD) loading.** The loader has single-file paths
   (`src/tfmx.c:248`–`251`, `352`–`372`) but the TFHD offsets
   (`nTFhd_offset`, `nTFhd_mdatsize`, `nTFhd_smplsize`) are never assigned;
   the parsing function is commented out (`src/tfmx.c:21`, `760`). See
   `FORMAT.md`.
5. **Inert hack flags.** `oopsUpHack`, `monkeyHack`, and `weirdZoutThm`
   are declared and referenced but never set in this snapshot; the MD5-based
   auto-detection that would set them is commented out
   (`src/tfmx.c:28`–`38`). See `PLAYER.md`.
6. **Unused format detector.** `tfmxtest()` (`src/tfmx.c:94`) is never
   called; its variant labels ("TFMX Pro", "TFMX1.5", "TFMX7V", "TFHD1.5",
   "TFHDPro", "TFHD7V") describe sniffing logic only. See `FORMAT.md`.
7. **Pitch reference.** The note/period table (`notevals[]`,
   `src/player.c:11`) and the debug display mapping (note 0 shown as "F#0")
   have no documented absolute pitch reference — **[unverified]**. See
   `MACROS.md` / `PATTERNS.md`.
8. **Missing include.** `src/player.c` includes `"machine/endian.h"`, which
   is not present in the repository tree or in git history ([observed]
   search). This is an implementation-state observation relevant to anyone
   building the snapshot; it is not addressed in the reference documents
   beyond this registry.

## Updating the reference

When extending these documents:

- Follow `AGENTS.md` (root) and `docs/AGENT_WORKFLOW.md`; documentation changes
  are subject to the same verification expectations.
- Re-validate every opcode table, variant description, and numeric claim
  against the current local code before finalizing.
- Add new contradictions to the discrepancy registry rather than silently
  choosing a side.
- Never broaden scope into implementation, tests, configuration, Tracks, or
  durable memory changes.
