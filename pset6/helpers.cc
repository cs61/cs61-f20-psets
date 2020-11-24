#include "helpers.hh"
#include <random>
#include <cstdlib>
#include <cctype>
#include <cassert>
#include <cstring>

// random_int(min, max)
//    Returns a random number in the range [`min`, `max`], inclusive.
int random_int(int min, int max) {
    static thread_local std::default_random_engine* random_engine;
    if (!random_engine) {
        random_engine = new std::default_random_engine{std::random_device()()};
    }
    return std::uniform_int_distribution<int>(min, max)(*random_engine);
}

// is_integer_string, is_real_string
//    Check whether `s` is a correctly-formatted decimal integer or
//    real number.
bool is_integer_string(const char* s) {
    char* last;
    (void) strtol(s, &last, 10);
    return last != s && !*last && !isspace((unsigned char) *s);
}

bool is_real_string(const char* s) {
    char* last;
    (void) strtod(s, &last);
    return last != s && !*last && !isspace((unsigned char) *s);
}

// simple_printer
//    Signal-safe printing functions.
simple_printer& simple_printer::operator<<(char ch) {
    assert(this->s_ < this->end_);
    *this->s_ = ch;
    ++this->s_;
    return *this;
}

simple_printer& simple_printer::operator<<(const char* s) {
    size_t len = strlen(s);
    assert(size_t(this->end_ - this->s_) >= len);
    memcpy(this->s_, s, len);
    this->s_ += len;
    return *this;
}

simple_printer& simple_printer::operator<<(unsigned long i) {
    int ndigits = 0;
    for (unsigned long x = i; ndigits == 0 || x != 0; x /= 10) {
        ++ndigits;
    }
    assert(this->end_ - this->s_ >= ndigits);
    int digit = ndigits;
    for (unsigned long x = i; digit != 0; x /= 10) {
        --digit;
        this->s_[digit] = '0' + (x % 10);
    }
    this->s_ += ndigits;
    return *this;
}

simple_printer& simple_printer::operator<<(long i) {
    if (i < 0) {
        assert(this->s_ < this->end_);
        *this->s_ = '-';
        ++this->s_;
        i = -i;
    }
    return (*this << static_cast<unsigned long>(i));
}
