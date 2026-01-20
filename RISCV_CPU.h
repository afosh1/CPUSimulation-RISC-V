#pragma once

#include <cstdint> // Required for uint32_t (guarantees 32-bit integers)
#include <iostream>
#include <vector>

enum class OpcodeType : uint32_t {
    LUI     = 0x37, // U-Type (Load Upper Immediate)
    AUIPC   = 0x17, // U-Type (Add Upper Immediate to PC)
    JAL     = 0x6F, // J-Type (Jump And Link)
    JALR    = 0x67, // I-Type (Jump And Link Register)
    BRANCH  = 0x63, // B-Type (Conditional Branches like BEQ)
    LOAD    = 0x03, // I-Type (Load from memory)
    STORE   = 0x23, // S-Type (Store to memory)
    OP_IMM  = 0x13, // I-Type (Arithmetic with Immediate, e.g., ADDI)
    OP      = 0x33, // R-Type (Register-Register ops, e.g., ADD) - No Immediate
    SYSTEM  = 0x73  // I-Type (System calls)
};

/**
 * Struct to hold the "broken down" parts of a 32-bit instruction.
 * This makes the instruction easier to process in the Execution stage later.
 */
struct DecodedInstruction {
    uint32_t opcode; // Operation Code: Determines the general type of instruction
    uint32_t rd;     // Destination Register: Where the result goes
    uint32_t funct3; // Function 3: A 3-bit field to distinguish instructions with the same Opcode
    uint32_t rs1;    // Source Register 1: The first operand
    uint32_t rs2;    // Source Register 2: The second operand
    uint32_t funct7; // Function 7: A 7-bit field for extra distinction (e.g., ADD vs SUB)
    // The reconstructed immediate value (signed 32-bit integer)
    int32_t imm;
};

class RISCV_CPU {
public:
    RISCV_CPU();
    ~RISCV_CPU();

    /**
     * Main Decoding Function.
     * Takes a raw 32-bit machine code and extracts its internal fields.
     */
    DecodedInstruction Decode(uint32_t inst);
    void Execute(DecodedInstruction& inst);

    // Helper to print details to the console (for debugging purposes)
    void PrintDecodedInst(const DecodedInstruction& dec);


    // Memory size
    static const uint32_t MEMORY_SIZE = 1024 * 1024;

    void LoadMemory(const std::vector<uint8_t>& programData, uint32_t startAddr);
    uint32_t GetRegisterValue(int reg_index) const;
    uint32_t GetPC() const;
    uint32_t FetchInstruction();
    FString Disassemble(const DecodedInstruction& inst);
    
    // Debug helper
    void DebugDump();

private:

    // Helper function to reconstruct the immediate value
    int32_t GenerateImmediate(uint32_t inst, uint32_t opcode);
    // --- State Elements ---
    uint32_t Registers[32]; // x0-x31 general purpose registers
    uint32_t PC;            // Program Counter

    // --- Bitmasks & Shift Constants ---
    // These constants map to the RISC-V 32-bit instruction format.
    static const uint32_t OPCODE_MASK = 0x7F;
    static const uint32_t REG_MASK    = 0x1F;
    static const uint32_t FUNCT3_MASK = 0x07;
    static const uint32_t FUNCT7_MASK = 0x7F;

    // Shift amounts define where the field starts in the 32-bit instruction.
    static const int RD_SHIFT      = 7;
    static const int FUNCT3_SHIFT  = 12;
    static const int RS1_SHIFT     = 15;
    static const int RS2_SHIFT     = 20;
    static const int FUNCT7_SHIFT  = 25;

    // The Physical Memory (RAM)
    std::vector<uint8_t> Memory;
    uint32_t MemRead(uint32_t addr, int size, bool signed_extend);
    void MemWrite(uint32_t addr, uint32_t data, int size);
};