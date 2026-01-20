#include "RISCV_CPU.h"
#include <iomanip> // Used for std::hex formatting

RISCV_CPU::RISCV_CPU() {
    
    // Resize Memory - fill with zeros
    Memory.resize(MEMORY_SIZE, 0);
    // Initialize Program Counter to 0 (or entry point)
    PC = 0;

    // Initialize all registers to 0
    for (int i = 0; i < 32; i++) {
        Registers[i] = 0;
    }
}

RISCV_CPU::~RISCV_CPU() {
    // Cleanup if necessary
}

DecodedInstruction RISCV_CPU::Decode(uint32_t inst) {
    DecodedInstruction decoded;

    // Extract standard fields
    decoded.opcode = inst & OPCODE_MASK;
    decoded.rd = (inst >> RD_SHIFT) & REG_MASK;
    decoded.funct3 = (inst >> FUNCT3_SHIFT) & FUNCT3_MASK;
    decoded.rs1 = (inst >> RS1_SHIFT) & REG_MASK;
    decoded.rs2 = (inst >> RS2_SHIFT) & REG_MASK;
    decoded.funct7 = (inst >> FUNCT7_SHIFT) & FUNCT7_MASK;

    // Generate the Immediate Value
    decoded.imm = GenerateImmediate(inst, decoded.opcode);

    return decoded;
}

void RISCV_CPU::PrintDecodedInst(const DecodedInstruction& dec) {
    // This function helps us debug by showing what the CPU "sees".
    std::cout << "--- Instruction Decode Result ---" << std::endl;
    std::cout << "Opcode: 0x" << std::hex << dec.opcode << std::dec << " (7 bits)" << std::endl;
    
    // Print Register Indices as standard decimals (e.g., x1, x31)
    std::cout << "Rd:     x"  << dec.rd << std::endl;
    std::cout << "Funct3: 0x" << std::hex << dec.funct3 << std::dec << std::endl;
    std::cout << "Rs1:    x"  << dec.rs1 << std::endl;
    std::cout << "Rs2:    x"  << dec.rs2 << std::endl;
    std::cout << "Funct7: 0x" << std::hex << dec.funct7 << std::dec << std::endl;
    std::cout << "---------------------------------" << std::endl;
}

int32_t RISCV_CPU::GenerateImmediate(uint32_t inst, uint32_t opcode) {
    int32_t imm = 0;
    OpcodeType op = static_cast<OpcodeType>(opcode);

    switch (op) {
        // --- I-Type (Immediate is top 12 bits) ---
        // Instructions: ADDI, LW, JALR
        case OpcodeType::JALR:
        case OpcodeType::LOAD:
        case OpcodeType::OP_IMM:
        case OpcodeType::SYSTEM:
            // Cast to signed int so '>> 20' performs arithmetic shift (preserves sign)
            imm = (int32_t)inst >> 20; 
            break;

        // --- S-Type (Immediate is split into two parts) ---
        // Instructions: SW, SH, SB
        case OpcodeType::STORE: {
            int32_t low = (inst >> 7) & 0x1F;   // Bottom 5 bits
            int32_t high = (inst >> 25) & 0x7F; // Top 7 bits
            imm = (high << 5) | low;            // Combine them
            
            // Manual Sign Extension: If the 12th bit (bit 11) is 1, make it negative
            if (imm & 0x800) imm |= 0xFFFFF800; 
            break;
        }

        // --- SB-Type (Conditional Branch) ---
        // Instructions: BEQ, BNE
        // Immediate is highly scrambled to keep register bits in fixed places
        case OpcodeType::BRANCH: {
            int32_t bit11   = (inst >> 7) & 0x1;
            int32_t bits4_1 = (inst >> 8) & 0xF;
            int32_t bits10_5= (inst >> 25) & 0x3F;
            int32_t bit12   = (inst >> 31) & 0x1; // The sign bit

            // Reassemble the bits in order: [12] [11] [10:5] [4:1] [0]
            // Note: Bit 0 is always 0 for branches
            imm = (bit12 << 12) | (bit11 << 11) | (bits10_5 << 5) | (bits4_1 << 1);

            // Sign Extension
            if (bit12) imm |= 0xFFFFE000;
            break;
        }

        // --- U-Type (Upper Immediate) ---
        // Instructions: LUI, AUIPC
        // Loads 20 bits directly into the upper part of the register
        case OpcodeType::LUI:
        case OpcodeType::AUIPC:
            imm = inst & 0xFFFFF000; // Zero out the bottom 12 bits
            break;
        
        // --- UJ-Type (Unconditional Jump) ---
        // Instructions: JAL
        // Scrambled similar to B-Type but larger (20 bits)
        case OpcodeType::JAL: {
            int32_t bits19_12 = (inst >> 12) & 0xFF;
            int32_t bit11     = (inst >> 20) & 0x1;
            int32_t bits10_1  = (inst >> 21) & 0x3FF;
            int32_t bit20     = (inst >> 31) & 0x1;

            // Reassemble: [20] [19:12] [11] [10:1] [0]
            imm = (bit20 << 20) | (bits19_12 << 12) | (bit11 << 11) | (bits10_1 << 1);

            // Sign Extension
            if (bit20) imm |= 0xFFE00000;
            break;
        }

        default:
            imm = 0; // R-Type instructions (like ADD) have no immediate
            break;
    }

    return imm;
}


