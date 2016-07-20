#include "RollingMean.hpp"

/** Append a number to the rolling mean.
 */
double RollingMean::add(double x)
{
    ++count;
    count = std::min(count, state.size());
    state.pop_back();
    state.push_front(x);
    m_mean = std::accumulate(state.begin(), state.begin()+count, 0.0) / count;
    return m_mean;
}

/** Return the mean of the current window.
 */
double RollingMean::mean() const
{
    return m_mean;
}

/** Return the variance of the current window.
 */
double RollingMean::var(size_t last) const
{
    //~ double tmp = std::accumulate(state.begin(), state.begin()+count, 0.0, [&](double part, double next){return part + (m_mean - next) * (m_mean - next);});
    if(last <= 0)
        last = count;
    last = std::min(last, count);

    double tmp = 0;
    for(auto i = state.begin(); i != state.begin()+last; ++i)
        tmp += (m_mean - *i) * (m_mean - *i);
    return tmp/last;
}
