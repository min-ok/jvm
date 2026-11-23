#include "core/vm.h"

int main() {
    std::vector<Instruction> ins;

    Program prg;

    ins.push_back(Instruction::AD(LI, 1, 20));
    ins.push_back(Instruction::AD(LI, 2, 6));

    ins.push_back(Instruction::ABC(ADD, 3, 1, 2));
    ins.push_back(Instruction::ABC(SUB, 4, 1, 2));
    ins.push_back(Instruction::ABC(MUL, 5, 1, 2));
    ins.push_back(Instruction::ABC(DIV, 6, 1, 2));
    ins.push_back(Instruction::ABC(MOD, 7, 1, 2));
    ins.push_back(Instruction::ABC(NEG, 8, 1));

    ins.push_back(Instruction::ABC(PR, 3));
    ins.push_back(Instruction::ABC(PR, 4));
    ins.push_back(Instruction::ABC(PR, 5));
    ins.push_back(Instruction::ABC(PR, 6));
    ins.push_back(Instruction::ABC(PR, 7));
    ins.push_back(Instruction::ABC(PR, 8));

    prg.code = ins;

    VM vm(prg);

    vm.run();

    return 0;
}
