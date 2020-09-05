#include "m61.hh"
#include <cstdio>
#include <cassert>
#include <cstring>
// Double free.

int main() {
    void* ptr = malloc(2001);
    fprintf(stderr, "Will free %p\n", ptr);
    free(ptr);
    free(ptr);
    m61_print_statistics();
}

//! Will free ??{0x\w+}=ptr??
//! MEMORY BUG???: invalid free of pointer ??ptr??, double free
//! ???
