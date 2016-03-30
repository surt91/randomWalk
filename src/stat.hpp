#pragma once

#include <algorithm>
#include <deque>

// T kann double, float oder so sein
template <typename T>
T mean(std::vector<T> a)
{
    return std::accumulate(a.begin(), a.end(), T(0.0)) / a.size();
}

// T kann double, float oder so sein
template <typename T>
T variance(std::vector<T> a, T m=T(0))
{
    if(!m)
        m = mean(a); // calculate mean here

    T tmp = std::accumulate(a.begin(), a.end(), T(0.0), [&](T part, T next){return part + (m - next) * (m - next);});
    return tmp/a.size();
}

class RollingMean
{
    public:
        RollingMean(int n)
            : n(n),
              state(n, 0)
        {}

        double add(double x);

    protected:
        int n;
        std::deque<double> state;
};
