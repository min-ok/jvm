#pragma once

#include <cstddef>
#include <string>
#include <vector>
#include "core/program.h"

struct VerifyError {
    size_t pc;
    std::string message;
};

enum class Format {
    Abc,
    Ad
};

enum class DCheck {
    None,
    Constant,
    Jump
};

struct OpInfo {
    Format format;
    bool use_a;
    bool use_b;
    bool use_c;
    DCheck d;
    bool terminal;
};

class Verifier {
public:
    explicit Verifier(const Program &program): program(program) {};

    std::vector<VerifyError> verify();
private:
    const Program& program;
    std::vector<VerifyError> errors;

    void error(size_t pc, const std::string& message);
    void check_program();
    void check_instruction(size_t pc, const Instruction& ins);

    void check_unused_fields(size_t pc, const Instruction& ins, const OpInfo& info);
    void check_d_field(size_t pc, const Instruction& ins, const OpInfo& info);
};
