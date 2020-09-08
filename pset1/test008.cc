#include "m61.hh"
#include <cstdio>
#include <cassert>
#include <cstring>
// heap_min and heap_max checking, simple case.

int main() {
    char* p = (char*) malloc(10);

    m61_statistics stat;
    m61_get_statistics(&stat);
    assert((uintptr_t) p >= stat.heap_min);
    assert((uintptr_t) p + 9 <= stat.heap_max);

    free(p);
}
