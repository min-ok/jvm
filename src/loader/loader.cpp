#include "loader.h"

#include <cmath>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace {

using Args = std::vector<std::string>;

struct OpInfo {
    Opcode op;
    size_t argc;
    Instruction (*parse)(Opcode, const Args &, size_t);
};

int64_t parse_int(const std::string &token, size_t line) {
    size_t end = 0;
    try {
        const long long value = std::stoll(token, &end);
        if (end == token.size()) return static_cast<int64_t>(value);
    } catch (const std::exception &) {}
    throw LoadError("invalid integer '" + token + "'", line);
}

double parse_double(const std::string &token, size_t line) {
    size_t end = 0;
    try {
        const double value = std::stod(token, &end);
        if (end == token.size() && std::isfinite(value)) return value;
    } catch (const std::exception &) {}
    throw LoadError("invalid float '" + token + "'", line);
}

int64_t parse_range(const std::string &token, size_t line, int64_t low, int64_t high, const char *what) {
    const int64_t value = parse_int(token, line);
    if (value < low || value > high) {
        throw LoadError(std::string(what) + " out of range: " + token, line);
    }
    return value;
}

uint8_t parse_register(const std::string &token, size_t line) {
    return static_cast<uint8_t>(parse_range(token, line, 0, 28, "register"));
}

int32_t parse_signed21(const std::string &token, size_t line) {
    return static_cast<int32_t>(parse_range(token, line, -(1 << 20), (1 << 20) - 1, "immediate"));
}

uint32_t parse_unsigned21(const std::string &token, size_t line) {
    return static_cast<uint32_t>(parse_range(token, line, 0, (1 << 21) - 1, "immediate"));
}

Instruction parse_regs(Opcode op, const Args &args, size_t line) {
    if (args.size() > 3) throw LoadError("too many register arguments", line);
    uint8_t regs[3] = {0, 0, 0};
    for (size_t i = 0; i < args.size(); ++i) regs[i] = parse_register(args[i], line);
    return Instruction::ABC(op, regs[0], regs[1], regs[2]);
}

Instruction parse_reg_imm_s(Opcode op, const Args &args, size_t line) {
    return Instruction::AD(op, parse_register(args[0], line), parse_signed21(args[1], line));
}

Instruction parse_reg_imm_u(Opcode op, const Args &args, size_t line) {
    return Instruction::AD(op, parse_register(args[0], line), parse_unsigned21(args[1], line));
}

Instruction parse_imm_u(Opcode op, const Args &args, size_t line) {
    return Instruction::AD(op, 0, parse_unsigned21(args[0], line));
}

Instruction parse_lf(Opcode op, const Args &args, size_t line) {
    return Instruction::AD(op, parse_register(args[0], line), parse_signed21(args[1], line));
}

Value parse_constant(const Args &args, size_t line) {
    if (args.size() != 2) throw LoadError("'CONST' expects 2 arguments", line);
    if (args[0] == "i") return Value::Int(parse_int(args[1], line));
    if (args[0] == "f") return Value::Float(parse_double(args[1], line));
    throw LoadError("invalid constant type '" + args[0] + "' (expected 'i' or 'f')", line);
}

const std::unordered_map<std::string, OpInfo> ops = {
    {"STP",    {STP, 0, parse_regs}},

    {"MV",     {MV, 2, parse_regs}},
    {"LI",     {LI,  2, parse_reg_imm_s}},
    {"LF",     {LF,  2, parse_lf}},
    {"LK",     {LK,  2, parse_reg_imm_u}},

    {"JMP",    {JMP, 1, parse_imm_u}},
    {"JZ",     {JZ,  2, parse_reg_imm_u}},
    {"JNZ",    {JNZ, 2, parse_reg_imm_u}},

    {"ADD",    {ADD, 3, parse_regs}},
    {"SUB",    {SUB, 3, parse_regs}},
    {"MUL",    {MUL, 3, parse_regs}},
    {"DIV",    {DIV, 3, parse_regs}},
    {"MOD",    {MOD, 3, parse_regs}},
    {"NEG",    {NEG, 2, parse_regs}},

    {"FADD",   {FADD, 3, parse_regs}},
    {"FSUB",   {FSUB, 3, parse_regs}},
    {"FMUL",   {FMUL, 3, parse_regs}},
    {"FDIV",   {FDIV, 3, parse_regs}},
    {"FMOD",   {FMOD, 3, parse_regs}},
    {"FNEG",   {FNEG, 2, parse_regs}},

    {"I2F",    {I2F, 2, parse_regs}},
    {"F2I",    {F2I, 2, parse_regs}},

    {"EQ",     {EQ,  3, parse_regs}},
    {"LT",     {LT,  3, parse_regs}},
    {"LE",     {LE,  3, parse_regs}},

    {"FEQ",    {FEQ, 3, parse_regs}},
    {"FLT",    {FLT, 3, parse_regs}},
    {"FLE",    {FLE, 3, parse_regs}},

    {"IN",     {IN,  1, parse_regs}},
    {"FIN",    {FIN, 1, parse_regs}},

    {"PR",     {PR,  1, parse_regs}},
    {"FPR",    {FPR,   1, parse_regs}},

    {"ALLOC",  {ALLOC, 2, parse_regs}},
    {"FREE",   {FREE,  1, parse_regs}},
    {"LOAD64", {LOAD64, 2, parse_reg_imm_s}},
    {"STORE64",{STORE64, 2, parse_reg_imm_s}}
};

} // namespace


Program load_program(const std::string &path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw LoadError("cannot open file '" + path + "'", 0);
    }

    Program program;
    std::string line;
    size_t count = 0;

    while (std::getline(file, line)) {
        count++;

        const auto comment = line.find_first_of("#");
        if (comment != std::string::npos) line.resize(comment);

        std::istringstream line_iss(line);
        std::string op_code;
        if (!(line_iss >> op_code)) continue;

        Args args;
        for (std::string arg; line_iss >> arg;)
            args.push_back(std::move(arg));

        if (op_code == "CONST") {
            program.constants.push_back(parse_constant(args, count));
            continue;
        }

        const auto found = ops.find(op_code);
        if (found == ops.end())
            throw LoadError("unknown instruction '" + op_code + "'", count);

        const OpInfo &info = found->second;
        if (args.size() != info.argc)
            throw LoadError("'" + op_code + "' expects " +
                            std::to_string(info.argc) +
                            " argument(s), got " +
                            std::to_string(args.size()),
                            count);

        program.code.push_back(info.parse(info.op, args, count));
    }

    return program;
}