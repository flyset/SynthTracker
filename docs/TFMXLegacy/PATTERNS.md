# Pattern Data Semantics

Patterns are the per-channel event streams that the sequencer steps through
(see `PLAYER.md` for the sequencing model). This document describes the
pattern word encoding and the pattern command set as implemented in
`DoTrack` (`src/player.c:812`). Evidence: `src/player.c` (`DoTrack`,
`GetTrackStep`, `NotePort`), `src/tfmx.c` (`pattcmds[]`,
`dump_pattern`), `src/player.h` (`struct Pattern`,
`struct PatternBlock`).

## Word encoding

A pattern is an array of 32-bit words. Each word is read big-endian with
`ntohl` (`src/player.c:907`); the **high byte** (`b0`) is the event type and
the remaining bytes/words carry parameters (`b1`, `b2`, `b3`; `w1` = the low
16 bits). The pattern advances one word at a time via `PStep`.

## Note events (`b0 < 0xF0`)

A word whose high byte is below `0xF0` is a note-related event
(`src/player.c:935`–`962`):

- `b0 < 0x80` — plain note. The word is passed to `NotePort`, which
  interprets it as: note number = `b0` (with transpose applied), macro
  number = `b1`, velocity = high nibble of `b2`, finetune = `b3`
  (`src/player.c:66`–`85`).
- `0x80 <= b0 < 0xC0` — note with wait. `PWait = b3` and `b3` is zeroed
  (no finetune), so the pattern waits `b3` ticks before the note
  (`src/player.c:938`–`942`). [observed]
- `0xC0 <= b0 < 0xF0` — portamento note: after transposition, `b0` is
  masked to 6 bits and the `0xC0` bit is re-set; `NotePort` treats a
  `0xC0`–`0xEF` high byte as "slide to this note" (`src/player.c:945`–`948`,
  `src/player.c:86`–`93`).

Transpose: the note number is `((b0 + PXpose) & 0x3F)` — the channel's
transpose value (`PXpose`, set from the trackstep, see `PLAYER.md`) is added
and the result wraps to 6 bits ([observed], `src/player.c:943`).

## Pattern commands (`b0 >= 0xF0`)

For command words the interpreter switches on the **low nibble** of the high
byte (`t & 0xF`, `src/player.c:965`). The debug-name tables `pattcmds[]`
(`src/tfmx.c:406`, duplicated at `src/player.c:820`) list the same 16
commands; note the naming discrepancy for `0xFD` below.

