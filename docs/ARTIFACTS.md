# Artifacts

This document catalogs artifacts that cross the approved target component
boundaries. Target artifacts are design vocabulary, not claims about the
current transitional CLI. This document does not define C APIs, concrete data
types, allocation or lifetime mechanics, threading, scheduling, or device
integration unless explicitly stated. [ADR-008](adr/ADR-008-audio-frame-block-mixed-value-boundary.md)
and [ASR-009](ASR.md#asr-009--audio-frame-block-boundary-invariants) are the
authority for the Audio Frame Block's mixed-value representation and validity
invariants. ADR-007 is the superseded historical record of the earlier fixed
PCM first format.

## Audio Frame Block

An **Audio Frame Block** is a finite, ordered block of zero or more rendered
audio frames. The `Mixer` emits it for consumption by the device-independent
`Audio Output` port or by `File I/O` for rendered-audio export; the exact
internal processing point inside a future `Mixer` remains open. Zero-frame
blocks are valid.

Each frame contains a signed-32 left mix value and a signed-32 right mix value
at one sample instant. The block carries raw mixed values, not serialized PCM
and not device-native data; it is not a synthesizer stream, a legacy TFMX
`Channel`, a legacy `struct Audio`, or a device-native buffer.

### Deferred boundary contract

The mixed-value representation and validity invariants are defined by ADR-008
and ASR-009. A future `Mixer` emits the same Audio Frame Blocks and may perform
additional internal mixing, effects, or processing before emitting them; the
precise internal processing point inside the `Mixer` is not fixed. Legacy
stereo blend, legacy low-pass filtering, and PCM byte packing are outside this
boundary and remain in the legacy renderer path, not in the block or in Audio
Output. Destination format adaptation is owned by Audio Output and its platform
adapters; File I/O serializes output for rendered-file export. The concrete C
API and types, numerical range/overflow/clip behavior, ownership, lifetime,
timing, ordering, queueing, backpressure, validation-result representation,
errors, adapter behavior, and implementation remain deferred.

### Responsibility

| Concern | Current target vocabulary |
| --- | --- |
| Producer | `Mixer` |
| Consumers | `Audio Output`; `File I/O` for rendered-audio export |
| Payload | An ordered sequence of frames, each with signed-32 left and right mix values emitted by the `Mixer`; the Mixer's internal creation point is not fixed |
| Descriptive metadata | Frame count and the left/right signed-32 mix-value representation; sample-rate and channel metadata remain deferred |
| Device ownership | None; SDL, CoreAudio, and other device handles are outside the artifact |

### Open contract questions

The following remain intentionally open:

- ownership, borrowing, copying, and lifetime across the consumer boundary;
- block timing, ordering, queueing, and backpressure;
- adapter capability and format-negotiation or conversion policy beyond the
  destination-adaptation ownership stated in ADR-008;
- validation-result and error representation beyond the invariants in ASR-009;
- the concrete C type or API used to carry the artifact.

These questions remain unresolved. The artifact name alone does not establish a
public C API or ABI.

### Target flow

```text
Tracker → timed musical events → Synthesizer → synthesizer streams
    → Mixer → Audio Frame Blocks → Audio Output → platform adapter
                              └→ File I/O for rendered-audio export
```

The `Synthesizer` stream and `Mix data` artifacts are related target vocabulary
but require their own definitions. The current legacy `player.c` and
`audio.c` translation units do not implement these target contracts.

### Compatibility boundary

During Phase 4, preserving current TFMX behavior where practical remains a
temporary development scaffold. The Audio Frame Block is target-architecture
vocabulary, not a claim about the current transitional CLI. In the current
CLI, the temporary legacy bridge cuts immediately after the legacy renderer
combines voices: its legacy stereo blending, low-pass filtering, and PCM
packing are intentionally absent from that temporary live route, remain in
the legacy renderer path, and are not moved into Audio Output. This
legacy-bridge compatibility behavior is distinct from the target `Mixer` rule
above, which requires a future `Mixer` to emit the same Audio Frame Blocks
without fixing its internal processing point. Defining the Audio Frame Block
does not change current SDL-era audio or file output behavior and does not
create a SynthTracker v1 compatibility promise.

## Artifact documentation rules

Each artifact entry should identify:

1. its producer and consumers;
2. its payload and descriptive metadata;
3. ownership, lifetime, timing, validation, and error expectations;
4. excluded responsibilities and compatibility impact; and
5. unresolved contract questions.
