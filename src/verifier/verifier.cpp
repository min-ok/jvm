#include "verifier.h"
#include "core/opcodes.h"
#include "core/program.h"
#include <cstddef>
#include <unordered_map>
#include <vector>

namespace {

const size_t MAX_REGISTER = 28;

const std::unordered_map<Opcode, OpInfo> op_table = {
    {STP,  {Format::Abc, false, false, false, DCheck::None,     ControlFlow::Stop       }},

    {MV,   {Format::Abc, true,  true,  false, DCheck::None,     ControlFlow::Normal     }},
    {LI,   {Format::Ad,  true,  false, false, DCheck::None,     ControlFlow::Normal     }},
    {LF,   {Format::Ad,  true,  false, false, DCheck::None,     ControlFlow::Normal     }},
    {LK,   {Format::Ad,  true,  false, false, DCheck::Constant, ControlFlow::Normal     }},

    {JMP,  {Format::Ad,  false, false, false, DCheck::Jump,     ControlFlow::Jump       }},
    {JZ,   {Format::Ad,  true,  false, false, DCheck::Jump,     ControlFlow::Conditional}},
    {JNZ,  {Format::Ad,  true,  false, false, DCheck::Jump,     ControlFlow::Conditional}},

    {ADD,  {Format::Abc, true,  true,  true,  DCheck::None,     ControlFlow::Normal     }},
    {SUB,  {Format::Abc, true,  true,  true,  DCheck::None,     ControlFlow::Normal     }},
    {MUL,  {Format::Abc, true,  true,  true,  DCheck::None,     ControlFlow::Normal     }},
    {DIV,  {Format::Abc, true,  true,  true,  DCheck::None,     ControlFlow::Normal     }},
    {MOD,  {Format::Abc, true,  true,  true,  DCheck::None,     ControlFlow::Normal     }},
    {NEG,  {Format::Abc, true,  true,  false, DCheck::None,     ControlFlow::Normal     }},

    {FADD, {Format::Abc, true,  true,  true,  DCheck::None,     ControlFlow::Normal     }},
    {FSUB, {Format::Abc, true,  true,  true,  DCheck::None,     ControlFlow::Normal     }},
    {FMUL, {Format::Abc, true,  true,  true,  DCheck::None,     ControlFlow::Normal     }},
    {FDIV, {Format::Abc, true,  true,  true,  DCheck::None,     ControlFlow::Normal     }},
    {FNEG, {Format::Abc, true,  true,  false, DCheck::None,     ControlFlow::Normal     }},

    {I2F,  {Format::Abc, true,  true,  false, DCheck::None,     ControlFlow::Normal     }},
    {F2I,  {Format::Abc, true,  true,  false, DCheck::None,     ControlFlow::Normal     }},

    {EQ,   {Format::Abc, true,  true,  true,  DCheck::None,     ControlFlow::Normal     }},
    {LT,   {Format::Abc, true,  true,  true,  DCheck::None,     ControlFlow::Normal     }},
    {LE,   {Format::Abc, true,  true,  true,  DCheck::None,     ControlFlow::Normal     }},

    {FEQ,  {Format::Abc, true,  true,  true,  DCheck::None,     ControlFlow::Normal     }},
    {FLT,  {Format::Abc, true,  true,  true,  DCheck::None,     ControlFlow::Normal     }},
    {FLE,  {Format::Abc, true,  true,  true,  DCheck::None,     ControlFlow::Normal     }},

    {IN,   {Format::Abc, true,  false, false, DCheck::None,     ControlFlow::Normal     }},
    {FIN,  {Format::Abc, true,  false, false, DCheck::None,     ControlFlow::Normal     }},
    {PR,   {Format::Abc, true,  false, false, DCheck::None,     ControlFlow::Normal     }},

    {CALL, {Format::Ad,  true,  false, false, DCheck::Jump,     ControlFlow::Call       }},
    {RET,  {Format::Abc, true,  false, false, DCheck::None,     ControlFlow::Return     }},

    // добавлю когда узнаю формат комманд
    // {ALLOC,   {Format::Abc, true,  true,  false, DCheck::None, ControlFlow::Normal}},
    // {FREE,    {Format::Abc, true,  false, false, DCheck::None, ControlFlow::Normal}},
    // {LOAD64,  {Format::Ad,  true,  false, false, DCheck::None, ControlFlow::Normal}},
    // {STORE64, {Format::Ad,  true,  false, false, DCheck::None, ControlFlow::Normal}},
};

const OpInfo* instruction_info(Opcode op) {
    auto it = op_table.find(op);
    return it == op_table.end() ? nullptr : &it->second;
}

std::vector<size_t> next_instructions(size_t pc, const Instruction& ins, const OpInfo& info) {
    std::vector<size_t> res;
    switch(info.flow) {
        case ControlFlow::Normal: res.push_back(pc + 1); break;
        case ControlFlow::Jump: res.push_back(ins.d()); break;
        case ControlFlow::Conditional: res.push_back(ins.d()); res.push_back(pc + 1); break;
        case ControlFlow::Call: res.push_back(ins.d()); res.push_back(pc + 1); break;
        case ControlFlow::Return: break;
        case ControlFlow::Stop: break;
    }
    return res;
}

bool is_go_forward(ControlFlow cf) {
    if(cf == ControlFlow::Normal || cf == ControlFlow::Conditional || cf == ControlFlow::Call)
        return true;
    return false;
}

std::vector<std::vector<size_t>> build_graph(const Program& program) {
    const size_t program_size = program.code.size();
    std::vector<std::vector<size_t>> graph(program_size);

    for(size_t pc = 0; pc < program_size; pc++) {
        const Instruction& ins = program.code[pc];
        const OpInfo* info = instruction_info(ins.opcode());
        if(!info) continue;

        for(size_t next : next_instructions(pc, ins, *info)) {
            if(next >= program_size) continue;
            graph[pc].push_back(next);
        }
    }
    return graph;
}

std::vector<std::vector<size_t>> build_reverse_graph(std::vector<std::vector<size_t>>& graph) {
    std::vector<std::vector<size_t>> reverse_graph(graph.size());
    for (size_t pc = 0; pc < graph.size(); pc++)
        for (size_t next : graph[pc])
            reverse_graph[next].push_back(pc);
    return reverse_graph;
}

bool can_terminate_program(ControlFlow cf) {
    if(cf == ControlFlow::Stop) return true;
    return false;
}

} // namespace

