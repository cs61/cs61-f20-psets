// More allocation functions used for testing heavy hitters.

void small1() {
    free(ptr);
    ptr = malloc(1);
}

void small2() {
    free(ptr);
    ptr = malloc(1);
}

void small3() {
    free(ptr);
    ptr = malloc(1);
}

void small4() {
    free(ptr);
    ptr = malloc(1);
}

void small5() {
    free(ptr);
    ptr = malloc(2);
}

void small6() {
    free(ptr);
    ptr = malloc(4);
}

void small7() {
    free(ptr);
    ptr = malloc(8);
}

void small8() {
    free(ptr);
    ptr = malloc(16);
}

void small9() {
    free(ptr);
    ptr = malloc(32);
}

void small10() {
    free(ptr);
    ptr = malloc(64);
}
