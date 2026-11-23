#include "vm/vm.h"

#include <cmath>
#include <iostream>

#include "core/errors.h"

VM::VM(const Program &program_) : program(program_) {}

Value &VM::reg(int64_t index) {
    if (index < 0 || index >= NUM_REGISTERS) {
        throw VMError("reg: index out of range", pc - 1, program.code[pc - 1]);
    }

    return registers[index];
}

const Value &VM::constant(int64_t index) {
    if (index < 0 ||
        index >= static_cast<int64_t>(program.constants.size())) {
        throw VMError("constant: index out of range", pc - 1,
                      program.code[pc - 1]);
    }

    return program.constants[index];
}

bool VM::step() {
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

void VM::run() {
    while (pc < program.code.size() && step())
        ;
}
