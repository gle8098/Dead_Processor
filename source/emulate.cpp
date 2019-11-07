#include "emulate.hpp"
#include <cstdio>
#include <cstring>
#include "Emulator.hpp"
#include "utility/FileHelper.hpp"
#include "friday_asm_lang.hpp"

using namespace FridayArch;

#ifdef FRIDAY_EMU_MAIN
int main(int argc, char** argv) {
    if (argc != 2 && (argc != 3 || strcmp(argv[1], "-d") != 0)) {
        PrintEmulatorHelp();
        return 0;
    }
    bool debug_mode = argc == 3;
    Emulate(argv[argc - 1], debug_mode);
}
#endif
void PrintEmulatorHelp() {
    printf("friday-emu [-d] <.friday program>\n"
           "Emulates executing of the program on friday processor\n"
           "-d : enables debug information, which is printed after every tick\n");
}

void Emulate(const char *filename, bool debug_mode) {
    std::string file;
    try {
        file = FileHelper::ReadFileFullyInBinary(filename);
    } catch (const std::exception& exc) {
        FileHelper::PrintErrorWorkingWithFile(filename, "reading", exc);
        return;
    }

    if (!CheckForFRDY(file.c_str())) {
        printf("error: file '%s' is not a .friday executable\n", filename);
        return;
    }

    Emulator emu;
    emu.LoadMemory(file.c_str(), file.size());
    emu.Run(debug_mode);
}
