#pragma once

#include <cstdint>
#include <vector>

#include "core/opcodes.h"
#include "core/value.h"

struct Instruction {
    static Instruction ABC(Opcode op, uint8_t a, uint8_t b = 0, uint8_t c = 0) {
        Instruction ins;
        ins.bits = (uint32_t(op) << 24) | (uint32_t(a) << 16) |
                   (uint32_t(b) << 8) | uint32_t(c);
        return ins;
    }

    static Instruction AD(Opcode op, uint8_t a, int32_t d) {
        Instruction ins;
        ins.bits =
            (uint32_t(op) << 24) | (uint32_t(a) << 16) | uint32_t(uint16_t(d));
        return ins;
    }

    Opcode opcode() const { return static_cast<Opcode>((bits >> 24) & 0xFF); }
    uint8_t a() const { return static_cast<uint8_t>((bits >> 16) & 0xFF); }
    uint8_t b() const { return static_cast<uint8_t>((bits >> 8) & 0xFF); }
    uint8_t c() const { return static_cast<uint8_t>(bits & 0xFF); }

    uint16_t d() const { return static_cast<uint16_t>(bits & 0xFFFF); }
    int16_t sd() const { return static_cast<int16_t>(bits & 0xFFFF); }

  private:
    uint32_t bits;
};

struct Program {
    std::vector<Instruction> code;
    std::vector<Value> constants;
};
