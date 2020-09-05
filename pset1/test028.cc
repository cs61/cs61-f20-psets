#include "m61.hh"
#include <cstdio>
#include <cassert>
#include <cstring>
// File name and line number of wild free.

int main() {
    void* ptr = malloc(2001);
    fprintf(stderr, "Bad pointer %p\n", (char*) ptr + 128);
    free((char*) ptr + 128);
    m61_print_statistics();
}

//! Bad pointer ??{0x\w+}=ptr??
//! MEMORY BUG: test???.cc:10: invalid free of pointer ??ptr??, not allocated
//! ???
