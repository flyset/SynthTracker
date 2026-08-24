# Malformed `unaligned_macro_entry` layout

This self-authored pair derives from `step8` and keeps its header, trackstep,
pattern table, and SMPL bytes. The first macro-table cell at `0x220` points to
`0x261`, which is not four-byte aligned.

The independent macro scan stops before accepting the entry and the loader
rejects the empty result. This is local structural-loader evidence, not a
universal TFMX validity claim.
