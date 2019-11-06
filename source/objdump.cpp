#include <cstdio>
#include "objdump.hpp"
#include "../utility/FileHelper.hpp"
#include "ListingGenerator.hpp"

#ifdef FRIDAY_OBJDUMP_MAIN
int main(int argc, char** argv) {
    if (argc != 2) {
        PrintObjdumpHelp();
    }
    Objdump(argv[1]);
    return 0;
}
#endif

void PrintObjdumpHelp() {
    printf("friday-objdump <.friday program>\n"
           "Display all information and assembler content from compiled .friday program\n");
}

void Objdump(const char *filename) {
    std::string file_ = FileHelper::ReadFileFullyInBinary(filename);
    const char* file = file_.c_str();

    printf("Objdump for file %s\n\n", filename);

    ListingGenerator listing(stdout);
    int file_size = file_.size();

    while (file_size > 0) {
        int bytes_read;
        if (file == file_.c_str()) {
            // We are in the beginning, read header
            bytes_read = listing.PrintHeader(file, file_size);
        } else {
            bytes_read = listing.PrintInstruction(file, file_size);
        }

        if (bytes_read < 0) {
            printf("<file_start+%04x> error reading instruction or header. Abort", listing.GetOffset());
            return;
        }

        file += bytes_read;
        file_size -= bytes_read;
    }
}
