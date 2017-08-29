#include <cstdint>

/** simple xorshift* generator
 *
 * takes a seed x and generates some random number
 *
 * https://en.wikipedia.org/wiki/Xorshift#xorshift.2A
 */
class Xorshift {
    public:
        Xorshift(uint64_t seed);
        double operator()();
        uint64_t raw();
        void seed(uint64_t s);

    private:
        uint64_t state;
};
