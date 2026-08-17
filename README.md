# TFMX.cpp

A **modern C23 reimplementation and refactor** of the legacy **TFMX music
player**, with SDL 1.1.7 retained as historical legacy context. The repository
identity remains **TFMX.cpp**.

## Overview
TFMX.cpp is one system with a current transitional legacy CLI plus SDL-backed
audio. It aims to **refactor and reimplement** the legacy **TFMX player** in
C23, with a future GUI-first TFMX DAW using SDL. All TFMX-owned production and
test source, including the future GUI/DAW, remains C23; no C++ port is planned.
Third-party dependency implementation languages are evaluated separately. The
goal is to maintain compatibility with existing TFMX modules while improving
performance, maintainability, and user experience.

### Current Status
- **Refactoring progress**: Initial refactoring and setup for the C23 product
  boundary.
- **UI Integration**: Planning and design for a future GUI-first DAW; no GUI or
  editing functionality is implemented yet.
- **Legacy Compatibility**: Retaining support for legacy TFMX modules and the SDL 1.2-era audio features currently used by the engine, including stereo blending and low-pass filtering.
- **Platform scope**: macOS only. Linux and other platform support are outside the current project scope; adding a platform requires a new explicit roadmap decision.
- **Roadmap**: Phase 5 is **C23 product readiness**: a reusable C playback core
  and a C-based GUI/DAW foundation.

## Features
- Plays **most TFMX modules**, including:
  - MasterBlazer, Apidya, Turrican II/III, JimPower
  - MUDS, R-Type Theme, Z-Out, The Oath, and more
- **Stereo blending** (adjustable for headphone users)
- **Low-pass filter** (high, medium, low cutoff frequencies)
- **macOS support** (the current platform scope)
- **CoreAudio output** is the intended future macOS Audio Output Adapter; it is
  not implemented yet.

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
   ./tfmx
   ```

> **Note**: This project is **legacy software**. The current Phase 1 boundary targets the SDL 1.2-era API surface used by the engine; SDL 1.1.7 describes the historical legacy context only.

## Usage
Run `-h` for usage instructions and feature details.

## License
This project is released under the **GNU GPLv3**. See `LICENSE` for details.

## Contributing
Want to help? Fork the repository and submit a pull request. For questions or legal clarifications, contact **Neochrome** at [neko@netcologne.de](mailto:neko@netcologne.de).

## Links
- Latest updates: [http://darkstar.tabu.stw-bonn.de/~neo/audio.html](http://darkstar.tabu.stw-bonn.de/~neo/audio.html)
- More tools: [http://darkstar.tabu.stw-bonn.de/~neo/](http://darkstar.tabu.stw-bonn.de/~neo/)