void RISCV_CPU::Execute(DecodedInstruction& inst) {
    
    //OPERAND PREPARATION (The "MUX" Logic)   
    // Source 1 (rs1) is always used if the instruction needs it.
    int32_t val1 = (int32_t)Registers[inst.rs1];
    
    // Source 2 (rs2 OR Immediate):
    // - R-Type (OP) uses Register rs2.
    // - I-Type/S-Type/U-Type use the Immediate.
    int32_t val2;
    if (inst.opcode == (uint32_t)OpcodeType::OP || inst.opcode == (uint32_t)OpcodeType::BRANCH || inst.opcode == (uint32_t)OpcodeType::STORE) {
        val2 = (int32_t)Registers[inst.rs2]; // Use Register for R-Type, Branch comparison, and Store Data
    } else {
        val2 = inst.imm; // Use Immediate for everything else (ADDI, LW, JALR, etc.)
    }

    int32_t result = 0;             // The value to write back to Rd
    bool write_to_reg = true;       // Does this instruction update a register?
    uint32_t next_pc = PC + 4;      // Default: Go to next instruction

    OpcodeType op = static_cast<OpcodeType>(inst.opcode);

    // ==========================================================
    // 2. EXECUTION LOGIC (The "ALU" & Control Unit)
    // ==========================================================
    switch (op) {
        
        // --- ARITHMETIC & LOGIC (R-Type & I-Type) ---
        // (ADD, SUB, XOR, OR, AND, SLL, SRL, SRA, SLT)
        case OpcodeType::OP:
        case OpcodeType::OP_IMM:
            switch (inst.funct3) {
                case 0x0: // ADD or SUB
                    // It is SUB only if it is R-Type (OP) AND Bit 30 of funct7 is 1 (0x20)
                    if (op == OpcodeType::OP && (inst.funct7 & 0x20)) {
                        result = val1 - val2;
                    } else {
                        result = val1 + (op == OpcodeType::OP ? val2 : inst.imm); // Note: For ADDI, val2 is technically imm, but we set val2 up top differently for OP_IMM. 
                        // Correction for OP_IMM: In the setup above, for OP_IMM, we set val2 = inst.imm.
                        // So for both OP and OP_IMM, we can just do:
                        result = val1 + val2;
                    }
                    break;
                
                case 0x1: // SLL (Shift Left Logical)
                    result = val1 << (val2 & 0x1F);
                    break;
                
                case 0x2: // SLT (Set Less Than - Signed)
                    result = (val1 < val2) ? 1 : 0;
                    break;
                
                case 0x3: // SLTU (Set Less Than - Unsigned)
                    result = ((uint32_t)val1 < (uint32_t)val2) ? 1 : 0;
                    break;
                
                case 0x4: // XOR
                    result = val1 ^ val2;
                    break;
                
                case 0x5: // SRL or SRA
                    // SRA if Bit 30 of funct7 is 1 (Only valid for R-Type or SRAI)
                    // Note: SRAI (I-Type) also uses funct7 bit 30!
                    if (inst.funct7 & 0x20) {
                        result = val1 >> (val2 & 0x1F); // Arithmetic Shift (preserves sign)
                    } else {
                        result = (int32_t)((uint32_t)val1 >> (val2 & 0x1F)); // Logical Shift (zero fill)
                    }
                    break;
                
                case 0x6: // OR
                    result = val1 | val2;
                    break;
                
                case 0x7: // AND
                    result = val1 & val2;
                    break;
            }
            break;

        // --- BRANCHES (B-Type) ---
        case OpcodeType::BRANCH:
            write_to_reg = false; // Branches do not write result to register
            bool take_branch;
            take_branch = false;

            switch (inst.funct3) {
                case 0x0: take_branch = (val1 == val2); break; // BEQ
                case 0x1: take_branch = (val1 != val2); break; // BNE
                case 0x4: take_branch = (val1 < val2); break;  // BLT
                case 0x5: take_branch = (val1 >= val2); break; // BGE
                case 0x6: take_branch = ((uint32_t)val1 < (uint32_t)val2); break;  // BLTU
                case 0x7: take_branch = ((uint32_t)val1 >= (uint32_t)val2); break; // BGEU
                default:  break;
            }

            if (take_branch) {
                next_pc = PC + inst.imm;
            }
            break;

        // --- JUMPS (J-Type & I-Type) ---
        case OpcodeType::JAL:
            result = PC + 4; // Save Return Address
            next_pc = PC + inst.imm;
            break;

        case OpcodeType::JALR:
            result = PC + 4; // Save Return Address
            // Target = rs1 + imm, LSB masked to 0
            next_pc = (val1 + inst.imm) & ~1;
            break;

        // --- UPPER IMMEDIATES (U-Type) ---
        case OpcodeType::LUI:
            result = inst.imm; // Load Upper Immediate directly
            break;

        case OpcodeType::AUIPC:
            result = PC + inst.imm; // PC + Offset
            break;

        // --- MEMORY (I-Type & S-Type) ---
        // (Placeholder - We will implement Memory Access Next)
        case OpcodeType::LOAD: {
            uint32_t addr = val1 + inst.imm; // Effective Address = rs1 + offset
            
            switch (inst.funct3) {
                case 0x0: // LB (Load Byte - Signed)
                    result = MemRead(addr, 1, true);
                    break;
                case 0x1: // LH (Load Half - Signed)
                    result = MemRead(addr, 2, true);
                    break;
                case 0x2: // LW (Load Word)
                    result = MemRead(addr, 4, true); // Sign extension doesn't matter for 32-bit, but true is fine
                    break;
                case 0x4: // LBU (Load Byte - Unsigned)
                    result = MemRead(addr, 1, false);
                    break;
                case 0x5: // LHU (Load Half - Unsigned)
                    result = MemRead(addr, 2, false);
                    break;
                default:
                    break;
            }
            break;
        }

        case OpcodeType::STORE: {
            write_to_reg = false; // Stores do NOT write to rd
            uint32_t addr = val1 + inst.imm; // Effective Address
            
            // For Store, val2 holds the data we want to write (from rs2)
            switch (inst.funct3) {
                case 0x0: // SB (Store Byte)
                    MemWrite(addr, val2, 1);
                    break;
                case 0x1: // SH (Store Half)
                    MemWrite(addr, val2, 2);
                    break;
                case 0x2: // SW (Store Word)
                    MemWrite(addr, val2, 4);
                    break;
                default:
                    break;
            }
            break;
        }

        case OpcodeType::SYSTEM:
            write_to_reg = false;
            // Handle ECALL / EBREAK here later
            break;

        default:
            // Illegal Instruction
            break;
    }

    // ==========================================================
    // 3. WRITE BACK (The "RegWrite" Logic)
    // ==========================================================
    // Rule: Never write to x0.
    if (inst.rd != 0 && write_to_reg) {
        Registers[inst.rd] = (uint32_t)result;
    }

    // ==========================================================
    // 4. PC UPDATE
    // ==========================================================
    PC = next_pc;
}

