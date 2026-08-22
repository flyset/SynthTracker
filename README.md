# SynthTracker

SynthTracker is the product and repository identity. It is a modern C product
with a transitional legacy CLI and a future GUI-first DAW. TFMX denotes the
legacy format, modules, semantics, and temporary compatibility lineage.

## Overview
SynthTracker is one system with a current transitional legacy CLI plus SDL-backed
audio. It aims to **refactor and reimplement** the legacy **TFMX player** in
modern C, with a future GUI-first DAW using SDL. All SynthTracker-owned production
and test source, including the future GUI/DAW, must use C23 or a later ISO C
standard; C++ is not a project direction.
Third-party dependency implementation languages are evaluated separately.
During Phase 4, preserve current TFMX behavior where practical as a
temporary development scaffold; every Phase 4 Track must assess compatibility
impact and retain appropriate evidence. This is not a SynthTracker v1
compatibility promise. The product also aims to improve performance,
maintainability, and user experience.

### Current Status
- **Refactoring progress**: Initial refactoring and setup for the modern C product
  boundary.
- **UI Integration**: Planning and design for a future GUI-first DAW; no GUI or
  editing functionality is implemented yet.
- **Legacy SDL-era audio features**: The SDL 1.2-era renderer still includes
  stereo blending and low-pass filtering. Both remain in the legacy `-o` file
  output path and are intentionally absent from the temporary compatible live
  route.
- **Platform scope**: macOS only. Linux and other platform support are outside the current project scope; adding a platform requires a new explicit roadmap decision.
- **Phase status**: Phase 3 is delivered; Phase 4 is in progress.
- **Temporary compatible live output (Phase 4)**: The temporary live route is a
  silent, device-free private submission immediately after the legacy mix. On
  macOS, a private CoreAudio adapter receives each raw signed-32 block through
  the private audio-output dispatch and converts it to interleaved Float32 with
  no device involvement; on non-macOS the private null submission remains.
  The route accepts exactly 44.1 kHz with `-b 1` or `-b 2` and rejects every
  other `-b`, `-8`, `-w`, or rate setting before playback. Legacy `-o` remains
  on the full legacy file path and is exempt from this strict profile.

## Features
- Plays **most TFMX modules**, including:
  - MasterBlazer, Apidya, Turrican II/III, JimPower
  - MUDS, R-Type Theme, Z-Out, The Oath, and more
- **Legacy `-o` file output** — retains stereo blending (adjustable for
  headphone users) and low-pass filtering (high, medium, low cutoff
  frequencies), which the temporary compatible live route intentionally omits.
- **macOS support** (the current platform scope)
- **Private CoreAudio adapter (macOS only)** — receives raw signed-32 audio
  blocks through the private audio-output dispatch and converts them to
  adapter-private interleaved Float32. It has no device or CoreAudio framework
  lifecycle, callback, buffering, clock, scheduling, or audible output (those
  remain Track 015), and it is not a public API or the future Audio Output
  Adapter.

## Known Issues
- Some TFMX files may not play correctly.
- A specific version of the **Z-Out theme** causes a segfault on macOS. Historically, this was reported as fixed in Linux; Linux is outside the current project scope.
- Performance is slightly lower than the legacy OSS implementation but acceptable on modern hardware.

## Build Instructions
### Dependencies
- **Phase 1 validation baseline**: C23 on macOS with Clang, against the SDL 1.2-era API surface currently used by the legacy engine. Other-platform validation is outside the current project scope. SDL 1.1.7 is historical context, not an asserted current build dependency.

- **CMake 3.13+**: Minimum version required for configuring the build.
- **CMocka 2.0.2**: Chosen C-native test framework, provided as a system-installed
  Homebrew package on macOS. CMake uses config-mode discovery via the Homebrew
  CMocka prefix, and the test target is linked and registered with CTest. CMocka
  is not downloaded or vendored.

### Steps
1. Create a build directory and navigate into it:
   ```bash
   mkdir -p build && cd build
   ```

2. Configure the project using CMake:
   ```bash
   cmake ..
   ```

3. Compile the project:
   ```bash
   make
   ```

4. Run the executable:
   ```bash
   ./SynthTracker
   ```

> **Note**: This project is **legacy software**. The current Phase 1 boundary targets the SDL 1.2-era API surface used by the engine; SDL 1.1.7 describes the historical legacy context only.

## Usage
Run `-h` for usage instructions and feature details.

## Documentation
- [Vision](docs/VISION.md) — product intent, boundaries, and future direction.
- [Architecture](docs/ARCHITECTURE.md) — current-system overview and entrypoint.
- [ADR index](docs/ADR.md) — governance for future architectural decisions.
- [ASR register](docs/ASR.md) — architecturally significant requirements.
- [Glossary](docs/GLOSSARY.md) — canonical product and protocol terminology.
- [Artifacts](docs/ARTIFACTS.md) — target component-boundary artifacts and open
  contracts.
- [Agent workflow](docs/AGENT_WORKFLOW.md) — contribution gates and verification.

## License
This project is released under the **GNU GPLv3**. See `LICENSE` for details.

## Contributing
Want to help? Fork the repository and submit a pull request. For questions or legal clarifications, contact **Neochrome** at [neko@netcologne.de](mailto:neko@netcologne.de).

## Links
- Latest updates: [http://darkstar.tabu.stw-bonn.de/~neo/audio.html](http://darkstar.tabu.stw-bonn.de/~neo/audio.html)
- More tools: [http://darkstar.tabu.stw-bonn.de/~neo/](http://darkstar.tabu.stw-bonn.de/~neo/)
