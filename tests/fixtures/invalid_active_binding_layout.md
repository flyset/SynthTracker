# Malformed `invalid_active_binding` layout

This is a wholly self-authored step8 derivative with no copied module or sample
content. MDAT offset `0x231` changes from `0x00` to `0x01`, changing the active
voice-0 binding word at `0x230` from `0x0000` to `0x0001`; the checked-in binary
records that exact change.
The SMPL pair remains the authored step8 payload.

The intended rejection is the first trackstep binding comparison in
`valid_mdat()` (`src/playback/tfmx_loader.c`, `read_be16(data + trackstart)`).
This is local implementation evidence **[inferred]**, not a universal TFMX claim.
