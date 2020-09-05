#include "m61.hh"
#include <cstdio>
#include <cassert>
#include <cstring>
// Free of invalid pointer after some successful allocations.

int main() {
    void* ptrs[10];
    for (int i = 0; i != 10; ++i) {
        ptrs[i] = malloc(i + 1);
    }
    for (int i = 0; i != 5; ++i) {
        free(ptrs[i]);
    }
    free((void*) 16);
    m61_print_statistics();
}

//! MEMORY BUG???: invalid free of pointer ???, not in heap
//! ???
