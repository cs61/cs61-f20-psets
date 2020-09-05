#include "m61.hh"
#include <cstdio>
#include <cassert>
#include <cstring>
#include <cstdlib>
// heap_min and heap_max checking, more allocations.

int main() {
    uintptr_t heap_min = 0;
    uintptr_t heap_max = 0;
    for (int i = 0; i != 100; ++i) {
        size_t sz = rand() % 100;
        char* p = (char*) malloc(sz);
        if (!heap_min || heap_min > (uintptr_t) p) {
            heap_min = (uintptr_t) p;
        }
        if (!heap_max || heap_max < (uintptr_t) p + sz) {
            heap_max = (uintptr_t) p + sz;
        }
        free(p);
    }

    m61_statistics stat;
    m61_get_statistics(&stat);
    assert(heap_min >= stat.heap_min);
    assert(heap_max <= stat.heap_max);
}