void Verifier::error(size_t pc, const std::string& message) {
    errors.push_back({pc, Severity::Error, message});
    return;
}

void Verifier::warning(size_t pc, const std::string& message) {
    errors.push_back({pc, Severity::Warning, message});
    return;
}


std::vector<VerifyError> Verifier::verify() {
    errors.clear();
    for (size_t pc = 0; pc < program.code.size(); pc++) 
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
    if (info.d == DCheck::Jump && ins.d() >= program.code.size())
        error(pc, "jump target out of range");
    return;
}

void Verifier::check_registers(size_t pc, const Instruction& ins, const OpInfo& info) {
    if(info.use_a && ins.a() > MAX_REGISTER)
        error(pc, "register 'a' must be in range [0; 28]");
    if(info.format == Format::Ad) return;
    if(info.use_b && ins.b() > MAX_REGISTER)
        error(pc, "register 'b' must be in range [0; 28]");
    if(info.use_c && ins.c() > MAX_REGISTER)
        error(pc, "register 'c' must be in range [0; 28]");
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
    check_registers(pc, ins, *info);
    return;
}

void Verifier::check_program_graph() {
    const size_t program_size = program.code.size();
    std::vector<std::vector<size_t>> graph = build_graph(program);
    std::vector<bool> visited(program_size, 0);
    std::vector<size_t> to_check = {0};

    while(!to_check.empty()) {
        size_t pc = to_check.back();
        to_check.pop_back();

        if(visited[pc]) continue;
        visited[pc] = true;

        const Instruction& ins = program.code[pc];
        const OpInfo* info = instruction_info(ins.opcode());
        if(!info) continue;

        if(pc + 1 >= program_size && is_go_forward(info->flow))
            error(pc, "execution can go past the end");
        for(size_t next : graph[pc]) if(!visited[next]) to_check.push_back(next);
    }
    for (size_t pc = 0; pc < program_size; pc++)
        if (!visited[pc]) warning(pc, "instruction is unreachable");

    std::vector<std::vector<size_t>> reverse_graph = build_reverse_graph(graph);
    std::vector<bool> can_terminate(program_size, false);
    to_check.clear();

    for (size_t pc = 0; pc < program_size; pc++) {
        const OpInfo* info = instruction_info(program.code[pc].opcode());
        if(info && visited[pc] && can_terminate_program(info->flow)) {
            can_terminate[pc] = true;
            to_check.push_back(pc);
        }
    }
    while(!to_check.empty()) {
        size_t pc = to_check.back();
        to_check.pop_back();
        for(size_t from : reverse_graph[pc]) {
            if(can_terminate[from]) continue;
            can_terminate[from] = true;
            to_check.push_back(from);
        }
    }
    if(!can_terminate[0]) error(0, "program has no path to termination");
    return;
}

void Verifier::check_program() {
    if (program.code.empty()) {
        error(0, "program is empty");
        return;
    }
    check_program_graph();
    return;
}
