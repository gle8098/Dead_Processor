#pragma once

#include <string>
#include <vector>

namespace FileHelper {
    std::string ReadFileFully(const char* filename);

    std::string ReadFileFullyInBinary(const char* filename);

    void WriteFileInBinary(const char *filename, const std::vector<char>& bytes);

    // action_with_file = "reading" or "writing to"
    void PrintErrorWorkingWithFile(const char* filename, const char* action_with_file, const std::exception& exc);
}
