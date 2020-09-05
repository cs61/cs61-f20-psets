#include "m61.hh"
#include <cstdio>
#include <cassert>
#include <cstring>
#include <cinttypes>
#include <cstddef>
// Check alignment of returned data.

int main() {
    double* ptr = (double*) malloc(sizeof(double));
    assert((uintptr_t) ptr % alignof(double) == 0);
    assert((uintptr_t) ptr % alignof(unsigned long long) == 0);
    assert((uintptr_t) ptr % alignof(std::max_align_t) == 0);

    char* ptr2 = (char*) malloc(1);
    assert((uintptr_t) ptr2 % alignof(double) == 0);
    assert((uintptr_t) ptr2 % alignof(unsigned long long) == 0);
    assert((uintptr_t) ptr2 % alignof(std::max_align_t) == 0);

    free(ptr);
    free(ptr2);
}
