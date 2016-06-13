#pragma once

#include <algorithm>
#include <numeric>
#include <vector>
#include <deque>
#include <ctime>
#include <string>
#include <iostream>
#include <cstdio>
#include <memory>

#ifdef __unix__
#include <unistd.h> //for getpid
#endif

template <typename T>
T mean(std::vector<T> a)
{
    return std::accumulate(a.begin(), a.end(), T(0.0)) / a.size();
}

template <typename T>
T variance(std::vector<T> a, T m=T(0))
{
    if(!m)
        m = mean(a); // calculate mean here

    T tmp = std::accumulate(a.begin(), a.end(), T(0.0), [&](T part, T next){return part + (m - next) * (m - next);});
    return tmp/a.size();
}

std::string vmPeak();

std::string time_diff(clock_t start, clock_t end, int op=1);

/** Calculates a rolling mean over the values feeded to it.
 */
class RollingMean
{
    public:
        /** Construct a RollingMean with a window of n.
         *
         * \param n width of the window
         */
        RollingMean(int n)
            : n(n),
              m_mean(0.0),
              count(0),
              state(n, 0)
        {}

        double add(double x);
        double mean() const;
        double var(size_t last = 0) const;

    protected:
        int n;
        double m_mean;
        size_t count;
        std::deque<double> state;
};
