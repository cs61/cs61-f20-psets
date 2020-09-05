#include "m61.hh"
#include <cstdio>
#include <cassert>
#include <cstring>
// Wild free.

int main() {
    int x;
    free(&x);
    m61_print_statistics();
}

//! MEMORY BUG???: invalid free of pointer ???, not in heap
//! ???
