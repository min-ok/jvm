#pragma once

#include <cmath>
#include <iostream>

#include "core/errors.h"
#include "core/opcodes.h"
#include "core/program.h"
#include "core/value.h"

constexpr int64_t NUM_REGISTERS = 256;

class VM {
    Value registers[NUM_REGISTERS];

    Program program;

    size_t pc = 0;

    Value &reg(int64_t index) {
        if (index < 0 || index >= NUM_REGISTERS) {
            throw VMError("reg: index out of range", pc - 1,
                          program.code[pc - 1]);
        }

        return registers[index];
    }

    const Value &constant(int64_t index) {
        if (index < 0 ||
            index >= static_cast<int64_t>(program.constants.size())) {
            throw VMError("constant: index out of range", pc - 1,
                          program.code[pc - 1]);
        }

        return program.constants[index];
    }

  public:
    VM(const Program &program_) : program(program_) {}

    bool step() {
        const Instruction &ins = program.code[pc++];

        switch (ins.opcode()) {
        case ADD:
            reg(ins.a()) =
                Value::Int(reg(ins.b()).asInt() + reg(ins.c()).asInt());
            break;
        case SUB:
            reg(ins.a()) =
                Value::Int(reg(ins.b()).asInt() - reg(ins.c()).asInt());
            break;
        case MUL:
            reg(ins.a()) =
                Value::Int(reg(ins.b()).asInt() * reg(ins.c()).asInt());
            break;
        case DIV:
            reg(ins.a()) =
                Value::Int(reg(ins.b()).asInt() / reg(ins.c()).asInt());
            break;
        case MOD:
            reg(ins.a()) =
                Value::Int(reg(ins.b()).asInt() % reg(ins.c()).asInt());
            break;
        case NEG:
            reg(ins.a()) = Value::Int(-reg(ins.b()).asInt());
            break;
        case FADD:
            reg(ins.a()) =
                Value::Float(reg(ins.b()).asFloat() + reg(ins.c()).asFloat());
            break;
        case FSUB:
            reg(ins.a()) =
                Value::Float(reg(ins.b()).asFloat() - reg(ins.c()).asFloat());
            break;
        case FMUL:
            reg(ins.a()) =
                Value::Float(reg(ins.b()).asFloat() * reg(ins.c()).asFloat());
            break;
        case FDIV:
            reg(ins.a()) =
                Value::Float(reg(ins.b()).asFloat() / reg(ins.c()).asFloat());
            break;
        case FMOD:
            reg(ins.a()) = Value::Float(
                std::fmod(reg(ins.b()).asFloat(), reg(ins.c()).asFloat()));
            break;
        case FNEG:
            reg(ins.a()) = Value::Float(-reg(ins.b()).asFloat());
            break;
        case LI:
            reg(ins.a()) = Value::Int(ins.sd());
            break;
        case LK:
            reg(ins.a()) = constant(ins.d());
            break;
        case PR:
            std::cout << reg(ins.a()).toString() << "\n";
            break;
        case STP:
            return false;
        default:
            throw VMError("step: no such opcode", pc - 1, program.code[pc - 1]);
        }

        return true;
    }

    void run() {
        while (pc < program.code.size() && step())
            ;
    }
};
