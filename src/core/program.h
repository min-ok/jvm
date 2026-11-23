#pragma once

#include <vector>

#include "core/instruction.h"
#include "core/value.h"

struct Program {
    std::vector<Instruction> code;
    std::vector<Value> constants;
};
