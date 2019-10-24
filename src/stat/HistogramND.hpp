#ifndef HISTOGRAMND_H
#define HISTOGRAMND_H

#include <vector>
#include <algorithm>
#include <cmath>

#include "../Logging.hpp"
#include "../visualization/Svg.hpp"

/** Histogram Class for d dimensions.
 *
 * The histogram is isotrope, same range and amount of bins in every direction.
 * Supports equidistant bins.
 * Points outside the bounds are just discarded.
 */
class HistogramND
{
    protected:
        const int bins;                 ///< total number of bins in each direction
        const int d;                    ///< dimension of the histogram

        const double lower;             ///< lower bound of the histogram
        const double upper;             ///< upper bound of the histogram
        const double bin_width;         ///< width of a single bin

        std::vector<int> data;          ///< data inside the bins

    public:
        HistogramND(const int bins, const int d, const double lower, const double upper);

        template<class T>
        void add(T &coordinate);
        int sum() const;
        int max() const;
        void reset();

        const std::vector<double> centers() const;
        const std::vector<double> get_bins() const;
        const std::vector<int>& get_data() const;
        int num_bins() const;

        void svg(const std::string filename) const;

        void print2D() const;
};



/** Adds an entry to the corresponding bin.
 *
 * \param coordinate A vector specifiing the bin to be updated.
 */
template<class T>
void HistogramND::add(T &coordinate)
{
    if((size_t) d != coordinate.size())
    {
        LOG(LOG_ERROR) << "dimensions do not agree: d = " << d <<", input = " << coordinate.size();
        throw std::invalid_argument("dimensions do not agree");
    }

    for(int i=0; i<d; ++i)
        if(coordinate[i] < lower || coordinate[i] > upper)
            return;

    std::vector<int> indices(d, 0);
    for(int i=0; i<d; ++i)
    {

        int idx = (coordinate[i] - lower) / bin_width;

        // if we hit the upper border -> put it into the highest bin
        if(idx == bins)
            --idx;

        indices[i] = idx;
    }

    int idx = 0;
    for(int i=0; i<d; ++i)
        idx += std::pow(bins, i) * indices[i];

    ++data[idx];
}

#endif
