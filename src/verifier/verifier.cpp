#include "verifier.h"
#include "core/opcodes.h"
#include <string>
#include <unordered_map>

namespace {

const std::unordered_map<Opcode, OpInfo> op_table = {
    {STP,  {Format::Abc, false, false, false, DCheck::None,     true }},

    {MV,   {Format::Abc, true,  true,  false, DCheck::None,     false}},
    {LI,   {Format::Ad,  true,  false, false, DCheck::None,     false}},
    {LF,   {Format::Ad,  true,  false, false, DCheck::None,     false}},
    {LK,   {Format::Ad,  true,  false, false, DCheck::Constant, false}},

    {JMP,  {Format::Ad,  false, false, false, DCheck::Jump,     true }},
    {JZ,   {Format::Ad,  true,  false, false, DCheck::Jump,     false}},
    {JNZ,  {Format::Ad,  true,  false, false, DCheck::Jump,     false}},

    {ADD,  {Format::Abc, true,  true,  true,  DCheck::None,     false}},
    {SUB,  {Format::Abc, true,  true,  true,  DCheck::None,     false}},
    {MUL,  {Format::Abc, true,  true,  true,  DCheck::None,     false}},
    {DIV,  {Format::Abc, true,  true,  true,  DCheck::None,     false}},
    {MOD,  {Format::Abc, true,  true,  true,  DCheck::None,     false}},
    {NEG,  {Format::Abc, true,  true,  false, DCheck::None,     false}},

    {FADD, {Format::Abc, true,  true,  true,  DCheck::None,     false}},
    {FSUB, {Format::Abc, true,  true,  true,  DCheck::None,     false}},
    {FMUL, {Format::Abc, true,  true,  true,  DCheck::None,     false}},
    {FDIV, {Format::Abc, true,  true,  true,  DCheck::None,     false}},
    {FNEG, {Format::Abc, true,  true,  false, DCheck::None,     false}},

    {I2F,  {Format::Abc, true,  true,  false, DCheck::None,     false}},
    {F2I,  {Format::Abc, true,  true,  false, DCheck::None,     false}},

    {EQ,   {Format::Abc, true,  true,  true,  DCheck::None,     false}},
    {LT,   {Format::Abc, true,  true,  true,  DCheck::None,     false}},
    {LE,   {Format::Abc, true,  true,  true,  DCheck::None,     false}},

    {FEQ,  {Format::Abc, true,  true,  true,  DCheck::None,     false}},
    {FLT,  {Format::Abc, true,  true,  true,  DCheck::None,     false}},
    {FLE,  {Format::Abc, true,  true,  true,  DCheck::None,     false}},

    {IN,   {Format::Abc, true,  false, false, DCheck::None,     false}},
    {FIN,  {Format::Abc, true,  false, false, DCheck::None,     false}},
    {PR,   {Format::Abc, true,  false, false, DCheck::None,     false}},

    {CALL, {Format::Ad,  true,  false, false, DCheck::Jump,     false}},
    {RET,  {Format::Abc, true,  false, false, DCheck::None,     true }},
};

const OpInfo* instruction_info(Opcode op) {
    auto it = op_table.find(op);
    return it == op_table.end() ? nullptr : &it->second;
}

} // namespace

void Verifier::error(size_t pc, const std::string& message) {
    errors.push_back({pc, message});
    return;
}

std::vector<VerifyError> Verifier::verify() {
    errors.clear();
    for (size_t pc = 0; pc < program.code.size(); ++pc) 
        check_instruction(pc, program.code[pc]);
    check_program();
    return errors;
}

void Verifier::check_unused_fields(size_t pc, const Instruction& ins, const OpInfo& info) {
    if (!info.use_a && ins.a() != 0) error(pc, "field 'a' must be 0");
    if (info.format != Format::Abc) return;
    if (!info.use_b && ins.b() != 0) error(pc, "field 'b' must be 0");
    if (!info.use_c && ins.c() != 0) error(pc, "field 'c' must be 0");
    return;
}

void Verifier::check_d_field(size_t pc, const Instruction& ins, const OpInfo& info) {
    if (info.d == DCheck::Constant && ins.d() >= program.constants.size())
        error(pc, "there is no constant with that number");
    if (info.d == DCheck::Jump) {
        if (ins.d() >= program.code.size()) error(pc, "jump target out of range");
        if (ins.d() == pc) error(pc, "infinite loop");
    }
    return;
}

void Verifier::check_instruction(size_t pc, const Instruction& ins) {
    const OpInfo* info = instruction_info(ins.opcode());
    if (!info) {
        error(pc, "unknown opcode");
        return;
    }
    check_unused_fields(pc, ins, *info);
    check_d_field(pc, ins, *info);
    return;
}

void Verifier::check_program() {
    if (program.code.empty()) {
        error(0, "program is empty");
        return;
    }
    bool has_stp = false;
    for (const Instruction& ins : program.code) {
        if (ins.opcode() == STP) {
            has_stp = true;
            break;
        }
    }
    if (!has_stp) error(0, "program has no STP instruction");
    const Instruction& last = program.code.back();
    const OpInfo* info = instruction_info(last.opcode());
    if (!info || !info->terminal)
        error(program.code.size() - 1, "last instruction must be STP/RET/JMP/JZ/JNZ");
}
