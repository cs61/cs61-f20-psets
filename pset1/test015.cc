#include "m61.hh"
#include <cstdio>
#include <cassert>
#include <vector>
// Now C++ library functions call your allocator
// (but do not provide line number information).

int main() {
    // The `m61_allocator<int>` argument tells the C++ standard library
    // to allocate `v`’s memory using `m61_malloc/free`.
    std::vector<int, m61_allocator<int>> v;
    for (int i = 0; i != 100; ++i) {
        v.push_back(i);
    }
    m61_print_statistics();
}

//! alloc count: active          1   total    ??>=1??   fail          0
//! alloc size:  active        ???   total  ??>=400??   fail        ???
