#include "m61.hh"
#include <cstdlib>
#include <cmath>
#include <cstring>
#include <cstdio>
#define NALLOCATORS 40
// hhtest: A sample framework for evaluating heavy hitter reports.

void* ptr = nullptr;

// 40 different allocation functions, defined in 3 files,
// for 40 different call sites
#include "hhtest-tinyalloc.cc"
#include "hhtest-smallalloc.cc"
#include "hhtest-largealloc.cc"

// An array of allocation functions
void (*allocators[])() = {
    &tiny1, &tiny2, &tiny3, &tiny4, &tiny5,
    &tiny6, &tiny7, &tiny8, &tiny9, &tiny10,
    &tiny11, &tiny12, &tiny13, &tiny14, &tiny15,
    &tiny16, &tiny17, &tiny18, &tiny19, &tiny20,
    &small1, &small2, &small3, &small4, &small5,
    &small6, &small7, &small8, &small9, &small10,
    &medium1, &medium2, &medium3, &medium4, &medium5,
    &large1, &large2, &large3, &large4, &large5
};

// Sizes allocated by those allocation functions;
// later allocation functions have much bigger sizes.
size_t allocation_sizes[NALLOCATORS] = {
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 2, 4, 8, 16, 32, 64,
    128, 256, 512, 1024, 2048, 4096, 8192, 16384, 20000, 24000
};

static void phase(double skew, unsigned long long count) {
    // Calculate the probability we'll call allocator I.
    // That probability equals  2^(-I*skew) / \sum_{i=0}^40 2^(-I*skew).
    // When skew=0, every allocator is called with equal probability.
    // When skew=1, the first allocator is called twice as often as the second,
    //   which is called twice as often as the third, and so forth.
    // When skew=-1, the first allocator is called HALF as often as the second,
    //   which is called HALF as often as the third, and so forth.
    double sum_p = 0;
    for (int i = 0; i < NALLOCATORS; ++i) {
        sum_p += pow(0.5, i * skew);
    }
    long limit[NALLOCATORS];
    double ppos = 0;
    for (int i = 0; i < NALLOCATORS; ++i) {
        ppos += pow(0.5, i * skew);
        limit[i] = RAND_MAX * (ppos / sum_p);
    }
    // Now the probability we call allocator I equals
    // (limit[i] - limit[i-1]) / (double) RAND_MAX,
    // if we pretend that limit[-1] == 0.

    // Pick `count` random allocators and call them.
    for (unsigned long long i = 0; i < count; ++i) {
        long x = random();
        int r = 0;
        while (r < NALLOCATORS - 1 && x > limit[r]) {
            ++r;
        }
        allocators[r]();
    }
}

int main(int argc, char **argv) {
    // use the system allocator, not the base allocator
    // (the base allocator can be slow)
    base_allocator_disable(1);

    if (argc > 1 && (strcmp(argv[1], "-h") == 0
                     || strcmp(argv[1], "--help") == 0)) {
        printf("Usage: ./hhtest\n\
       OR ./hhtest SKEW [COUNT]\n\
       OR ./hhtest SKEW1 COUNT1 SKEW2 COUNT2 ...\n\
\n\
  Each SKEW is a real number. 0 means each allocator is called equally\n\
  frequently. 1 means the first allocator is called twice as much as the\n\
  second, and so on. The default SKEW is 0.\n\
\n\
  Each COUNT is a positive integer. It says how many allocations are made.\n\
  The default is 1000000.\n\
\n\
  If you give multiple SKEW COUNT pairs, then ./hhtest runs several\n\
  allocation phases in order.\n");
        exit(0);
    }

    // parse arguments and run phases
    for (int position = 1; position == 1 || position < argc; position += 2) {
        double skew = 0;
        if (position < argc) {
            skew = strtod(argv[position], 0);
        }

        unsigned long long count = 1000000;
        if (position + 1 < argc) {
            count = strtoull(argv[position + 1], 0, 0);
        }

        phase(skew, count);
    }

    m61_print_heavy_hitter_report();
}
