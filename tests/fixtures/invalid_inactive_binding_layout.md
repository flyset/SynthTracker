# Malformed `invalid_inactive_binding` layout

This wholly self-authored pair derives from step8 and contains no external
module or sample material. MDAT byte `0x233` changes from `0x01` to `0x00`, so
the voice-1 inactive binding at `0x232` becomes `0xFE00` rather than `0xFE01`.
The SMPL file remains the authored two-byte step8 payload.

The intended local rejection is the distinct inactive-channel binding check in
`valid_mdat()` (`src/playback/tfmx_loader.c`). This bounded check is repository
framing **[inferred]**, not a universal format statement.
