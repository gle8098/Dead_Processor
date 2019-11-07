#include "assembler.hpp"
#include "cstring"
#include "objdump.hpp"

#include <cstdio>

#ifdef FRIDAY_ASM_MAIN
int main(int argc, char* argv[]) {
    auto args = ParseAssemblerArgs(argc, argv);
    if (args._bad_syntax) {
        PrintAssemblerHelp();
        return 0;
    }
    AssemblyAndLink(args);
}

#endif

//**  FUNCTIONS FOR PARSING COMMAND LINE ARGUMENTS  **//
//#################################################################################################
AssemblerArgs ParseAssemblerArgs(int argc, char **argv) {
    AssemblerArgs result;
    result.output_filename = "a.friday";

    int i = 1;
    for (; i < argc; i += 2) {
        if (argv[i][0] != '-') {
            break;
        }

        if (i + 1 >= argc) {
            printf("error: nothing after '%s' argument\n", argv[i]);
            result._bad_syntax = true;
            return result;
        }
        if (strcmp(argv[i], "-o") == 0) {
            result.output_filename = argv[i + 1];
        }
    }

    result.input_files.reserve(argc - i);
    for (; i < argc; ++i) {
        if (argv[i][0] == '-') {
            printf("error: parameter '%s' must be before input files list\n", argv[i]);
            result._bad_syntax = true;
            return result;
        }
        result.input_files.push_back(argv[i]);
    }

    if (result.input_files.empty()) {
        printf("error: no files to compile\n");
        result._bad_syntax = true;
    }

    return result;
}

void PrintAssemblerHelp() {
    printf("friday-asm [-o <out_filename>] <main_file> [other files...]\n"
           "Assembly and link .friday program\n"
           "-o : specify output filename, default is \"a.friday\"\n"
           "<main_file>, [other files] : files to assembly. Code execution will start from first instruction of <main_file>\n");
}
//#################################################################################################

/*
FILE& operator << (FILE& stream, int key) { fprintf (&stream, "%d", key); return stream; }

*stdout << 42;
*/
