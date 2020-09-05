#include "m61.hh"
#include <cstdio>
#include <cassert>
#include <cstring>
// Check for boundary write errors off the end of an allocated block.

int main() {
    int* ptr = (int*) malloc(sizeof(int) * 10);
    fprintf(stderr, "Will free %p\n", ptr);
    for (int i = 0; i <= 10 /* Whoops! Should be < */; ++i) {
        ptr[i] = i;
    }
    free(ptr);
    m61_print_statistics();
}

//! Will free ??{0x\w+}=ptr??
//! MEMORY BUG???: detected wild write during free of pointer ??ptr??
//! ???
