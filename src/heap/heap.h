#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <stdexcept>
#include <vector>



class HeapError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};


class BlockInfo {
public:
    uint64_t real_addr;
    uint64_t size;
    uint8_t flag;
    BlockInfo(uint64_t real_addr, uint64_t size, uint8_t flag = 1)
        : real_addr(real_addr), size(size), flag(flag) {};
};


class Heap {
private:
        std::vector<uint8_t> memory;
        std::map<uint64_t, BlockInfo> blocks;
        uint64_t next_addr = 1;
        uint64_t offset = 0;

        uint64_t new_addr(uint64_t size);
        uint64_t compress();
public:
    explicit Heap(uint64_t size) : memory(size, 0) {
            if(size == 0) throw HeapError("incorrect size");
    };

    Heap(const Heap&) = delete;
    Heap& operator=(const Heap&) = delete;

    uint64_t alloc(uint64_t size);
    void free(uint64_t addr);

    int64_t read64(uint64_t addr) const;
    uint8_t read8(uint64_t addr) const;
    void write64(uint64_t addr, int64_t value);
    void write8(uint64_t addr, uint8_t value);
};