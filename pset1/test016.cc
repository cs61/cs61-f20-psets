#include "m61.hh"
#include <cstdio>
#include <cassert>
#include <vector>
// Demonstrate destructors.

void f() {
    std::vector<int, m61_allocator<int>> v;
    for (int i = 0; i != 100; ++i) {
        v.push_back(i);
    }
    // `v` has automatic lifetime, so it is destroyed here.
}

int main() {
    f();
    m61_print_statistics();
    m61_print_leak_report();
}

//! alloc count: active          0   total    ??>=1??   fail          0
//! alloc size:  active        ???   total  ??>=400??   fail          0
