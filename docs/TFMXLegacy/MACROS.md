# Macro (Instrument Soundmacro) Semantics

Macros are the instrument-level event streams: they control sample playback
addresses and lengths, volume, pitch effects, waits, loops, and sub-macro
calls. Each channel runs one macro at a time, selected by the note event's
macro number (`b1` of a pattern note; see `PATTERNS.md`). Evidence:
`src/player.c` (`RunMacro`, `DoEffects`, `DoMacro`, `NotePort`, `noteevals`),
`src/player.h` (`struct Channel`).

## Word encoding

Like patterns, macros are arrays of 32-bit words read big-endian with
`ntohl` (`src/player.c:141`). The high byte `b0` is the opcode; `b1`, `b2`,
`b3` are parameter bytes and `w1` is the low 16-bit word. `RunMacro`
(`src/player.c:134`) advances `MacroStep` through the macro at `MacroPtr`
and loops internally (`goto loop`) until an opcode yields (`return`) or
blocks.

## Old-style vs. new-style macro flow

[observed, `src/player.c`] `NewStyleMacro` distinguishes two flow modes:

- **Old-style** (`NewStyleMacro != 0`, the default after a note trigger):
  opcodes that use the `MAYBEWAIT` macro (`src/player.c:113`) `return` after
  one step, so the macro advances at most one opcode per tick until it hits
  a blocking `Wait` (`0x04`).
- **New-style** (`NewStyleMacro == 0`, set by the DMA-off opcode `0x13` with
  a non-zero flag): `MAYBEWAIT` instead resets `NewStyleMacro = 0xFF` and
  falls through, so non-blocking steps execute in the same tick. This is the
  sample-playback portion of a macro. A new note trigger resets
  `NewStyleMacro = 0xFF` (`NotePort`, `src/player.c:77`).

## Macro opcode table

The table below lists every opcode that has a `case` label in `RunMacro`
(`src/player.c:134`–`502`). Count: **44 case labels** [observed, verified by
direct count]. The debug-name table `macrocmds[]` (`src/tfmx.c:425`,
duplicated at `src/player.c:839`) has **42 entries** (opcodes `0x00`–`0x29`);
see the discrepancy note at the end of this document. Labels in the table
are the code comments; the `macrocmds[]` names differ for some entries and
are noted where useful.

