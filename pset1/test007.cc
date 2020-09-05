#include "m61.hh"
#include <cstdio>
#include <cassert>
#include <cstring>
// Null pointers are freeable.

int main() {
    void* p = malloc(10);
    free(nullptr);
    free(p);
    m61_print_statistics();
}

//! alloc count: active          0   total          1   fail          0
//! alloc size:  active        ???   total         10   fail          0
