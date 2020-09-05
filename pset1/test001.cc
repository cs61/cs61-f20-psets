#include "m61.hh"
#include <cstdio>
// Trivial check: no allocations == zero statistics.

int main() {
    m61_print_statistics();
}

// Lines starting with "//!" define the expected output for this test.

//! alloc count: active          0   total          0   fail          0
//! alloc size:  active          0   total          0   fail          0
