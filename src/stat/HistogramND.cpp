#include "HistogramND.hpp"


 /*  _    _ _     _                                  _   _ _____
 | |  | (_)   | |                                | \ | |  __ \
 | |__| |_ ___| |_ ___   __ _ _ __ __ _ _ __ ___ |  \| | |  | |
 |  __  | / __| __/ _ \ / _` | '__/ _` | '_ ` _ \| . ` | |  | |
 | |  | | \__ \ || (_) | (_| | | | (_| | | | | | | |\  | |__| |
 |_|  |_|_|___/\__\___/ \__, |_|  \__,_|_| |_| |_|_| \_|_____/
                         __/ |
                        |___/                                  */


HistogramND::HistogramND(const int bins, const int d, const double lower, const double upper)
    :   bins(bins),
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

void HistogramND::reset()
{
    for(int i=0, total_bins=std::pow(bins, d); i<total_bins; ++i)
        data[i] = 0;
}

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

const std::vector<double>& HistogramND::get_data() const
{
    return data;
}

void HistogramND::svg(const std::string filename) const
{
    // TODO
    LOG(LOG_WARNING) << "not yet implemented";

    SVG pic(filename);

    // place rectangles for every bin
    // color should scale with number of entries

    if(d > 2)
        pic.text(lower, upper-20, "projected from d=" + std::to_string(d), "red");

    pic.setGeometry(lower-1, lower-1, upper+1, upper+1);
    pic.save();
}
