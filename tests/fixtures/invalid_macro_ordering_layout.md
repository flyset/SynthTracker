# Malformed `invalid_macro_ordering` layout

This wholly self-authored step8 derivative contains no copied module or sample
content. The leading macro setup words are malformed: MDAT bytes `0x260..0x267`
are `02 00 00 00 03 00 00 02` rather than the step8 bytes
`09 00 00 00 02 00 00 00`. The bytes therefore begin with sample-start and
sample-length setup instead of the authored pitch-first ordering; the binary
does not contain a swapped `09...02...` sequence. The paired SMPL remains the
authored two-byte step8 payload.

The intended local rejection is the ordered macro contract in `valid_mdat()` at
`src/playback/tfmx_loader.c`. This is repository-specific evidence **[inferred]**.
