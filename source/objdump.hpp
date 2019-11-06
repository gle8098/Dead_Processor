#pragma once

#ifdef FRIDAY_OBJDUMP_MAIN
// Установите этот макрос, чтобы скомпилировать точку входа
int main(int argc, char** argv);
#endif

void PrintObjdumpHelp();

void Objdump(const char* filename);
