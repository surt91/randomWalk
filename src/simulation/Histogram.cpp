#include "Histogram.hpp"

Histogram::Histogram(int num_bins, double lower, double upper)
    : num_bins(num_bins),
      lower(lower),
      upper(upper),
      m_cur_min(0),
      m_total(0),
      m_sum(0),
      above(0),
      below(0),
      data(num_bins, 0)


{
    double binwidth = (upper - lower) / num_bins;
    bins.reserve(num_bins);
    for(int i=0; i<num_bins; ++i)
        bins.emplace_back(lower + i*binwidth);
    bins.emplace_back(upper);
}

Histogram::Histogram(std::vector<double> bins)
    : num_bins(bins.size()),
      lower(bins.front()),
      upper(bins.back()),
      m_cur_min(0),
      m_total(0),
      m_sum(0),
      above(0),
      below(0),
      data(num_bins, 0)
{
}

/** Adds an entry to the corresponding bin.
 *
 * \param where A value for which the corresponding bin is updated
 * \param what  The value by which the bin should be updated (default 1)
 */
void Histogram::add(double where, double what)
{
    if(where >= upper)
        above += what;
    if(where < lower)
        below += what;

    int idx = std::upper_bound(bins.begin(), bins.end(), where) - bins.begin();
    --idx;

    double tmp = data[idx];
    data[idx] += what;
    ++m_total;
    m_sum += what;

    // see if this is the current minimum and update, if necessary
    if(tmp == m_cur_min)
    {
        m_cur_min += what;
        for(int i=0; i<num_bins; ++i)
            if(data[i] < m_cur_min)
                m_cur_min = data[i];
    }
}

/// minimum value of all bins
int Histogram::min() const
{
    return m_cur_min;
}

/// mean value of all bins
double Histogram::mean() const
{
    return (double) m_sum / num_bins;
}

/// number of insertions into the histogram
int Histogram::count() const
{
    return m_total;
}

/// sum of all bins (equal to count, for standard histograms)
int Histogram::sum() const
{
    return m_sum;
}

/// sets all entries to zero and clears statistical data
void Histogram::reset()
{
    m_cur_min = 0;
    m_total = 0;
    m_sum = 0;
    for(int i=0; i<num_bins; ++i)
        data[i] = 0;
}

double Histogram::operator[](const double value) const
{
    if(value >= upper)
        return above;
    if(value < lower)
        return below;

    int idx = std::upper_bound(bins.begin(), bins.end(), value) - bins.begin();
    --idx;
    return data[idx];
}

/// vector of num_bins elements containing their centers
const std::vector<double> Histogram::centers() const
{
    std::vector<double> c;
    c.reserve(num_bins);
    for(int i=0; i<num_bins; ++i)
        c.emplace_back((bins[i] + bins[i+1]) / 2);
    return c;
}

/// vector of num_bins elements containing their data
const std::vector<double>& Histogram::get_data() const
{
    return data;
}

/// vector of of num_bins + 1 elements containing their borders
const std::vector<double>& Histogram::borders() const
{
    return bins;
}

std::ostream& operator<<(std::ostream& os, const Histogram &obj)
{
    os << "[";
    for(int i=0; i<obj.num_bins; ++i)
        os << obj.bins[i] << " < " << obj.data[i] << " < " << obj.bins[i+1] << std::endl;
    os << "] ";
    return os;
}
