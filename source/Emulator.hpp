#pragma once

#include <vector>
#include <cstdint>

namespace FridayArch {

class Emulator {
public:
    const static int MEMORY_SIZE = 128 * 1024 * 1024;  // 128 kb
    const static int NO_SIGNAL = 0;
    const static int SIGNAL_EXIT = 1;
    const static int SIGNAL_SIGSEGV = 2;
    const static int SIGNAL_SIGILL = 3;
    const static int SIGNAL_MEMORY_NOT_READY = -1;

    std::vector<int32_t> regs;
    int32_t sp, ip, ap;  // special regs: stack ptr, instruction ptr (addr of next inst), argument ptr
    char* const mem;
    int signal = SIGNAL_MEMORY_NOT_READY;

    Emulator();
    ~Emulator();

    Emulator(const Emulator&) = delete;
    Emulator(Emulator&&) = delete;
    Emulator& operator=(const Emulator&) = delete;
    Emulator& operator=(Emulator&&) = delete;

    void LoadMemory(const char* program, int program_size);

    void push(const char* bytes, int length);
    int pop_int();
    float pop_float();
    // Извлекает из стека элемент с указаной длиной. Обратите внимание, что возвращается указатель на место, где
    // лежал на стеке этот элемент, поэтому он указывает на корректные данные до тех пор, пока стек лишь уменьшается.
    const char* pop(int byte_length);

    // Функция-помошник. Возвращает указатель на первый аргумент текущей функции
    // return mem + ip + sizeof(friday_inst_t)
    const char* get_arg_ptr() const;
    char* get_stack_ptr() const;

    void PrintDebugInfo() const;
    void Run(bool debug_mode);
};

}
