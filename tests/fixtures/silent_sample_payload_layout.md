# Malformed `silent_sample_payload` layout

This wholly self-authored pair keeps the valid step8 MDAT unchanged and changes
both SMPL bytes at offsets `0x000` and `0x001` from `40 C0` to `00 00`. No
external module or sample material is present.

The intended local rejection is the loader's silent-two-byte-payload predicate
after `valid_mdat()` succeeds in `tfmx_loader_read()`
(`src/playback/tfmx_loader.c`). This is a repository-specific fixture check
**[inferred]**, not a universal TFMX format claim.