| Op | Code label | Effect as implemented |
|----|-----------|-----------------------|
| `0x00` | dmaoff+reset | Resets `EnvReset`, `VibReset`, `PortaRate`, `AddBeginTime` to 0; with the GemX hack applies a velocity-dependent volume; **falls through** to `0x13` — `src/player.c:155`–`166`. |
| `0x01` | dma on | `EfxRun = b1`; `audio->mode = 1`; unless the macro is new-style (or the Danger Freak hack is on), reloads the sample from `SaveAddr`/`SaveLen` and sets `mode |= 2` — `src/player.c:188`–`206`. |
| `0x02` | setbegin | `SaveAddr = CurAddr = x.l` (full word = sample address) — `src/player.c:207`–`211`. |
| `0x03` | setlen | `SaveLen = CurrLength = w1` — `src/player.c:222`–`225`. |
| `0x04` | wait | Blocks for `w1` ticks (see `ReallyWait` note) — `src/player.c:235`–`249`. |
| `0x05` | loop | Loops back to `MacroStep = w1`, count `b1` (`b1 + 1` iterations; `b1 = 0` infinite) — `src/player.c:286`–`293`. |
| `0x06` | cont | Continue in another macro: `MacroPtr = macros[b1]; MacroStep = w1; Loop = 0xFFFF` — `src/player.c:402`–`407`. |
| `0x07` | stop | `MacroRun = 0; return` — `src/player.c:294`–`297`. |
| `0x08` | addnote | Pitch effect from the current note: `SetNote` with `a = CurrNote` — `src/player.c:335`–`337`. |
| `0x09` | setnote | Pitch effect from a base of 0: `SetNote` with `a = 0` — `src/player.c:339`–`341`. |
| `0x0A` | reset | Resets `EnvReset`, `VibReset`, `PortaRate`, `AddBeginTime` to 0 — `src/player.c:379`–`383`. |
| `0x0B` | portamento | `PortaReset = b1; PortaTime = 1`; seeds `PortaPer = DestPeriod` if no rate is active; `PortaRate = w1` (full word) — `src/player.c:355`–`361`. |
| `0x0C` | vibrato | `VibTime = (VibReset = b1) >> 1; VibWidth = b3; VibFlag = 1`; if no portamento active, resets `CurPeriod = DestPeriod; VibOffset = 0` — `src/player.c:362`–`372`. |
| `0x0D` | addvolume | `CurVol = min(0x40, Velocity*3 + b3)`; a `b2 == 0xFE` marker prints an unsupported warning — `src/player.c:298`–`315`. |
| `0x0E` | setvolume | `CurVol = b3`; `b2 == 0xFE` marker prints an unsupported warning — `src/player.c:316`–`324`. |
| `0x0F` | envelope | `EnvReset = EnvTime = b2; EnvEndvol = b3; EnvRate = b1` — `src/player.c:373`–`378`. |
| `0x10` | loop key up | If `KeyUp` is set (no key-up yet), behaves like the normal loop `0x05`; if a key-up is pending (`KeyUp == 0`) the loop is skipped (`break`) — `src/player.c:282`–`285`. |
| `0x11` | addbegin | `AddBeginTime = AddBeginReset = b1; AddBegin = (S16)w1; SaveAddr = CurAddr += AddBegin` — `src/player.c:212`–`221`. |
| `0x12` | addlen | `CurrLength += w1; SaveLen = CurrLength` — `src/player.c:226`–`234`. |
| `0x13` | dmaoff | `audio->loop = LoopOff`; with `b1 == 0`: `audio->mode = 0` (plus `slen = 0` for new-style macros); with `b1 != 0`: `mode |= 4`, `NewStyleMacro = 0`, `return` — `src/player.c:167`–`187`. |
| `0x14` | wait key up | While the key is held (`KeyUp` set), counts down a loop counter seeded from `b3` and stays on this opcode one tick at a time; when a key-up occurs (`KeyUp == 0`) or the count expires, it proceeds — `src/player.c:384`–`397`. |
| `0x15` | go sub | Saves `ReturnPtr`/`ReturnStep`; **falls through** to `0x06` — `src/player.c:398`–`401`. |
| `0x16` | return sub | Restores `MacroPtr`/`MacroStep` from `ReturnPtr`/`ReturnStep` — `src/player.c:408`–`412`. |
| `0x17` | setperiod | `DestPeriod = w1` (and `CurPeriod` too if no portamento active) — `src/player.c:350`–`354`. |
| `0x18` | sampleloop | `SaveAddr += (w1 & 0xFFFE); SaveLen -= w1 >> 1`; re-derives `CurrLength`/`CurAddr` — `src/player.c:413`–`419`. |
| `0x19` | oneshot | `SaveAddr = CurAddr = 0; SaveLen = CurrLength = 1` — `src/player.c:420`–`425`. |
| `0x1A` | wait on DMA | `audio->loop = LoopOn; audio->channel = channel; WaitDMACount = w1; MacroRun = 0`; yields via `MAYBEWAIT` — `src/player.c:250`–`257`. |
| `0x1B` | random play | **[unsupported]** Prints a TODO message and continues. The code comment notes it is needed for Master Blazer Ingame — `src/player.c:272`–`276`. |
| `0x1C` | note split | If `CurrNote > b1`, jump to `MacroStep = w1` — `src/player.c:258`–`262`. |
| `0x1D` | vol split | If `CurVol > b1`, jump to `MacroStep = w1` — `src/player.c:263`–`267`. |
| `0x1E` | random limit | **[unsupported]** Prints a TODO message and continues — `src/player.c:277`–`281`. |
| `0x1F` | set prev note | Pitch effect from `PrevNote`: `SetNote` with `a = PrevNote` — `src/player.c:331`–`334`. |
| `0x20` | cue | `idb.Cue[b1 & 0x03] = w1` (labeled "Signal" in `macrocmds[]`) — `src/player.c:426`–`429`. |
| `0x21` | start macro | Re-triggers a note through `NotePort` using `CurrNote` and the current velocity — `src/player.c:325`–`330`. |
| `0x22` | SID setbeg | **[partial]** Prints a TODO message; also sets `AddBeginTime = 0; CurAddr = x.l` (the code comment says "seems to work similar to 02") — `src/player.c:445`–`452`. |
| `0x23` | SID setlen | **[unsupported]** Prints a TODO message only — `src/player.c:453`–`457`. |
| `0x24` | SID op3 ofs | **[unsupported]** Prints a TODO message only — `src/player.c:458`–`462`. |
| `0x25` | SID op3 frq | **[unsupported]** Prints a TODO message only — `src/player.c:463`–`467`. |
| `0x26` | SID op2 ofs | **[unsupported]** Prints a TODO message only — `src/player.c:468`–`472`. |
| `0x27` | SID op2 frq | **[unsupported]** Prints a TODO message only — `src/player.c:473`–`477`. |
| `0x28` | SID op1 | **[unsupported]** Prints a TODO message only — `src/player.c:478`–`482`. |
| `0x29` | SID stop | **[unsupported]** Prints a TODO message only — `src/player.c:483`–`487`. |
| `0x30` | (unknown) | **[unsupported]** Prints a TODO message only — `src/player.c:488`–`492`. |
| `0x31` | (Turrican 3 title) | Ignored (no-op); the code comment says "we can safely ignore" — `src/player.c:493`–`494`. |