uint32_t RISCV_CPU::MemRead(uint32_t addr, int size, bool signed_extend) {
    // 1. Bounds Check (Prevent crashing if address is out of range)
    if (addr + size > MEMORY_SIZE) {
        std::cerr << "Error: Memory Read Out of Bounds at " << std::hex << addr << std::endl;
        return 0;
    }

    uint32_t value = 0;

    // 2. Read Bytes (Little Endian: LSB at addr)
    for (int i = 0; i < size; i++) {
        value |= ((uint32_t)Memory[addr + i]) << (i * 8);
    }

    // 3. Sign Extension (If requested)
    // If accessing Byte (8-bit) and the 8th bit is 1, fill upper bits.
    // If accessing Half (16-bit) and the 16th bit is 1, fill upper bits.
    if (signed_extend) {
        int bit_width = size * 8;
        // Check if the MSB (Sign bit) is set
        if ((value >> (bit_width - 1)) & 1) {
            // Create a mask of 1s for the upper bits
            // e.g., if width is 8, mask is 0xFFFFFF00
            uint32_t mask = 0xFFFFFFFF << bit_width;
            value |= mask;
        }
    }

    return value;
}

void RISCV_CPU::MemWrite(uint32_t addr, uint32_t data, int size) {
    // 1. Bounds Check
    if (addr + size > MEMORY_SIZE) {
        std::cerr << "Error: Memory Write Out of Bounds at " << std::hex << addr << std::endl;
        return;
    }

    // 2. Write Bytes (Little Endian)
    // We take the bottom 8 bits, write them, shift data right, repeat.
    for (int i = 0; i < size; i++) {
        Memory[addr + i] = (data >> (i * 8)) & 0xFF;
    }
}

