#include <map>
#include <vector>
#include <iostream>

#include "Logging.hpp"

/// This class stores a permutation and makes it easy to change it as well as
/// get access to its inverse
class Permutation
{
protected:
    int length;

    std::vector<int> permutation;
    std::map<int, int> inverse_permutation;

public:
    Permutation();
    Permutation(int n);

    int value(int index);
    int inverse(int value);
    void swap(int a, int b);

    void sort();

    template<typename Iter>
    void shuffle(Iter begin, Iter end);

    int operator[](std::size_t idx) const;

    friend std::ostream& operator<<(std::ostream& os, const Permutation &p);
};


template<typename Iter>
void Permutation::shuffle(Iter begin, Iter end)
{
    auto t = begin;
    // fisher yates
    for(int i=length-1; i>0; ++t, --i)
    {
        if(t == end)
        {
            LOG(LOG_ERROR) << "not enough random numbers given";
            exit(1);
        }

        int j = (i+1) * *t;

        int tmp = permutation[j];
        permutation[j] = permutation[i];
        permutation[i] = tmp;
    }

    inverse_permutation.clear();
    for(int i=0; i<length; ++i)
    {
        inverse_permutation.emplace(permutation[i], i);
    }
}
