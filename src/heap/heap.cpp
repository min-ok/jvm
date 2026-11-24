#include "heap.h"
#include <cstdint>
#include <cstring>


uint64_t Heap::new_addr(uint64_t size) {
    uint64_t res = next_addr;
    next_addr += size;
    return res;
}

uint64_t Heap::compress() {
    uint64_t offset = 0;
    for(auto block = blocks.begin(); block != blocks.end(); block++) {
        uint8_t flag = block->second.flag;
        uint64_t real_addr = block->second.real_addr;
        uint64_t size = block->second.size;
        if(flag) {
            if(real_addr > offset) {
            std::memmove(memory.data()+offset, memory.data()+real_addr, size);
            block->second.real_addr = offset;
            }
            offset += size;
        }
    }
    return offset;
}

uint64_t Heap::alloc(uint64_t size) {
    if(size == 0) throw HeapError("size must be > 0");
    if(size + offset > memory.size()) offset = compress();
    if(size + offset > memory.size()) throw HeapError("not enough memory");

    uint64_t addr = new_addr(size);
    blocks.emplace(addr, BlockInfo(offset, size));
    offset += size;

    return addr;
}

void Heap::free(uint64_t addr) {
    auto found = blocks.find(addr);
    if (found == blocks.end() || !found->second.flag)
        throw HeapError("incorrect address");
    blocks.erase(found);
    return;
}

int64_t Heap::read64(uint64_t addr) const {
    auto found = blocks.upper_bound(addr);
    if (found == blocks.begin()) throw HeapError("address is not allocated");
    --found;

    if (!found->second.flag) throw HeapError("use after free");

    uint64_t start = found->first;
    uint64_t bsize = found->second.size;
    if (addr - start + 8 > bsize) throw HeapError("access beyond allocated block");

    uint64_t phys = found->second.real_addr + (addr - start);

    int64_t value;
    std::memcpy(&value, memory.data() + phys, 8);
    return value;
}

uint8_t Heap::read8(uint64_t addr) const {
    auto found = blocks.upper_bound(addr);
    if (found == blocks.begin()) throw HeapError("address is not allocated");
    --found;

    if (!found->second.flag) throw HeapError("use after free");

    uint64_t start = found->first;
    uint64_t bsize = found->second.size;
    if (addr - start + 1 > bsize) throw HeapError("access beyond allocated block");

    uint64_t phys = found->second.real_addr + (addr - start);
    return memory[phys];
}

void Heap::write64(uint64_t addr, int64_t value) {
    auto found = blocks.upper_bound(addr);
    if (found == blocks.begin()) throw HeapError("address is not allocated");
    --found;

    if (!found->second.flag) throw HeapError("use after free");

    uint64_t start = found->first;
    uint64_t bsize = found->second.size;
    if (addr - start + 8 > bsize) throw HeapError("access beyond allocated block");

    uint64_t phys = found->second.real_addr + (addr - start);
    std::memcpy(memory.data() + phys, &value, 8);
}

void Heap::write8(uint64_t addr, uint8_t value) {
    auto found = blocks.upper_bound(addr);
    if (found == blocks.begin()) throw HeapError("address is not allocated");
    --found;

    if (!found->second.flag) throw HeapError("use after free");

    uint64_t start = found->first;
    uint64_t bsize = found->second.size;
    if (addr - start + 1 > bsize) throw HeapError("access beyond allocated block");

    uint64_t phys = found->second.real_addr + (addr - start);
    memory[phys] = value;
}