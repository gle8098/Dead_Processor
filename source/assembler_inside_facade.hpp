#pragma once

#include "assembler.hpp"
#include "utility/StringHashTable.hpp"
#include "friday_asm_lang.hpp"

std::vector<std::string_view> SplitLine(const char* line, int& index);

// Класс, знающий, какую строчку мы сейчас компилируем, и умеющий печатать текст ошибки
struct TextLocation {
    const char* filename = nullptr;
    int line = -1;

    void SetFile(const char* filename);
    void IncLine();
    void ResetFile();

    void PrintCompileMessage(const char* text, ...);
};


class FridayAsmWriter {
    TextLocation* loc;
    std::vector<char> bytecode;
    StringHashTable<FridayArch::friday_address_t> labels;
    bool custom_register_count = false;

    template <typename FRIDAY_ARG_TYPE>
    inline void WriteToBuffer(const FRIDAY_ARG_TYPE& argument, int code_offset = -1);

public:
    explicit FridayAsmWriter(TextLocation* loc):
            loc(loc)
    {}

    void WriteHeader();

    FridayArch::InstructionArgument ParseAndCompileArgument(const std::string_view& arg, bool link);

    bool WriteInstruction(const std::vector<std::string_view>& inst_and_args, bool link);

    void WriteToFile(const char* filename) const;

    FridayArch::friday_address_t GetCurrentCodeOffset() const;

    void RegisterLabelAtCurrentOffset(const std::string_view& label);

    FridayArch::friday_address_t GetLabelAddress(const std::string_view& label);

    void RemoveCodeFrom(FridayArch::friday_address_t from_address);

    FridayArch::friday_reg_t GetCurrentRegisterCount() const;

    void SetCustomRegistersCount(FridayArch::friday_reg_t count);

    bool IsCustomRegisterValue() const;
};


bool CompileAndLinkFile(const char* file, TextLocation& loc, FridayAsmWriter& writer, bool now_linking);
bool CompileDotCommand(const std::vector<std::string_view>& line, TextLocation& loc, FridayAsmWriter& writer);