| Command byte | Low nibble | Label (code) | Effect as implemented |
|--------------|------------|--------------|-----------------------|
| `0xF0` | 0 | End | End the pattern: `PNum=0xFF`; advance `CurrPos` (wrapping `LastPos` → `FirstPos`); call `GetTrackStep`; signal the track loop to restart (`return 1`) — `src/player.c:970`–`976`. |
| `0xF1` | 1 | Loop | Loop back to `PStep = w1` with count `b1`. See count semantics below — `src/player.c:977`–`990`. |
| `0xF2` | 2 | Cont | Continue in another pattern: `PAddr = patterns[b1]; PStep = w1` — `src/player.c:996`–`998`. |
| `0xF3` | 3 | Wait | `PWait = b1`; the pattern yields (`return 0`) — `src/player.c:1000`–`1003`. |
| `0xF4` | 4 | Stop | `PNum = 0xFF`; the pattern yields — `src/player.c:1007`–`1008`. |
| `0xF5` | 5 | Kup^ (key up) | Passed to `NotePort`, which sets `KeyUp = 0` — `src/player.c:1010`, `src/player.c:107`–`109`. |
| `0xF6` | 6 | Vibr | Passed to `NotePort` as a vibrato command — `src/player.c:1011`, `src/player.c:101`–`106`. |
| `0xF7` | 7 | Enve | Passed to `NotePort` as an envelope command — `src/player.c:1012`, `src/player.c:96`–`100`. |
| `0xF8` | 8 | GsPt | Save the current address/step (`PRoAddr`/`PRoStep`), then behave like `0xF2` (Cont) — `src/player.c:991`–`998`. |
| `0xF9` | 9 | RoPt | Return: restore `PAddr = PRoAddr; PStep = PRoStep` — `src/player.c:1017`–`1021`. |
| `0xFA` | 10 | Fade | `DoFade(b1, b3)` — a global master-volume fade — `src/player.c:1022`–`1025` (see `PLAYER.md`). |
| `0xFB` | 11 | PPat | Play pattern: set the pattern for another track, `t = b2 & 0x07`, from `b1` (pattern number) and `b3` (transpose); resets `PStep`, `PWait`, `PLoop` — `src/player.c:1030`–`1039`. |
| `0xFC` | 12 | Lock | Passed to `NotePort` as the SFX lock command — `src/player.c:1013`, `src/player.c:59`–`64`. |
| `0xFD` | 13 | Cue (code) | `idb.Cue[b1 & 0x03] = w1` — a cue/signal word — `src/player.c:1026`–`1029`. **[discrepancy]** Both `pattcmds[]` tables label index 13 "No entry", yet the interpreter implements `0xFD` as Cue. |
| `0xFE` | 14 | StCu | Stop custom pattern: `PlayPattFlag = 0`, then behaves like `0xF4` (Stop) — `src/player.c:1004`–`1008`. |
| `0xFF` | 15 | NOP | No operation — `src/player.c:967`–`969`. |

Pattern loop count semantics (`0xF1`), [observed]: `PLoop` is initialized to
`0xFFFF` when a pattern is loaded (`GetTrackStep`, `src/player.c:805`; and
in `PPat`). On the first encounter of `0xF1`, `PLoop = b1`, then `PLoop--`
and the pattern jumps to `w1`. Each later encounter decrements `PLoop`;
when it reaches 0 the loop exits (and `PLoop` is reset to `0xFFFF`). So the
section repeats `b1 + 1` times, and `b1 = 0` repeats forever ([inferred]
off-by-one as coded).

## Wait / step accounting

`DoTrack` is called once per pattern-tick (see `PLAYER.md` for the tick
derivation). Each call first checks `if (pattern->PWait--) return 0;`
(`src/player.c:899`) — the pattern yields while `PWait` is non-zero. A note
event with wait (`0x80`–`0xBF`) or an explicit `0xF3` sets `PWait`, so
waits are counted in ticks.

## Track-to-pattern binding

Eight pattern slots exist, one per track: `patternBlockData.p[0..7]`
(`src/player.h` `struct PatternBlock`). The trackstep table binds
tracks to patterns and transposes (`GetTrackStep`, `src/player.c:796`–`809`):
each 16-byte trackstep holds 8 `uint16` words, one per track; the high byte
is the pattern number (`PNum`), the low byte the transpose (`PXpose`). A
`PNum < 0x80` loads the pattern from `patterns[PNum]` and resets `PStep`,
`PWait`, `PLoop`. A `PNum == 0xFE` turns the channel off using the low byte
as the channel selector (`DoTrack`, `src/player.c:884`–`890`). A `PNum >=
0x90` is ignored (`return 0`), and `PNum` in `0x80`–`0x8F` keeps the
previously loaded pattern state [inferred from `GetTrackStep`].

## Debug display mapping

The debug/dump code displays note numbers with the tables
`n1 = "CCDDEFFGGAAB"`, `n2 = " # #  # # # "` and the formula
`zz = (note & 0x3F) + 6; name = n1[zz % 12] + n2[zz % 12]; octave = 48 + zz / 12`
(`src/player.c:815`, `src/player.c:951`–`954`; `dump_pattern` in
`src/tfmx.c:485`–`489`). With this mapping, note number 0 displays as
"F#0". The absolute pitch reference of the note numbers is **[unverified]**;
the code does not document it.
