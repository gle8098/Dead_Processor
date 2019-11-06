#pragma once

#ifdef FRIDAY_EMU_MAIN
// Установите этот макрос, чтобы скомпилировать точку входа
int main(int argc, char** argv);
#endif
void PrintEmulatorHelp();

void Emulate(const char* filename, bool debug_mode);
