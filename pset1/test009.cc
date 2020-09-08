#include "m61.hh"
#include <cstdio>
#include <cassert>
#include <cstring>
#include <cstdlib>
// heap_min and heap_max checking, more allocations.

int main() {
    uintptr_t heap_first = 0;
    uintptr_t heap_last = 0;
    for (int i = 0; i != 100; ++i) {
        size_t sz = rand() % 100;
        char* p = (char*) malloc(sz);
        if (!heap_first || heap_first > (uintptr_t) p) {
            heap_first = (uintptr_t) p;
        }
        if (!heap_last || heap_last < (uintptr_t) p + sz) {
            heap_last = (uintptr_t) p + sz;
        }
        free(p);
    }

    m61_statistics stat;
    m61_get_statistics(&stat);
    assert(stat.heap_min <= heap_first);
    assert(heap_last - 1 <= stat.heap_max);
}
