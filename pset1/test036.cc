#include "m61.hh"
#include <cstdio>
#include <cassert>
#include <cstring>
// Memory leak report with one leak.
// This version does not rely on the base allocator.

struct node {
    node* next;
};

int main() {
    base_allocator_disable(true);
    node* list = nullptr;

    // create a list
    for (int i = 0; i != 400; ++i) {
        node* n = (node*) malloc(sizeof(node));
        n->next = list;
        list = n;
    }

    // free everything in it but one
    while (list && list->next) {
        node** pprev = &list;
        while (node* n = *pprev) {
            *pprev = n->next;
            free(n);
            if (*pprev) {
                pprev = &(*pprev)->next;
            }
        }
    }

    printf("EXPECTED LEAK: %p with size %zu\n", list, sizeof(node));
    m61_print_leak_report();
}

//! EXPECTED LEAK: ??{0x\w*}=ptr?? with size ??{\d+}=size??
//! LEAK CHECK: test???.cc:18: allocated object ??ptr?? with size ??size??
