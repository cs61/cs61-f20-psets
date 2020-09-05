#include "m61.hh"
#include <cstdio>
#include <cassert>
#include <cstring>
// Wild free inside heap-allocated data.

struct whatever {
    int first[100];
    char second[3000];
    int third[200];
};

int main(int argc, char** argv) {
    m61_allocator<whatever> allocator;
    // “allocate space for one `whatever` object”
    whatever* object = allocator.allocate(1);

    uintptr_t addr = reinterpret_cast<uintptr_t>(object);
    if (argc < 2) {
        addr += 3000;
    } else {
        addr += strtol(argv[1], nullptr, 0);
    }

    whatever* trick = reinterpret_cast<whatever*>(addr);
    allocator.deallocate(trick, 1);
    m61_print_statistics();
}

//! MEMORY BUG???: invalid free of pointer ???, not allocated
//! ???