Any opcode outside the table prints the `NOTSUPPORTED` diagnostic
(`src/player.c:8`, default case at `src/player.c:495`) and continues.

### `SetNote` (shared by `0x08`, `0x09`, `0x1F`)

The pitch computation ([observed], `src/player.c:342`–`349`):

```
period = (notevals[(a + b1) & 0x3F] * (0x100 + Finetune + (S8)b3)) >> 8
DestPeriod = period; if (!PortaRate) CurPeriod = period;
```

where `a` is `CurrNote` (`0x08`), `0` (`0x09`), or `PrevNote` (`0x1F`).
`b1` is a semitone offset, `b3` a signed detune added to the 8-bit finetune
base. The result is masked to 6 bits, matching the 64-entry period table.

## `NotePort` special codes

`NotePort` (`src/player.c:52`) is the shared entry point for notes, both
from pattern note events and from the macro opcodes `0x21` (and the pattern
command words `0xF5`–`0xF7`, `0xFC`). Depending on the high byte:

| High byte | Meaning | Effect |
|-----------|---------|--------|
| `< 0xC0` | Note | Starts a macro: `MacroPtr = macros[MacroNum = b1]`; `CurrNote = b0`; `Velocity = (b2 >> 4) & 0xF`; `Finetune = b3` (forced to 0 under the Danger Freak hack); resets `MacroStep`, `EfxRun`, `MacroWait`, `KeyUp = 1`, `Loop = -1`, `MacroRun = -1`, `NewStyleMacro = 0xFF`, `ReallyWait = 1` — `src/player.c:66`–`85`. Ignored if the SFX flag is set (`src/player.c:65`). |
| `0xC0`–`0xEF` | Portamento | `PortaReset = b1; PortaTime = 1; DestPeriod = noteevals[CurrNote = (b0 & 0x3F)]; PortaRate = b3` (seeding `PortaPer` if no rate active) — `src/player.c:86`–`93`. |
| `0xF5` | key up | `KeyUp = 0` — `src/player.c:107`–`109`. |
| `0xF6` | vibrato | `VibTime = (VibReset = (b1 & 0xFE)) >> 1; VibWidth = b3; VibFlag = 1; VibOffset = 0` — `src/player.c:101`–`106`. |
| `0xF7` | envelope | `EnvRate = b1; EnvReset = EnvTime = (b2 >> 4) + 1; EnvEndvol = b3` — `src/player.c:96`–`100`. |
| `0xFC` | SFX lock | `SfxFlag = b1; SfxLockTime = b3` — `src/player.c:59`–`64`. |
| other `0xF0`–`0xFF` | — | No action (no default case in the switch) — `src/player.c:94`–`110`. |

