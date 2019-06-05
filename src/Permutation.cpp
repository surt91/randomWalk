#include "Permutation.hpp"

Permutation::Permutation()
{
}

Permutation::Permutation(int length_)
{
    length = length_;
    permutation.resize(length);
    for(int i=0; i<length; ++i)
        permutation[i] = i;
}

void Permutation::sort()
{
    for(int i=0; i<length; ++i)
        permutation[i] = i;
}

int Permutation::value(int index)
{
    return permutation[index];
}

int Permutation::inverse(int value)
{
    return inverse_permutation[value];
}

void Permutation::swap(int a, int b)
{
    int tmp = permutation[a];
    permutation[a] = permutation[b];
    permutation[b] = tmp;

    inverse_permutation[permutation[a]] = a;
    inverse_permutation[permutation[b]] = b;
}

std::ostream& operator<<(std::ostream& os, const Permutation &p)
{
    os << "(";
    for(int i=0; i<p.length; ++i)
        os << p.permutation[i] << ", ";
    os << ") ";
    return os;
}

int Permutation::operator[](std::size_t idx) const
{
    return permutation[idx];
}
