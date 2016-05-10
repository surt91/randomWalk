#include "Histogram.hpp"

Histogram::Histogram(int bins, double lower, double upper)
    : bins(bins),
      lower(lower),
      upper(upper),
      binwidth((upper - lower) / bins),
      counts(bins, 0)
{
    cur_min = 0;
    total = 0;
}

int Histogram::min() const
{
    return cur_min;
}

double Histogram::mean() const
{
    return (double) total / bins;
}

int Histogram::sum() const
{
    return total;
}

void Histogram::add(double value)
{
    // do not count values bigger than upper or smaller than lower
    if(value > upper || value < lower)
    {
        //~ LOG(LOG_ERROR) << "value out of bounds: " << value << " not in [" << lower << "," << upper << "]";
        return;
    }

    int idx = (value - lower) / binwidth;
    counts[idx]++;
    ++total;

    // see if this was the former minimum and update, if necessary
    if(counts[idx] - 1 == cur_min)
    {
        ++cur_min;
        for(auto i=0; i<bins; ++i)
            if(counts[i] < cur_min)
                cur_min = counts[i];
    }
}

void Histogram::reset()
{
    cur_min = 0;
    total = 0;
    for(int i=0; i<bins; ++i)
        counts[i] = 0;
}

int Histogram::operator[](int idx) const
{
    return counts[idx];
}

std::ostream& operator<<(std::ostream& os, const Histogram &obj)
{
    os << "[";
    for(int i=0; i<obj.bins; ++i)
        os << (i*obj.binwidth+obj.lower) << " " << obj.counts[i] << std::endl;
    os << "] ";
    return os;
}
