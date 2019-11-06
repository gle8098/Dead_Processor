cmake_minimum_required(VERSION 3.10)
project(Dead_Processor)

set(CMAKE_CXX_STANDARD 17)

set(COMMON_SOURCE utility/FileHelper.cpp source/friday_asm_lang.cpp source/FridayAsmWriter.cpp source/assembler_inside_facade.cpp
        source/ListingGenerator.cpp source/Emulator.cpp)

add_executable(friday-asm source/assembler.cpp source/assembler.hpp ${COMMON_SOURCE})
target_compile_definitions(friday-asm PUBLIC FRIDAY_ASM_MAIN)

add_executable(friday-objdump source/objdump.cpp ${COMMON_SOURCE})
target_compile_definitions(friday-objdump PUBLIC FRIDAY_OBJDUMP_MAIN)

add_executable(friday-emu source/emulate.cpp ${COMMON_SOURCE})
target_compile_definitions(friday-emu PUBLIC FRIDAY_EMU_MAIN)
