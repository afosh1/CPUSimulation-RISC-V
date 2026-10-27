# CPUSimulation-RISC-V

A RISC-V (RV32I) CPU simulator in C++ with a **3D visualization in Unreal Engine 5**:
each call to `Step()` fetches, decodes and executes one instruction, and the scene
shows it happening: register pillars light up for the source and destination
registers, the wires on the active datapath glow, the ALU shows its operation, and
a floating label displays the disassembled instruction.

> **Successor:** I rewrote the simulator core in Rust as
> [rv32i-sim](https://github.com/afosh1/rv32i-sim). It passes all 42 official
> riscv-tests `rv32ui` tests, writes VCD waveforms, and
> [runs in the browser](https://afosh1.github.io/#demo). Porting it also surfaced
> the bugs listed under [Known issues](#known-issues) below.

## What's here

| File | What it does |
|---|---|
| `RISCV_CPU.h/.cpp` | The CPU core: 32 registers, PC, 1 MiB byte-addressable little-endian memory, `Decode` (all RV32I immediate formats with sign extension), `Execute`, loads/stores, and a disassembler |
| `RISCV_Processor.h/.cpp` | The UE5 actor: owns a `RISCV_CPU`, builds the register pillars, ALU mesh and text labels, and exposes `Step()` / `ResetAndLoad()` to Blueprints |
| `SimManager.h/.cpp` | Drives the datapath visualization: makes tagged wire actors glow for the registers an instruction reads and writes |
| `Programs.h/.cpp` | The demo program: a hand-assembled Fibonacci loop |
| `CPUSimulator.*` | UE5 module boilerplate (depends on Core, Engine, InputCore, Niagara) |

The CPU is single-cycle: one `Step()` executes one whole instruction.

## Supported instructions

RV32I integer instructions: `LUI`, `AUIPC`, `JAL`, `JALR`, all six branches, `LB/LH/LW/LBU/LHU`,
`SB/SH/SW`, and the register-register and register-immediate ALU operations (`ADD/SUB`, shifts,
`SLT/SLTU`, `XOR/OR/AND`). `ECALL`/`EBREAK` decode but do nothing yet.

## Using it in Unreal Engine 5

These are the C++ sources of a UE5 project module named `CPUSimulator`:

1. Create a UE5 C++ project called `CPUSimulator` and copy these files into `Source/CPUSimulator/`.
2. Place an `ARISCV_Processor` and an `ASimManager` actor in the level, and assign the pillar
   mesh and the default/active/glow materials in the Details panel.
3. Tag the wire actors `Wire`, plus per-path tags such as `W_MuxA_x5`, so `ASimManager` can light them.
4. Call `Step()` (e.g. from a key binding in a Blueprint) to execute one instruction; `ResetAndLoad()` reloads the program.

## Known issues

Kept here for honesty; all of these are fixed in [rv32i-sim](https://github.com/afosh1/rv32i-sim):

- `ADD` and the shifts operate on `int32_t`, so overflow and left-shifting negative values are undefined behaviour in C++ (RISC-V wraps).
- The memory bounds check `addr + size > MEMORY_SIZE` is done in `uint32_t` and can wrap around for addresses near `0xFFFFFFFF`.
- The disassembler checks `funct7 == 0x20` before `funct3`, so `SRA` is printed as `SUB`.
