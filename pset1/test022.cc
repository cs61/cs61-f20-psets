#include "m61.hh"
#include <cstdio>
#include <cassert>
#include <cstring>
// Free of invalid pointer.

int main() {
    char* ptr = (char*) malloc(32);
    free(ptr - 32);
    m61_print_statistics();
}

//! MEMORY BUG???: invalid free of pointer ???, not in heap
//! ???
