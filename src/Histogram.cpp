#include "Histogram.hpp"

Histogram::Histogram(int bins, double lower, double upper)
    : bins(bins),
      lower(lower),
      upper(upper),
      counts(bins, 0)
{
}

int Histogram::min()
{
    return cur_min;
}

double Histogram::mean()
{
    return (double) total / bins;
}

void Histogram::add(double value)
{
    // do not count values bigger than upper or smaller than lower
    if(value > upper || value < lower)
    {
        LOG(LOG_ERROR) << "value out of bounds";
        return;
    }


    int idx = (value - lower) / upper * bins;
    counts[idx]++;
    ++total;

    // see if this was the former minimum
    if(counts[idx] - 1 == cur_min)
        // search for the new minimum
        cur_min = *std::min(counts.begin(), counts.end());
}

void Histogram::reset()
{
    for(int i=0; i<bins; ++i)
        counts[i] = 0;
}

int Histogram::operator[](int idx)
{
    return counts[idx];
}
