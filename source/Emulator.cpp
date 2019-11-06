#include "Emulator.hpp"
#include "friday_asm_lang.hpp"
#include <cstring>
#include "../utility/BytesHelper.hpp"
#include <cstdio>

using namespace FridayArch;

Emulator::Emulator() :
    sp(-1),
    ip(-1),
    ap(-1),
    mem(new char[MEMORY_SIZE])
{}

Emulator::~Emulator() {
    delete[] mem;
}

void Emulator::push(const char *bytes, int length) {
    sp -= length;
    std::memcpy(mem + sp, bytes, length);
}

const char *Emulator::pop(int byte_length) {
    sp += byte_length;
    return mem + sp - byte_length;
}

const char *Emulator::get_arg_ptr() const {
    return mem + ap;
}
char *Emulator::get_stack_ptr() const {
    return mem + sp;
}

int Emulator::pop_int() {
    int res = BytesHelper::BytesAs<int>(mem + sp);
    sp += 4;
    return res;
}

float Emulator::pop_float() {
    float res = BytesHelper::BytesAs<float>(mem + sp);
    sp += 4;
    return res;
}

void Emulator::LoadMemory(const char *program, int program_size) {
    std::memcpy(mem, program, program_size);
    int regs_count = BytesHelper::BytesAs<int>(program, HEADER_REG_COUNT_OFFSET);
    regs.assign(regs_count, 0);
    ip = HEADER_SIZE;
    sp = MEMORY_SIZE - 1;
    signal = NO_SIGNAL;
}

void Emulator::Run(bool debug_mode) {
    while (true) {
        if (debug_mode) {
            PrintDebugInfo();
        }

        // Check signals
        switch (signal) {
            case SIGNAL_MEMORY_NOT_READY:
                printf("Emulator error: memory for emulator is not loaded, cannot run.\n");
                return;
            case SIGNAL_EXIT:
                return;
            case SIGNAL_SIGILL:
            case SIGNAL_SIGSEGV:
                printf("FATAL SIGNAL %d. ip = 0x%08x, sp = 0x%08x\n", signal, ip, sp);
                return;
            default: break;
        }

        Instruction* inst = GetInstructionByBytecode(mem[ip]);
        if (inst == nullptr) {
            signal = SIGNAL_SIGILL;
        } else {
            ap = ip + sizeof(friday_inst_t);
            ip += inst->inst_full_size;
            inst->callback(this);
        }
    }
}

void Emulator::PrintDebugInfo() const {
    printf("ip = 0x%08x, sp = 0x%08x, ap = %08x, sig = %d | ", ip, sp, ap, signal);
    Instruction* inst = GetInstructionByBytecode(mem[ip]);
    if (inst != nullptr) {
        printf("next inst is %02x (%s)", inst->inst, inst->name);
    } else {
        printf("unk inst to execute next %02x", mem[ip]);
    }
    printf("\n");
}
