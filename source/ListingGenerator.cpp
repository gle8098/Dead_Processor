#include "ListingGenerator.hpp"
#include "friday_asm_lang.hpp"
#include "utility/BytesHelper.hpp"

using namespace FridayArch;
using namespace BytesHelper;

ListingGenerator::ListingGenerator(FILE *outstream) :
        outstream(outstream)
{}

void PrintRawBytesAndAlign(FILE* outstream, const char* bytes, int length) {
    for (int i = 0; i < length; ++i) {
        if (i % 2 == 0 && i > 0) {
            fprintf(outstream, " ");
        }
        fprintf(outstream, "%02x", bytes[i]);
    }

    int chars_printed = 2 * length + (length - 1) / 2;  // Количество уже выведенных символов
    for (; chars_printed < 19; ++chars_printed) {  // 19 = столько символов выводится, когда печатается заголовок (8 байт)
        fprintf(outstream, " ");
    }
}

int ListingGenerator::PrintInstruction(const char *code, int code_length) {
    Instruction* inst = GetInstructionByBytecode(*code);
    if (inst->inst_full_size > code_length) {
        return -1;
    }

    fprintf(outstream, "%04x\t", offset);
    PrintRawBytesAndAlign(outstream, code, inst->inst_full_size);
    fprintf(outstream, "\t%s", inst->name);
    code += sizeof(friday_inst_t);

    for (int i = 0; i < inst->args_count; ++i) {
        if (i != 0) {
            fprintf(outstream, ",");
        }

        switch (inst->args[i]) {
            case CONSTANT:
                // todo: print float
                static_assert(sizeof(int32_t) == sizeof(friday_constant_t));
                fprintf(outstream, " %d", ReadFromBytes<int32_t>(code));
                break;
            case REGISTER:
                fprintf(outstream, " r%d", ReadFromBytes<friday_reg_t>(code));
                break;
            case LABEL:
                fprintf(outstream, " <file_start+%04x>", ReadFromBytes<friday_address_t>(code));
                break;
            case _BAD_ARG: return -1;
        }
        code += GetInstructionArgumentSize(inst->args[i]);
    }
    fprintf(outstream, "\n");

    offset += inst->inst_full_size;
    return inst->inst_full_size;
}

int ListingGenerator::PrintHeader(const char *code, int code_length) {
    if (offset != 0 || code_length < HEADER_SIZE) {
        return -1;
    }

    // Check FRDY
    for (int i = 0; i < HEADER_ASM_VER_OFFSET; ++i) {
        if (FRDY[i] != code[i]) {
            return -1;
        }
    }

    auto version = ReadFromBytes<int16_t>(code + HEADER_ASM_VER_OFFSET);
    auto regs = ReadFromBytes<friday_reg_t>(code + HEADER_REG_COUNT_OFFSET);
    fprintf(outstream, "0000\t");
    PrintRawBytesAndAlign(outstream, code, HEADER_SIZE);
    fprintf(outstream, "\t{FRIDAY EXECUTABLE} Target arch version = %d; number of registers = %d.\n", version, regs);

    offset = HEADER_SIZE;
    return HEADER_SIZE;
}

int ListingGenerator::GetOffset() const {
    return offset;
}
