cmake_minimum_required(VERSION 3.10)
project(Dead_Processor)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} -g)

set(COMMON_SOURCE source/utility/FileHelper.cpp source/friday_asm_lang.cpp source/FridayAsmWriter.cpp
        source/assembler_inside_facade.cpp source/ListingGenerator.cpp source/Emulator.cpp)
add_library(friday-shared STATIC ${COMMON_SOURCE})

add_executable(friday-asm source/assembler.cpp)
target_compile_definitions(friday-asm PUBLIC FRIDAY_ASM_MAIN)
target_link_libraries(friday-asm friday-shared)

add_executable(friday-objdump source/objdump.cpp)
target_compile_definitions(friday-objdump PUBLIC FRIDAY_OBJDUMP_MAIN)
target_link_libraries(friday-objdump friday-shared)

add_executable(friday-emu source/emulate.cpp)
target_compile_definitions(friday-emu PUBLIC FRIDAY_EMU_MAIN)
target_link_libraries(friday-emu friday-shared)
