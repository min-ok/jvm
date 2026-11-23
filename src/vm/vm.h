#pragma once

#include <cstdint>

#include "core/program.h"
#include "core/value.h"

constexpr int64_t NUM_REGISTERS = 256;

class VM {
    Value registers[NUM_REGISTERS];

    Program program;

    size_t pc = 0;

    Value &reg(int64_t index);
    const Value &constant(int64_t index);

  public:
    explicit VM(const Program &program_);

    bool step();
    void run();
};
