#pragma once

#include <cstddef>
#include <cstdint>
#include <map>
#include <stdexcept>
#include <vector>



class HeapError : public std::runtime_error {
    using std::runtime_error::runtime_error;
};


class Heap {
private:
        explicit Heap(uint64_t size) : memory(size, 0) {
            if(size == 0) throw HeapError("incorrect size");
        };
    
        std::vector<uint8_t> memory;
        std::map<uint64_t, uint64_t> blocks;
public:
    static Heap& get_heap(uint64_t size) {
        static Heap heap = Heap(size);
        return heap;
    };

    Heap(const Heap&) = delete;
    Heap& operator=(const Heap&) = delete;
    Heap(Heap&&) = delete;
    Heap& operator=(Heap&&) = delete;

    uint64_t alloc(uint64_t size);
    void dealloc(uint64_t addr);

    int64_t read64(uint64_t addr) const;
    uint8_t read8(uint64_t addr) const;
    void write64(uint64_t addr, int64_t value);
    void write8(uint64_t addr, uint8_t value);
};