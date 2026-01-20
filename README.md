# CPUSimulation-RISC-V

A cycle-accurate (or configurable-level) simulator of a RISC-V CPU designed for learning, experimentation, and testing microarchitecture ideas. This project implements a subset of the RISC-V ISA (RV32I) and can be used to run small assembly programs, study pipeline behavior (stalls, hazards, forwarding), and validate instruction semantics.

## Table of Contents

- [Features](#features)
- [Supported ISA](#supported-isa)
- [Architecture Overview](#architecture-overview)
- [Getting Started](#getting-started)
  - [Prerequisites](#prerequisites)
  - [Build](#build)
  - [Run](#run)
- [Usage Examples](#usage-examples)
- [Testing](#testing)
- [Project Structure](#project-structure)
- [Contributing](#contributing)
- [Roadmap](#roadmap)
- [License](#license)
- [Contact](#contact)

## Features

- Implements the RV32I base integer ISA (loads/stores, arithmetic, control flow).
- Configurable pipeline (single-cycle, multi-cycle, 5-stage pipeline) depending on build/config.
- Hazard detection and simple forwarding to reduce stalls.
- Basic assembler or support for loading binary ELF or raw hex images (depending on implementation).
- Memory model and register file with diagnostic logging for each cycle.
- Tracing mode to produce per-cycle state for debugging (PC, instructions, pipeline latches).
- Testbench harness for running small programs and unit tests for core modules.

## Supported ISA

- RV32I (subset)
  - Integer arithmetic and logical operations (ADD, SUB, AND, OR, XOR, SLT, SLTU, etc.)
  - Immediate arithmetic (ADDI, ANDI, ORI, etc.)
  - Loads and stores (LB/LH/LW/SB/SH/SW) — aligned accesses recommended
  - Control flow (JAL, JALR, BEQ, BNE, BLT, BGE, BLTU, BGEU)
  - LUI/AUIPC
- Extensions not implemented: F/D (floating point), atomics (A), compressed (C) — may be added later.

Adjust the README below if your implementation supports a different set.

## Architecture Overview

This simulator is structured in modular components:

- Frontend / Assembler: Parses assembly into machine code (or accepts pre-assembled binaries).
- Instruction Decoder: Decodes binary instructions to internal micro-ops.
- Pipeline: Implementation of pipeline stages (IF, ID, EX, MEM, WB) or alternative timing models.
- Register File: 32 general-purpose registers (x0 hardwired to 0).
- Memory: Byte-addressable memory module with configurable size and initial contents loader.
- Control & Hazard Unit: Detects data/control hazards and inserts stalls / performs forwarding.
- Tracing & Logging: Optional cycle-by-cycle trace output for visualization and debugging.

Diagram (conceptual):
IF -> ID -> EX -> MEM -> WB

## Getting Started

These steps assume a typical source layout. Adapt commands to your project's build system (Make/CMake/Python/Go/Java/C#).

### Prerequisites

- A modern C/C++ compiler (gcc/clang) if implemented in C/C++
- Python 3.8+ if implemented in Python
- make and/or CMake if build system uses them
- Optional: objdump/riscv toolchain if assembling real RISC-V binaries

### Build

If your project uses Make:

```bash
git clone https://github.com/afosh1/CPUSimulation-RISC-V.git
cd CPUSimulation-RISC-V
make         # or make all
```

If using CMake:

```bash
mkdir build && cd build
cmake ..
cmake --build .
```

If this is a Python project:

```bash
python -m venv .venv
source .venv/bin/activate
pip install -r requirements.txt
```

Adjust commands to the repository's actual build instructions.

### Run

Basic run (CLI simulator):

```bash
# Example: run program.bin with 65536 bytes of memory
./simulator --binary examples/hello.bin --mem-size 65536 --trace
```

Python example:

```bash
python run_sim.py --program examples/hello.asm --trace
```

Options commonly supported:
- --binary / --program: path to program (binary, ELF, or assembly)
- --mem-size: memory size in bytes
- --trace: enable cycle-by-cycle trace output
- --pipeline: choose pipeline mode (single, multi, pipeline)
- --max-cycles: limit simulation cycles

## Usage Examples

Assemble and run a simple "hello" program (if assembler included):

```bash
tools/assembler examples/hello.s -o examples/hello.bin
./simulator --binary examples/hello.bin --trace
```

Run tests:

```bash
make test
# or
pytest tests/
```

Trace output sample (per cycle):
Cycle 0001: PC=0x00000000 IF=0x00000093 (addi x1, x0, 0x0)
Registers: x1=0x00000000 ...
Pipeline latches: IF/ID: ..., ID/EX: ...

## Testing

- Unit tests for decoder, ALU, memory, pipeline registers.
- Integration tests: small programs with expected register/memory states after execution.
- Test harness scripts usually live in the tests/ directory.

To add a new test:
1. Add assembly/binary in tests/programs/
2. Add an expected output file (registers/memory) or a test case in tests/test_xxx.py
3. Run the test suite and verify output.

## Project Structure (example)

- src/           - simulator source code
- include/       - headers (C/C++)
- tools/         - assembler, utilities
- examples/      - sample programs
- tests/         - unit and integration tests
- docs/          - design notes and architecture diagrams
- scripts/       - helper scripts (run_all, generate_traces)

Adjust to your repository's actual layout.

## Contributing

Contributions welcome!

- Open an issue describing the feature or bug.
- Fork the repository, create a feature branch, and submit a pull request.
- Follow the coding style in the repository; include tests for new functionality.
- Document new features in README or docs/.

Guidelines:
- Keep commits focused and atomic.
- Write clear commit messages.
- Run tests before提交 (run the test suite).

## Roadmap

Planned improvements (examples):
- Support RV64I
- Add floating-point extensions (F/D)
- Implement compressed instruction support (C)
- Better debug/visualization UI (web-based pipeline visualizer)
- More advanced branch prediction algorithms

## License

Specify your license here (e.g., MIT, Apache-2.0). If you don't have one yet, add a LICENSE file. Example:

This project is licensed under the MIT License — see the [LICENSE](LICENSE) file for details.

## Contact

Maintainer: afosh1 (https://github.com/afosh1)

If you want a customized README that matches the repository's exact language, files, and build system, point me to the repository contents or tell me which language/build system this project uses and I will tailor this README accordingly.