## The note/period table

`notevals[]` (`src/player.c:11`) has **64 entries** [observed, verified by
direct count], indexed with 6-bit masking (`& 0x3F`). The values are
descending semitone periods: index 0 = `0x6AE` (1710), descending to
`0x071` (113) at index 59; the final indices wrap back up (e.g. index 60 =
`0x0D6` = 214). The table therefore spans roughly five octaves of descending
periods with repeated high-octave tail values [inferred layout]. The absolute
pitch reference of the period values is **[unverified]**.

## Effects (per-tick, `DoEffects`)

`DoEffects` (`src/player.c:504`) is applied to each channel every tick
(after macro stepping, `src/player.c:653`):

- **AddBegin** (`0x11`): while `AddBeginTime` is non-zero, `CurAddr`/`SaveAddr`
  advance by `AddBegin` each tick; when the time expires the delta sign
  flips and the count resets — `src/player.c:514`–`527`.
- **Vibrato** (`0x0C`, `0xF6`): `VibOffset += VibWidth`; the played period is
  `(DestPeriod * (0x800 + VibOffset)) >> 11`; `VibWidth` negates every
  `VibTime` ticks — `src/player.c:534`–`544`.
- **Portamento** (`0x0B`, pattern porta): `PortaPer` slides toward
  `DestPeriod` by `PortaRate` every `PortaReset` ticks, with
  `(PortaPer*(256±PortaRate))>>8` steps; the rate clears when the target is
  reached — `src/player.c:545`–`564`.
- **Envelope** (`0x0F`, `0xF7`): every `EnvReset` ticks, `CurVol` moves by
  `EnvRate` toward `EnvEndvol`; the envelope ends (and `CurVol` snaps to the
  target) when it crosses — `src/player.c:565`–`586`.
- **Fade** (global, from pattern `0xFA` or trackstep type 4): `MasterVol`
  moves toward `FadeDest` by `FadeSlope` every `FadeReset` ticks —
  `src/player.c:592`–`597`.

## `ReallyWait` behavior in `Wait` (`0x04`)

The `Wait` opcode with bit 0 of `b1` set checks `channel->ReallyWait++`
(`src/player.c:237`–`241`). `ReallyWait` is set to 1 by note events
(`NotePort`, `src/player.c:76`) and is never reset elsewhere in the
interpreter, so after a note has been triggered, such a wait returns
immediately instead of blocking. The intended semantics of this flag are
**[inferred]** from the code.

## Macro count discrepancy (unresolved)

Three different numbers appear for "how many macro opcodes exist":

- `macrocmds[]` — the debug-name table — has **42 entries** (opcodes
  `0x00`–`0x29`), both at `src/tfmx.c:425` and `src/player.c:839`.
- `RunMacro` — the interpreter — has **44 case labels** by direct count
  (`src/player.c:134`–`502`), covering opcodes `0x00`–`0x31` (minus none;
  including the `0x30`/`0x31` cases beyond the name table).
- A historical repository-documentation statement records "`RunMacro` (47
  macro opcodes)" [discrepancy]; this is not a source-derived count or a
  settled canonical value.

Which count is canonical is **unresolved** — see `PROVENANCE.md`. The
numbers are recorded here as facts about the current snapshot; no resolution
is asserted.

## Debug dumper caveat

The interactive debug command `pm` (`do_debug`, `src/tfmx.c:557`–`573`)
prints each macro opcode by indexing `macrocmds[opcode]` directly. Because
the name table has only 42 entries (`0x00`–`0x29`), a macro containing an
opcode of `0x2A` or above indexes out of bounds [observed, risk of undefined
behavior]. This is a diagnostic tool, not part of playback.

## Relationship to the reimplementation

This reference describes the legacy implementation only, from direct reading
of the local code, and contains no external material. Design work for the
macro layer of the new engine is tracked in `../MACRO_DESIGN.md`, which is
intentionally pre-design and leaves its open questions unresolved. Nothing in
this document decides how the new engine represents or executes macros.
