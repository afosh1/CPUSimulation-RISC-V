#include "Programs.h"

void Run_fibonacciProgram(std::vector<uint8_t>& memoryBytes)
{
    std::vector<uint32_t> fibonacciProgram = {
        0x00100093, // 0: ADDI x1, x0, 1  (x1 = 1) -> Current
        0x00000113, // 4: ADDI x2, x0, 0  (x2 = 0) -> Previous
        // LOOP START (PC = 8)
        0x001101B3, // 8:  ADD  x3, x2, x1 (x3 = prev + curr)
        0x00100133, // 12: ADD  x2, x0, x1 (x2 = curr) -> Update Prev
        0x003000B3, // 16: ADD  x1, x0, x3 (x1 = x3)   -> Update Curr
        0xFF5FF06F  // 20: JAL  x0, -12    (Jump back to PC 8)
    };

    for (uint32_t inst : fibonacciProgram) {
        memoryBytes.push_back(inst & 0xFF);
        memoryBytes.push_back((inst >> 8) & 0xFF);
        memoryBytes.push_back((inst >> 16) & 0xFF);
        memoryBytes.push_back((inst >> 24) & 0xFF);
    }
}