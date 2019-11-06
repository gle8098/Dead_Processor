#include "assembler_inside_facade.hpp"
#include "utility/FileHelper.hpp"
#include <cstring>
#include <cassert>
#include "utility/BytesHelper.hpp"

using namespace FridayArch;

template<typename FRIDAY_ARG_TYPE>
void FridayAsmWriter::WriteToBuffer(const FRIDAY_ARG_TYPE &argument, int code_offset) {
    if (code_offset == -1) {
        code_offset = bytecode.size();
        bytecode.insert(bytecode.end(), sizeof(FRIDAY_ARG_TYPE), 0);
    }
    BytesHelper::WriteBytes(bytecode.data(), argument, code_offset);
}

void FridayAsmWriter::WriteHeader() {
    bytecode.insert(bytecode.end(), HEADER_SIZE, 0);
    std::memcpy(bytecode.data(), FRDY, HEADER_ASM_VER_OFFSET);
    WriteToBuffer(ARCH_VERSION, HEADER_ASM_VER_OFFSET);
    WriteToBuffer(DEFAULT_REG_COUNT, HEADER_REG_COUNT_OFFSET);
}

bool FridayAsmWriter::WriteInstruction(const std::vector<std::string_view> &args, bool link) {
    assert(!args.empty());

    std::vector<InstructionArgument> types(args.size() - 1);

    // Reserve memory for instruction command
    size_t inst_offset = bytecode.size();
    bytecode.insert(bytecode.end(), sizeof(friday_inst_t), 0);

    for (int i = 1; i < args.size(); ++i) {
        auto type = types[i - 1] = ParseAndCompileArgument(args[i], link);
        if (type == _BAD_ARG) {
            return false;
        }
    }

    Instruction* inst = FindInstructionBySignature(args[0], types.size(), types.data());
    if (inst == nullptr) {
        loc->PrintCompileMessage("error: undefined instruction");
        printf("\t%.*s  arg_types[", static_cast<int>(args[0].size()), args[0].data());
        for (auto type : types) {
            printf("%s, ", GetInstructionArgumentName(type));
        }
        printf("]\n");
        return false;
    }

    WriteToBuffer(inst->inst, inst_offset);
    return true;
}

FridayArch::InstructionArgument FridayAsmWriter::ParseAndCompileArgument(const std::string_view &arg, bool link) {
    char* bad_ptr = nullptr;
    std::string arg_(arg);

    // Is int?
    int value_int = strtol(arg_.c_str(), &bad_ptr, 0);
    if (*bad_ptr == '\0') {
        // It is int
        WriteToBuffer(static_cast<friday_constant_t>(value_int));
        return CONSTANT;
    }

    // Is float?
    float value_float = strtof(arg_.c_str(), &bad_ptr);
    if (*bad_ptr == '\0') {
        // It is float
        WriteToBuffer(static_cast<friday_constant_t>(value_float));
        return CONSTANT;
    }

    // Is register?
    if (arg[0] == 'r' && arg.size() > 1) {
        int reg_index = strtol(arg_.c_str() + 1, &bad_ptr, 10);
        if (*bad_ptr == '\0') {
            // It is register
            int cur_max_regs = GetCurrentRegisterCount();
            if (reg_index < 0 || reg_index >= cur_max_regs) {
                loc->PrintCompileMessage("error: register %s is out of range. Program has asked for only %d registers",
                                    arg_.c_str(), cur_max_regs);
                return _BAD_ARG;
            }
            WriteToBuffer(static_cast<friday_reg_t>(reg_index));
            return REGISTER;
        }
    }

    // Now it must be a label. Check it doesn't start with a digit
    if (isdigit(arg[0])) {
        loc->PrintCompileMessage("error: bad argument '%s'", arg_.c_str());
        return _BAD_ARG;
    }

    friday_address_t addr = 0;
    if (link) {
        addr = GetLabelAddress(arg);
        if (addr == static_cast<friday_address_t>(-1)) {
            loc->PrintCompileMessage("error: label not found '%s'", arg_.c_str());
            return _BAD_ARG;
        }
    }
    WriteToBuffer(static_cast<friday_address_t>(addr));
    return LABEL;
}

void FridayAsmWriter::WriteToFile(const char *filename) const {
    FileHelper::WriteFileInBinary(filename, bytecode);
}

FridayArch::friday_address_t FridayAsmWriter::GetCurrentCodeOffset() const {
    auto res = bytecode.size();
    if (MAX_ADDRESS_VALUE < res) {
        return MAX_ADDRESS_VALUE;
    }
    return res;
}

void FridayAsmWriter::RegisterLabelAtCurrentOffset(const std::string_view &label) {
    labels.Insert(std::string(label), GetCurrentCodeOffset());
}

FridayArch::friday_address_t FridayAsmWriter::GetLabelAddress(const std::string_view &label) {
    friday_address_t* res = labels.Find(std::string(label));
    if (res == nullptr) {
        return static_cast<friday_address_t>(-1);
    }
    return *res;
}

void FridayAsmWriter::RemoveCodeFrom(FridayArch::friday_address_t from_address) {
    bytecode.erase(bytecode.begin() + from_address, bytecode.end());
}

friday_reg_t FridayAsmWriter::GetCurrentRegisterCount() const {
    return *reinterpret_cast<const friday_reg_t*>(bytecode.data() + HEADER_REG_COUNT_OFFSET);
}

void FridayAsmWriter::SetCustomRegistersCount(friday_reg_t count) {
    custom_register_count = true;
    WriteToBuffer(count, HEADER_REG_COUNT_OFFSET);
}

bool FridayAsmWriter::IsCustomRegisterValue() const {
    return custom_register_count;
}
