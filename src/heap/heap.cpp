#include "heap.h"
#include <cstdint>
#include <cstring>


uint64_t Heap::alloc(uint64_t size) {
    if(size == 0) throw HeapError("size must be > 0");

    uint64_t candidate = 1;

    for(auto block = blocks.begin(); block != blocks.end(); block++) {
        uint64_t gap = block->first - candidate;
        if(gap >= size) {
            blocks[candidate] = size;
            return candidate;
        }
        candidate = block->first + block->second;
    }

    if(candidate + size > memory.size()) throw HeapError("not enough memory");
    blocks[candidate] = size;
    return candidate;
}

void Heap::dealloc(uint64_t addr) {
    auto found = blocks.find(addr);
    if(found == blocks.end()) throw HeapError("incorrect address");
    blocks.erase(found);
    return;
}

int64_t Heap::read64(uint64_t addr) const {
    if(addr >= memory.size() || 8 > memory.size() - addr) throw HeapError("incorrect address");

    auto found = blocks.upper_bound(static_cast<std::size_t>(addr));
    if(found == blocks.begin()) throw HeapError("address is not allocated");
    found--;

    uint64_t start = found->first;
    uint64_t bsize = found->second;
    if(addr - start + 8 > bsize) throw HeapError("access beyond allocated block");

    int64_t value;
    std::memcpy(&value, memory.data() + addr, 8);
    return value;
}

uint8_t Heap::read8(uint64_t addr) const {
    if(addr >= memory.size() || 1 > memory.size() - addr) throw HeapError("incorrect address");

    auto found = blocks.upper_bound(static_cast<std::size_t>(addr));
    if(found == blocks.begin()) throw HeapError("address is not allocated");
    found--;

    uint64_t start = found->first;
    uint64_t bsize = found->second;
    if(addr - start + 1 > bsize) throw HeapError("access beyond allocated block");

    uint8_t value;
    std::memcpy(&value, memory.data() + addr, 1);
    return value;
}

void Heap::write64(uint64_t addr, int64_t value) {
    if (addr >= memory.size() || 8 > memory.size() - addr) throw HeapError("incorrect address");

    auto found = blocks.upper_bound(static_cast<std::size_t>(addr));
    if (found == blocks.begin()) throw HeapError("address is not allocated");
    found--;

    uint64_t start = found->first;
    uint64_t bsize = found->second;
    if(addr - start + 8 > bsize) throw HeapError("access beyond allocated block");

    std::memcpy(memory.data() + addr, &value, 8);
    return;
}

void Heap::write8(uint64_t addr, uint8_t value) {
    if (addr >= memory.size() || 1 > memory.size() - addr) throw HeapError("incorrect address");

    auto found = blocks.upper_bound(static_cast<std::size_t>(addr));
    if (found == blocks.begin()) throw HeapError("address is not allocated");
    found--;

    uint64_t start = found->first;
    uint64_t bsize = found->second;
    if(addr - start + 1 > bsize) throw HeapError("access beyond allocated block");

    std::memcpy(memory.data() + addr, &value, 1);
    return;
}