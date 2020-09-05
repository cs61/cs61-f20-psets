// More allocation functions used for testing heavy hitters.

void medium1() {
    free(ptr);
    ptr = malloc(128);
}

void medium2() {
    free(ptr);
    ptr = malloc(256);
}

void medium3() {
    free(ptr);
    ptr = malloc(512);
}

void medium4() {
    free(ptr);
    ptr = malloc(1024);
}

void medium5() {
    free(ptr);
    ptr = malloc(2048);
}

void large1() {
    free(ptr);
    ptr = malloc(4096);
}

void large2() {
    free(ptr);
    ptr = malloc(8192);
}

void large3() {
    free(ptr);
    ptr = malloc(10000);
}

void large4() {
    free(ptr);
    ptr = malloc(12000);
}

void large5() {
    free(ptr);
    ptr = malloc(14000);
}
