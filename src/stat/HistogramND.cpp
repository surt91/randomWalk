#include "HistogramND.hpp"

HistogramND::HistogramND(const int bins, const int d, const double lower, const double upper)
    :   bins(bins),
        d(d),
        lower(lower),
        upper(upper),
        bin_width((upper-lower) / bins),
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

/// return the bin borders
const std::vector<double> HistogramND::get_bins() const
{
    std::vector<double> borders;

    for(int i=0; i<bins+1; ++i)
    {
        double width = (upper - lower) / bins;
        borders.push_back(width * i);
    }

    return borders;
}

/// get a reference to the raw data array
const std::vector<int>& HistogramND::get_data() const
{
    return data;
}

/// get the number of bins in each direction
int HistogramND::num_bins() const
{
    return bins;
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

void HistogramND::print2D() const
{
    if(d != 2)
    {
        LOG(LOG_ERROR) << "this function does only work for d = 2";
        return;
    }
    for(int x=0; x<bins; ++x)
    {
        for(int y=0; y<bins; ++y)
            printf(" %5d", data[x*bins + y]);
        printf("\n");
    }
}
