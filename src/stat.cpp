#include "stat.hpp"

double RollingMean::add(double x)
{
    state.pop_front();
    state.push_back(x);
    return std::accumulate(state.begin(), state.end(), 0.0) / state.size();
}
