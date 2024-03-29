#include <fstream>
#include <cstdio>
#include "FileHelper.hpp"

std::string FileHelper::ReadFileFully(const char *filename) {
    std::ifstream file(filename);
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();

    std::string buffer(size, '\0');
    file.seekg(0);
    file.read(buffer.data(), size);
    file.close();

    return buffer;
}

std::string FileHelper::ReadFileFullyInBinary(const char *filename) {
    std::ifstream file(filename, std::ios::binary);
    file.seekg(0, std::ios::end);
    size_t size = file.tellg();

    std::string buffer(size, '\0');
    file.seekg(0);
    file.read(buffer.data(), size);
    file.close();

    return buffer;
}

void FileHelper::WriteFileInBinary(const char *filename, const std::vector<char>& bytes) {
    std::ofstream file(filename, std::ios::binary);
    file.exceptions(std::ios::badbit);
    file.write(bytes.data(), bytes.size());
}

void FileHelper::PrintErrorWorkingWithFile(const char *filename, const char *action_with_file,
        const std::exception& exc) {
    printf("Error while %s file '%s'. Check the file exists and is not a directory\n", action_with_file, filename);
    printf("Exception's 'what()': %s\n", exc.what());
}
