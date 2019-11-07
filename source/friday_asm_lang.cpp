#include "friday_asm_lang.hpp"
#include "Emulator.hpp"
#include "utility/BytesHelper.hpp"
#include <cstdio>
#ifndef NDEBUG
    #include <cassert>
    #include <cstring>
#include <cmath>

#endif

namespace FridayArch {

std::vector<Instruction> INSTRUCTION_SET;
std::vector<friday_inst_t> MAP_OF_INSTRUCTIONS_BY_BYTECODE(MAX_INSTRUCTION_VALUE + 1, -1);

bool AreInstructionArgsEqual(unsigned int args_count, const InstructionArgument *array1,
                             const InstructionArgument *array2) {
    for (int i = static_cast<int>(args_count) - 1; i >= 0; --i) {
        if (array1[i] != array2[i]) {
            return false;
        }
    }
    return true;
}

char RegisterInstruction(const char *name, friday_inst_t inst, int args_count, InstructionArgument *args,
        void (*callback)(Emulator*)) {
#ifndef NDEBUG
    for (auto &inst_ : INSTRUCTION_SET) {
        assert(inst != inst_.inst);
        assert(strcmp(name, inst_.name) != 0 || args_count != inst_.args_count ||
               !AreInstructionArgsEqual(args_count, args, inst_.args));
    }
#endif

    INSTRUCTION_SET.emplace_back(name, inst, args_count, args, callback);
    MAP_OF_INSTRUCTIONS_BY_BYTECODE[inst] = INSTRUCTION_SET.size() - 1;
    return '\0';
}

Instruction *FindInstructionBySignature(const std::string_view &name, int args_count,
                                                                const InstructionArgument *args) {
    for (auto &inst : INSTRUCTION_SET) {
        if (name == inst.name && args_count == inst.args_count &&
            AreInstructionArgsEqual(args_count, args, inst.args)) {
            return &inst;
        }
    }

    return nullptr;
}

const char *GetInstructionArgumentName(InstructionArgument value) {
    switch (value) {
        case CONSTANT: return "CONSTANT";
        case REGISTER: return "REGISTER";
        case LABEL: return "LABEL";
        case _BAD_ARG: return "_BAD_ARG";
    }
    return "what the hell is this InstructionArgument";
}

size_t GetInstructionArgumentSize(InstructionArgument value) {
    switch (value) {
        case CONSTANT: return sizeof(friday_constant_t);
        case REGISTER: return sizeof(friday_reg_t);
        case LABEL: return sizeof(friday_address_t);
        case _BAD_ARG: return -1;
    }
    return -1;
}

Instruction *GetInstructionByBytecode(friday_inst_t bytecode) {
    int index = MAP_OF_INSTRUCTIONS_BY_BYTECODE[bytecode];
    if (index == -1) {
        return nullptr;
    }
    return &INSTRUCTION_SET[index];
}

bool CheckForFRDY(const char *text) {
    for (int i = 0; i < HEADER_ASM_VER_OFFSET; ++i) {
        if (FRDY[i] != text[i]) {
            return false;
        }
    }
    return true;
}

size_t CalculateInstructionFullSize(int args_count, const InstructionArgument *args) noexcept {
    size_t result = sizeof(friday_inst_t);
    for (int i = 0; i < args_count; ++i) {
        result += GetInstructionArgumentSize(args[i]);
    }
    return result;
}

Instruction::Instruction(const char *name, friday_inst_t instruction, int args_count,
                     const InstructionArgument *args, void (*callback)(Emulator*)) :
    name(name),
    inst(instruction),
    args_count(args_count),
    args(args),
    inst_full_size(CalculateInstructionFullSize(args_count, args)),
    callback(callback)
{}


//**  MACROS FOR INSTRUCTIONS  **//
//#################################################################################################
#define FRIDAY_INST_CLASS_NAME(name, inst) __Instruction##_##name##_##inst
#define FRIDAY_INST(name, inst, args)                                                                        \
class FRIDAY_INST_CLASS_NAME(name, inst) {                                                                   \
    FRIDAY_INST_CLASS_NAME(name, inst)() = default; /* Private constructor */                                \
public:                                                                                                      \
    static void Execute(Emulator*);                                                                          \
    static char __register_instruction __attribute__ ((unused));                                             \
};                                                                                                           \
char FRIDAY_INST_CLASS_NAME(name, inst)::__register_instruction = RegisterInstruction(                       \
        #name /* name */, inst /* instruction */,                                                            \
        sizeof((InstructionArgument[]) args) / sizeof(InstructionArgument) /* args_count */,                 \
        (InstructionArgument[]) args /* args */,                                                             \
        FRIDAY_INST_CLASS_NAME(name, inst)::Execute /* callback */ );                                        \
void FRIDAY_INST_CLASS_NAME(name, inst)::Execute(Emulator* emu) /* now define callback */
//#################################################################################################


//**  FRIDAY ASM INSTRUCTIONS  **//
//#################################################################################################
using namespace BytesHelper;
inline void InstDepart(Emulator* emu);
inline void InstUnconditionalJump(Emulator* emu);
template <typename U, typename T>
inline void InstConditionalJump(Emulator* emu, T condition);
template <typename U, typename T>
inline void InstArithmetics(Emulator* emu, T operation);
//------------------------------------------------------------------------------------
FRIDAY_INST(end,  0x00, {})            { emu->signal = Emulator::SIGNAL_EXIT; }
FRIDAY_INST(push, 0x01, { CONSTANT })  { emu->push(emu->get_arg_ptr(), 4); }
FRIDAY_INST(push, 0x02, { REGISTER })  { int32_t value = emu->regs[BytesAs<uint8_t>(emu->get_arg_ptr())];
                                         emu->push(AsBytes(value), 4); }
FRIDAY_INST(pop,  0x03, { REGISTER })  { emu->regs[BytesAs<uint8_t>(emu->get_arg_ptr())] = emu->pop_int();
                                         /* either would work with float */ }
FRIDAY_INST(in,   0x04, {})            { emu->sp -= 4; scanf("%d", &BytesAs<int>(emu->get_stack_ptr())); }
FRIDAY_INST(out,  0x05, {})            { printf("%d\n", emu->pop_int()); }
FRIDAY_INST(outf, 0x06, {})            { printf("%g\n", emu->pop_float()); }
// dep (fully: depart) = push ip
FRIDAY_INST(dep,  0x07, {})            { InstDepart(emu); }
// call = push ip && jmp LABEL
FRIDAY_INST(call, 0x08, { LABEL })     { InstDepart(emu); InstUnconditionalJump(emu); }
// ret (fully: return) = pop ip
FRIDAY_INST(ret,  0x09, {})            { emu->ip = emu->pop_int(); }
// ci2f (full convert integer to float) = pop integer && push float of the same value
FRIDAY_INST(ci2f, 0x0a, {})           { emu->push(AsBytes(static_cast<float>(emu->pop_int())), sizeof(float)); }
// ci2f (full convert float to integer) = pop float && push integer of the same value
FRIDAY_INST(cf2i, 0x0b, {})           { emu->push(AsBytes(static_cast<int>(emu->pop_float())), sizeof(int)); }
FRIDAY_INST(in_f, 0x0c, {})           { emu->sp -= 4; scanf("%f", &BytesAs<float>(emu->get_stack_ptr())); }


FRIDAY_INST(jmp,  0x10, { LABEL })    { InstUnconditionalJump(emu); }
FRIDAY_INST(ja,   0x11, { LABEL })    { InstConditionalJump<int>(emu, [] (int a, int b) -> bool { return a >  b; }); }
FRIDAY_INST(jae,  0x12, { LABEL })    { InstConditionalJump<int>(emu, [] (int a, int b) -> bool { return a >= b; }); }
FRIDAY_INST(jb,   0x13, { LABEL })    { InstConditionalJump<int>(emu, [] (int a, int b) -> bool { return a <  b; }); }
FRIDAY_INST(jbe,  0x14, { LABEL })    { InstConditionalJump<int>(emu, [] (int a, int b) -> bool { return a <= b; }); }
FRIDAY_INST(je,   0x15, { LABEL })    { InstConditionalJump<int>(emu, [] (int a, int b) -> bool { return a == b; }); }
FRIDAY_INST(jne,  0x16, { LABEL })    { InstConditionalJump<int>(emu, [] (int a, int b) -> bool { return a != b; }); }
FRIDAY_INST(jaf,  0x1a, { LABEL })    { InstConditionalJump<float>(emu, [] (float a, float b) -> bool { return a >  b; }); }
FRIDAY_INST(jaef, 0x1b, { LABEL })    { InstConditionalJump<float>(emu, [] (float a, float b) -> bool { return a >= b; }); }
FRIDAY_INST(jbf,  0x1c, { LABEL })    { InstConditionalJump<float>(emu, [] (float a, float b) -> bool { return a <  b; }); }
FRIDAY_INST(jbef, 0x1d, { LABEL })    { InstConditionalJump<float>(emu, [] (float a, float b) -> bool { return a <= b; }); }
FRIDAY_INST(jef,  0x1e, { LABEL })    { InstConditionalJump<float>(emu, [] (float a, float b) -> bool { return a == b; }); }
FRIDAY_INST(jnef, 0x1f, { LABEL })    { InstConditionalJump<float>(emu, [] (float a, float b) -> bool { return a != b; }); }


FRIDAY_INST(add,  0x20, {})           { InstArithmetics<int>(emu, [] (int a, int b) -> int { return a + b; }); }
FRIDAY_INST(sub,  0x21, {})           { InstArithmetics<int>(emu, [] (int a, int b) -> int { return a - b; }); }
FRIDAY_INST(mul,  0x22, {})           { InstArithmetics<int>(emu, [] (int a, int b) -> int { return a * b; }); }
FRIDAY_INST(div,  0x23, {})           { InstArithmetics<int>(emu, [] (int a, int b) -> int { return a / b; }); }
FRIDAY_INST(mod,  0x24, {})           { InstArithmetics<int>(emu, [] (int a, int b) -> int { return a % b; }); }
FRIDAY_INST(addf, 0x2a, {})           { InstArithmetics<float>(emu, [] (float a, float b) -> float { return a + b; }); }
FRIDAY_INST(subf, 0x2b, {})           { InstArithmetics<float>(emu, [] (float a, float b) -> float { return a - b; }); }
FRIDAY_INST(mulf, 0x2c, {})           { InstArithmetics<float>(emu, [] (float a, float b) -> float { return a * b; }); }
FRIDAY_INST(divf, 0x2d, {})           { InstArithmetics<float>(emu, [] (float a, float b) -> float { return a / b; }); }
FRIDAY_INST(sqrt, 0x2e, {})           { emu->push(AsBytes(static_cast<float>(sqrt(emu->pop_float()))), sizeof(friday_constant_t)); }
//------------------------------------------------------------------------------------
inline void InstDepart(Emulator* emu) {
    emu->push(AsBytes(emu->ip), sizeof(emu->ip));
}
inline void InstUnconditionalJump(Emulator* emu) {
    emu->ip = BytesAs<friday_address_t>(emu->get_arg_ptr());
}
template <typename U, typename T>
inline void InstConditionalJump(Emulator* emu, T condition) {
    U op2 = BytesAs<U>(emu->pop(sizeof(friday_constant_t)));
    U op1 = BytesAs<U>(emu->pop(sizeof(friday_constant_t)));
    if (condition(op1, op2)) {
        emu->ip = BytesAs<friday_address_t>(emu->get_arg_ptr());
    }
}
template <typename U, typename T>
inline void InstArithmetics(Emulator* emu, T operation) {
    const char* op2 = emu->pop(sizeof(friday_constant_t));
    const char* op1 = emu->pop(sizeof(friday_constant_t));
    U result = operation(BytesAs<U>(op1), BytesAs<U>(op2));
    emu->push(AsBytes(result), sizeof(friday_constant_t));
}
//#################################################################################################

}
