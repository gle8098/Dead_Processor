#include <cassert>
#include <cstdarg>
#include "assembler_inside_facade.hpp"
#include "utility/FileHelper.hpp"

using namespace FridayArch;

std::vector<std::string_view> SplitLine(const char *line, int& index) {
    std::vector<std::string_view> result;

    int word_start = -1;
    bool comment = false;

    for (; true; ++index) {
        if (!comment) {
            bool word_ended = !isgraph(line[index]);
            if (line[index] == '#') {
                comment = true;
                word_ended = true;
            } else if (!word_ended && word_start == -1) {
                word_start = index;
            }

            if (word_ended && word_start != -1) {
                assert(index - word_start > 0);
                result.emplace_back(line + word_start, index - word_start);
                word_start = -1;
            }
        }

        if (line[index] == '\n' || line[index] == '\0') {
            break;
        }
    }

    return result;
}

bool CompileAndLinkFile(const char* file, TextLocation& loc, FridayAsmWriter& writer, bool now_linking) {
    int index = 0;
    friday_address_t cur_code_offset = writer.GetCurrentCodeOffset();

    // In each iteration of cycle is only one line read
    for (; file[index] != '\0';) {
        auto line = SplitLine(file, index);

        if (line.empty()) {
            // Line is empty => ignore
        } else if (line[0][0] == '.') {
            // Dot-command
            if (!CompileDotCommand(line, loc, writer)) {
                return false;
            }
        } else if (line[0][line[0].size() - 1] == ':') {
            // Label
            line[0].remove_suffix(1);  // Remove ':'
            if (!now_linking) {
                writer.RegisterLabelAtCurrentOffset(line[0]);
            } else {
                assert(writer.GetLabelAddress(line[0]) == writer.GetCurrentCodeOffset());
            }
        } else {
            // Instruction

            // Сначала обработает аргументы: удалим символы запятой
            bool last_was_comma = false;
            for (int i = 1; i < line.size(); ++i) {
                if (line[i][line[i].size() - 1] == ',') {
                    if (last_was_comma) {
                        loc.PrintCompileMessage("error: argument expected after comma");
                        return false;
                    }
                    last_was_comma = true;
                    line[i].remove_suffix(1);
                    if (line[i].empty()) {
                        line.erase(line.begin() + i);
                        --i;
                    }
                }
            }

            if (!writer.WriteInstruction(line, now_linking)) {
                return false;
            }
        }

        if (writer.GetCurrentCodeOffset() == static_cast<friday_address_t>(-1)) {
            loc.PrintCompileMessage("internal error: current code offset is more than maximal code offset in friday "
                                     "architecture, which is %d", MAX_ADDRESS_VALUE);
            return false;
        }

        // Now read '\n'
        loc.IncLine();
        ++index;
    }

    if (now_linking) {
        return true;
    } else {
        writer.RemoveCodeFrom(cur_code_offset);
        loc.ResetFile();
        return CompileAndLinkFile(file, loc, writer, true);
    }
}

bool CompileDotCommand(const std::vector<std::string_view> &line, TextLocation &loc, FridayAsmWriter& writer) {
    if (line[0] == ".friday_asm") {
        if (line.size() > 2) {
            loc.PrintCompileMessage("error: excepted one optional argument after .friday_asm");
            return false;
        }
        int asm_ver = ARCH_VERSION;
        if (line.size() == 2) {
            char *bad_ptr = nullptr;
            std::string arg_str(line[1]);
            asm_ver = strtol(arg_str.c_str(), &bad_ptr, 10);
        }

        if (asm_ver != ARCH_VERSION) {
            loc.PrintCompileMessage("fatal: file is using arch version %d, but this is compiler of "
                                     "version %d. Abort", asm_ver, ARCH_VERSION);
        }
    } else if(line[0] == ".registers") {
        char* bad_ptr = nullptr;
        std::string arg_str(line[1]);
        int regs_value = strtol(arg_str.c_str(), &bad_ptr, 10);
        if (regs_value < 0 || regs_value > MAX_REGISTER_INDEX) {
            loc.PrintCompileMessage("error: invalid number of registers: %d. Excepted a key between 0 and %d",
                                regs_value, MAX_REGISTER_INDEX);
            return false;
        }
        if (writer.IsCustomRegisterValue()) {
            // В других файлах мог быть объявлен свой .registers. Тогда нужно взять максимум, чтобы регистров хватило
            // всем компилируемым файлам
            regs_value = std::max<int>(writer.GetCurrentRegisterCount(), regs_value);
        }
        writer.SetCustomRegistersCount(regs_value);
        loc.PrintCompileMessage("warning: using .registers dot-command is not recommended\n");
    } else {
        loc.PrintCompileMessage("error: unknown .%.*s dot-command", line[0].size(), line[0].data());
        return false;
    }

    return true;
}

bool AssemblyAndLink(const AssemblerArgs &args) {
    assert(!args._bad_syntax);

    TextLocation loc;
    FridayAsmWriter writer(&loc);

    writer.WriteHeader();

    for (char* filename : args.input_files) {
        std::string _file = FileHelper::ReadFileFully(filename);
        const char* file = _file.c_str();
        loc.SetFile(filename);

        CompileAndLinkFile(file, loc, writer, false);
    }

    writer.WriteToFile(args.output_filename);
    return true;
}

void TextLocation::PrintCompileMessage(const char *text, ...) {
    printf("%s:%d  ", filename, line);

    // Pass arguments to vfprintf
    va_list argptr;
    va_start(argptr, text);
    vfprintf(stdout, text, argptr);
    va_end(argptr);

    printf("\n");
}

void TextLocation::SetFile(const char *filename) {
    this->filename = filename;
    line = 1;
}

void TextLocation::IncLine() {
    ++line;
}

void TextLocation::ResetFile() {
    line = 1;
}
