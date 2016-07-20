#pragma once

#include <deque>

#include "stat.hpp"

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
