# Malformed `unaligned_pattern_entry` layout

This self-authored pair derives from `step8` and keeps its header, trackstep,
macro table, and SMPL bytes. The first pattern-table cell at `0x224` points to
`0x251`, which is not four-byte aligned.

The independent pattern scan stops before accepting the entry and the loader
rejects the empty result. This is local structural-loader evidence, not a
universal TFMX validity claim.
