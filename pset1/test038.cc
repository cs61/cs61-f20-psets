#include "m61.hh"
#include <cstdio>

char* ptrs[100];

void test(int nalloc, int nfree) {
    m61_allocator<char> allocator;
    for (int i = 0; i < nalloc - 1; ++i) {
        ptrs[i] = allocator.allocate(i + 1);
    }
    if (nalloc > 0) {
        ptrs[nalloc - 1] = allocator.allocate(nalloc);
    }
    for (int i = 0; i != nfree; ++i) {
        allocator.deallocate(ptrs[i], i + 1);
    }
}

int main(int argc, char** argv) {
    int nalloc = argc < 2 ? 10 : strtol(argv[1], nullptr, 0);
    int nfree = argc < 3 ? 5 : strtol(argv[2], nullptr, 0);
    assert(nalloc >= 0 && nfree >= 0);
    assert(nalloc <= 100 && nfree <= nalloc);
    test(nalloc, nfree);
    m61_print_statistics();
    m61_print_leak_report();
}

//!!UNORDERED
//! alloc count: active          5   total         10   fail        ???
//! alloc size:  active         40   total         55   fail        ???
//! LEAK CHECK: ???: allocated object ??{\w+}?? with size 6
//! LEAK CHECK: ???: allocated object ??{\w+}?? with size 7
//! LEAK CHECK: ???: allocated object ??{\w+}?? with size 8
//! LEAK CHECK: ???: allocated object ??{\w+}?? with size 9
//! LEAK CHECK: ???: allocated object ??{\w+}?? with size 10
