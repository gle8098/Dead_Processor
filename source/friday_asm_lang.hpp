#pragma once

#include <vector>
#include <cstdint>
#include <string_view>

constexpr unsigned int TwoInPowerOf(unsigned int power) noexcept {
    power *= 8;
    unsigned int result = 1;
    for (; power > 0; --power) {
        result *= 2;
    }
    return result;
}

namespace FridayArch {

const int16_t ARCH_VERSION = 1;

// Некоторые параметры заголовка
const static char* FRDY = "FRDY";
const static int HEADER_ASM_VER_OFFSET = 4;
const static int HEADER_REG_COUNT_OFFSET = 6;
const static int HEADER_SIZE = 8;

// Типы архитектуры Friday
typedef uint8_t  friday_reg_t;       // Тип номера регистра
typedef char     friday_inst_t;      // Тип номера инструкции
typedef uint32_t friday_constant_t;  // Тип для хранения константы
typedef uint16_t friday_address_t;   // Тип адреса

const unsigned int MAX_REGISTER_INDEX = 8;
const unsigned int MAX_INSTRUCTION_VALUE = TwoInPowerOf(sizeof(friday_inst_t)) - 1;
const unsigned int MAX_CONSTANT_VALUE = TwoInPowerOf(sizeof(friday_constant_t)) - 1;
const unsigned int MAX_ADDRESS_VALUE = TwoInPowerOf(sizeof(friday_address_t)) - 1;

const friday_reg_t DEFAULT_REG_COUNT = 4;  // Количество регистров, зарезервированных по умолчанию

class Emulator;

typedef enum A {
    CONSTANT,
    REGISTER,
    LABEL,
    _BAD_ARG
} InstructionArgument;

const char* GetInstructionArgumentName(InstructionArgument value);
size_t GetInstructionArgumentSize(InstructionArgument value);

struct Instruction {
    const char* const name;
    const friday_inst_t inst;
    const int args_count;
    const InstructionArgument *args;
    const size_t inst_full_size;
    void (*const callback)(Emulator*);

    Instruction(const char* name, friday_inst_t instruction, int args_count, const InstructionArgument *args,
                void (*callback)(Emulator*));
};

Instruction* GetInstructionByBytecode(friday_inst_t bytecode);

char RegisterInstruction(const char* name, friday_inst_t inst, int args_count, InstructionArgument *args,
        void (*callback)(Emulator*));

Instruction* FindInstructionBySignature(const std::string_view& name, int args_count, const InstructionArgument* args);

}
