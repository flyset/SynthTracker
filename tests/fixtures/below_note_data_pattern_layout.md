# Malformed `below_note_data_pattern` layout

This self-authored pair derives from `step8` and keeps its header, trackstep,
macro table, and SMPL bytes. The first pattern-table cell at `0x224` points to
`0x1FC`, below the loader's minimum raw target offset `0x200`.

The independent pattern scan stops before accepting the entry and the loader
rejects the empty result. This is local structural-loader evidence, not a
universal TFMX validity claim.
