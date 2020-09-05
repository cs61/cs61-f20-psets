#include "m61.hh"
#include <cstdio>
#include <cassert>
#include <cstring>
// Memory leak report with multiple leaks.

int main() {
    char* ptrs[10];
    ptrs[0] = (char*) malloc(10);
    ptrs[1] = (char*) malloc(11);
    ptrs[2] = (char*) malloc(12);
    ptrs[3] = (char*) malloc(13);
    ptrs[4] = (char*) malloc(14);
    ptrs[5] = (char*) malloc(15);
    ptrs[6] = (char*) malloc(16);
    ptrs[7] = (char*) malloc(17);
    ptrs[8] = (char*) malloc(18);
    ptrs[9] = (char*) malloc(19);

    free(ptrs[3]);
    free(ptrs[8]);
    free(ptrs[0]);
    m61_print_leak_report();
}

// The "//!!UNORDERED" line tells the check.pl script to allow the
// expected lines to occur in any order.

//!!UNORDERED
//! LEAK CHECK: test???.cc:10: allocated object ??{\w+}?? with size 11
//! LEAK CHECK: test???.cc:11: allocated object ??{\w+}?? with size 12
//! LEAK CHECK: test???.cc:13: allocated object ??{\w+}?? with size 14
//! LEAK CHECK: test???.cc:14: allocated object ??{\w+}?? with size 15
//! LEAK CHECK: test???.cc:15: allocated object ??{\w+}?? with size 16
//! LEAK CHECK: test???.cc:16: allocated object ??{\w+}?? with size 17
//! LEAK CHECK: test???.cc:18: allocated object ??{\w+}?? with size 19
