# Malformed `empty_pattern_table` layout

This self-authored pair derives from `step8` and keeps its header, trackstep,
macro table, and SMPL bytes. The first pattern-table cell at `0x224` is zero,
so the independent pattern scan terminates without accepting an entry.

The loader must reject the pair because every table must yield at least one
readable, aligned target. This is local structural-loader evidence, not a
universal TFMX validity claim.
