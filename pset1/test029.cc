#include "m61.hh"
#include <cstdio>
#include <cassert>
#include <cstring>
// Misaligned wild free.

int main() {
    void* ptr = malloc(2001);
    fprintf(stderr, "Bad pointer %p\n", (char*) ptr + 127);
    free((char*) ptr + 127);
    m61_print_statistics();
}

//! Bad pointer ??{0x\w+}=ptr??
//! MEMORY BUG: test???.cc:10: invalid free of pointer ??ptr??, not allocated
//! ???
