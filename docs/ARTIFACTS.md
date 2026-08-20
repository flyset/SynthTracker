# Artifacts

This document catalogs artifacts that cross the approved target component
boundaries. Target artifacts are design vocabulary, not claims about the
current transitional CLI. This document does not define C APIs, concrete data
types, allocation or lifetime mechanics, threading, scheduling, or device
integration unless explicitly stated. [ADR-007](adr/ADR-007-audio-frame-block-boundary-and-fixed-first-format.md)
and [ASR-009](ASR.md#asr-009--audio-frame-block-boundary-invariants) are the
authority for the Audio Frame Block's fixed first format and validity
invariants.

## Audio Frame Block

An **Audio Frame Block** is a finite, ordered block of rendered audio frames.
The `Mixer` produces it for consumption by the device-independent `Audio
Output` port or by `File I/O` for rendered-audio export.

One frame represents the samples for all output channels at one sample instant.
The block is the data artifact at the boundary between rendering and output;
it is not a synthesizer stream, a legacy TFMX `Channel`, a legacy `struct
Audio`, or a device-native buffer.

### Deferred boundary contract

The fixed format and validity invariants are defined by ADR-007 and ASR-009.
Format negotiation or conversion beyond the first format, ownership, lifetime,
timing, ordering, queueing, backpressure, validation-result representation,
errors, the concrete C API and types, adapter behavior, and implementation
remain deferred.

### Responsibility

| Concern | Current target vocabulary |
| --- | --- |
| Producer | `Mixer` |
| Consumers | `Audio Output`; `File I/O` for rendered-audio export |
| Payload | An ordered sequence of rendered sample values |
| Descriptive metadata | Frame count, sample rate, channel layout/count, and sample representation |
| Device ownership | None; SDL, CoreAudio, and other device handles are outside the artifact |

### Open contract questions

The following remain intentionally open:

- ownership, borrowing, copying, and lifetime across the consumer boundary;
- block timing, ordering, queueing, and backpressure;
- format negotiation or conversion beyond the fixed first format;
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
temporary development scaffold. Defining the Audio Frame Block does not change
current SDL-era stereo blending, low-pass filtering, PCM conversion, or file
output behavior, and does not create a SynthTracker v1 compatibility promise.

## Artifact documentation rules

Each artifact entry should identify:

1. its producer and consumers;
2. its payload and descriptive metadata;
3. ownership, lifetime, timing, validation, and error expectations;
4. excluded responsibilities and compatibility impact; and
5. unresolved contract questions.
