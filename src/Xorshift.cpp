#include "Xorshift.hpp"

Xorshift::Xorshift(uint64_t s)
    : state(s)
{
}

void Xorshift::seed(uint64_t s) {
    state = s ^ 0xa907c6d4a766cd5c;

    // warm up the generator -- a bit
    for(int i=0; i<10; ++i)
        raw();
}

double Xorshift::operator()() {
    return (double) raw() / (double) UINT64_MAX;
}

uint64_t Xorshift::raw() {
    uint64_t x = state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    state = x;
    return x * 0x2545F4914F6CDD1D;
}
