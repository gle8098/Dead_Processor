#pragma once

#include <string>
#include <vector>

#ifdef FRIDAY_ASM_MAIN
// Установите этот макрос, чтобы скомпилировать точку входа для ассемблера
int main(int argc, char** argv);
#endif

// Параметры, необходимые для запуска ассемблера
typedef struct AssemblerArgs {
    const char *output_filename = nullptr;
    std::vector<char*> input_files;

    bool _bad_syntax = false;

    AssemblerArgs() = default;
} AssemblerArgs;

AssemblerArgs ParseAssemblerArgs(int argc, char **argv);
void PrintAssemblerHelp();

bool AssemblyAndLink(const AssemblerArgs& args);
