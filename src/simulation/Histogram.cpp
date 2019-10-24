#include "Histogram.hpp"

Histogram::Histogram(const int num_bins, const double lower, const double upper)
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

Histogram::Histogram(const std::vector<double> bins)
    : num_bins(bins.size()-1),
      lower(bins.front()),
      upper(bins.back()),
      m_cur_min(0),
      m_total(0),
      m_sum(0),
      above(0),
      below(0),
      bins(bins),
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
    {
        above += what;
        return;
    }
    if(where < lower)
    {
        below += what;
        return;
    }

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
int Histogram::get_num_bins() const
{
    return num_bins;
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

/** trim the histogram
 *
 * discard all bins left of the smallest without entries
 * and all bins right of the largest without entries
 */
void Histogram::trim()
{
    int left=0;
    int right=num_bins-1;

    while(data[left] == 0)
        ++left;
    while(data[right] == 0)
        --right;
    ++right;

    if(left == num_bins-1)
    {
        LOG(LOG_ERROR) << "The Histogram is empty after trimming!";
    }

    lower = bins[left];
    upper = bins[right];
    num_bins = right-left;

    std::vector<double> new_bins(num_bins+1);
    std::vector<double> new_data(num_bins);
    for(int i=left, j=0; i<=right; ++i, ++j)
        new_bins[j] = bins[i];

    // minimum needs to be updated
    m_cur_min = data[left+1];
    for(int i=left, j=0; i<right; ++i, ++j)
    {
        new_data[j] = data[i];
        if(data[i] < m_cur_min)
            m_cur_min = data[i];
    }

    bins = new_bins;
    data = new_data;
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

double& Histogram::operator[](const double value)
{
    if(value >= upper)
        return above;
    if(value < lower)
        return below;

    int idx = std::upper_bound(bins.begin(), bins.end(), value) - bins.begin();
    --idx;
    return data[idx];
}

double& Histogram::at(int idx)
{
    return data[idx];
}

const std::string Histogram::ascii_table() const
{
    std::stringstream ss;
    ss << ("# centers counts\n");
    auto c = centers();
    auto d = get_data();
    for(int i=0; i<num_bins; ++i)
        ss << c[i] << " " << d[i] << "\n";
    return ss.str();
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
        os << "[" <<obj.bins[i] << " - " << obj.bins[i+1] << "] :"
           << obj.data[i] << std::endl;
    os << "] ";
    return os;
}
