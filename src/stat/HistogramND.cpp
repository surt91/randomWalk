#include "HistogramND.hpp"

HistogramND::HistogramND(const int bins, const int d, const double lower, const double upper)
    :   bins(bins),
        d(d),
        lower(lower),
        upper(upper),
        data(std::pow(bins, d), 0)
{
}

/// sum of all bins (for standard histograms)
int HistogramND::sum() const
{
    int s = 0;

    for(int i=0, total_bins=std::pow(bins, d); i<total_bins; ++i)
        s += data[i];

    return s;
}

/// max of all bins (for standard histograms)
int HistogramND::max() const
{
    int m = 0;

    for(int i=0, total_bins=std::pow(bins, d); i<total_bins; ++i)
        if(data[i] > m)
            m = data[i];

    return m;
}

/// resets the histogram
void HistogramND::reset()
{
    for(int i=0, total_bins=std::pow(bins, d); i<total_bins; ++i)
        data[i] = 0;
}

/// return the centers of the bins
const std::vector<double> HistogramND::centers() const
{
    std::vector<double> center;

    for(int i=0; i<bins; ++i)
    {
        double width = (upper - lower) / bins;
        center.push_back(width * i + width/2.);
    }

    return center;
}

/// get a reference to the raw data array
const std::vector<int>& HistogramND::get_data() const
{
    return data;
}

/// save a visualization, only d = 2
void HistogramND::svg(const std::string filename) const
{
    SVG pic(filename);

    // place rectangles for every bin
    for(int x=0; x<bins; ++x)
        for(int y=0; y<bins; ++y)
        {
            // ignore not-visited fields
            if(!data[x*bins + y])
                continue;

            // color should scale with number of entries
            std::string color = "#ff0000";
            double m = max();
            double opacity = data[x*bins + y] / m;
            pic.square(x, y, 1., color, opacity);
        }

    if(d > 2)
        pic.text(lower, upper-20, "projected from d=" + std::to_string(d), "red");

    pic.setGeometry(lower-1, lower-1, upper+1, upper+1);
    pic.save();
}