uint32_t RISCV_CPU::GetRegisterValue(int reg_index) const {
    if (reg_index < 0 || reg_index > 31) return 0;
    return Registers[reg_index];
}

uint32_t RISCV_CPU::GetPC() const {
    return PC;
}

void RISCV_CPU::DebugDump() {
    std::cout << " [State] PC:" << PC 
              << " x1:" << Registers[1] 
              << " x2:" << Registers[2] 
              << " x3:" << Registers[3] << std::endl;
}

uint32_t RISCV_CPU::FetchInstruction() {
    
    return MemRead(PC, 4, false);
}

void RISCV_CPU::LoadMemory(const std::vector<uint8_t>& programData, uint32_t startAddr) {
    for (size_t i = 0; i < programData.size(); i++) {
        if (startAddr + i < MEMORY_SIZE) {
            Memory[startAddr + i] = programData[i];
        }
    }
}

FString RISCV_CPU::Disassemble(const DecodedInstruction& inst) {
    FString OpName = TEXT("UNKNOWN");
    OpcodeType op = static_cast<OpcodeType>(inst.opcode);

    switch (op) {
    case OpcodeType::OP:
        if (inst.funct7 == 0x20) OpName = TEXT("SUB");
        else {
            switch (inst.funct3) {
            case 0x0: OpName = TEXT("ADD"); break;
            case 0x1: OpName = TEXT("SLL"); break;
            case 0x2: OpName = TEXT("SLT"); break;
            case 0x4: OpName = TEXT("XOR"); break;
            case 0x5: OpName = (inst.funct7 == 0x20) ? TEXT("SRA") : TEXT("SRL"); break;
            case 0x6: OpName = TEXT("OR");  break;
            case 0x7: OpName = TEXT("AND"); break;
            }
        }
        return FString::Printf(TEXT("%s x%d, x%d, x%d"), *OpName, inst.rd, inst.rs1, inst.rs2);

    case OpcodeType::OP_IMM:
        switch (inst.funct3) {
        case 0x0: OpName = TEXT("ADDI"); break;
        case 0x4: OpName = TEXT("XORI"); break;
        case 0x6: OpName = TEXT("ORI");  break;
        case 0x7: OpName = TEXT("ANDI"); break;
        case 0x1: OpName = TEXT("SLLI"); break;
        case 0x5: OpName = (inst.funct7 == 0x20) ? TEXT("SRAI") : TEXT("SRLI"); break;
        }
        return FString::Printf(TEXT("%s x%d, x%d, %d"), *OpName, inst.rd, inst.rs1, inst.imm);

    case OpcodeType::JAL:
        return FString::Printf(TEXT("JAL x%d, %d"), inst.rd, inst.imm);

    case OpcodeType::BRANCH:
        switch (inst.funct3) {
        case 0x0: OpName = TEXT("BEQ"); break;
        case 0x1: OpName = TEXT("BNE"); break;
        case 0x4: OpName = TEXT("BLT"); break;
        case 0x5: OpName = TEXT("BGE"); break;
        }
        return FString::Printf(TEXT("%s x%d, x%d, %d"), *OpName, inst.rs1, inst.rs2, inst.imm);

    case OpcodeType::LUI:   return FString::Printf(TEXT("LUI x%d, 0x%X"), inst.rd, inst.imm);
    case OpcodeType::AUIPC: return FString::Printf(TEXT("AUIPC x%d, %d"), inst.rd, inst.imm);

    default: return TEXT("UNKNOWN INST");
    }

}
