#pragma once

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
        int bins;                 ///< total number of bins in each direction
        int d;                    ///< dimension of the histogram

        double lower;             ///< lower bound of the histogram
        double upper;             ///< upper bound of the histogram

        std::vector<double> data; ///< data inside the bins

    public:
        HistogramND(const int bins, const int d, const double lower, const double upper);

        template<class T>
        void add(T &coordinate);
        int sum() const;
        int max() const;
        void reset();

        const std::vector<double> centers() const;
        const std::vector<double>& get_data() const;

        void svg(const std::string filename) const;

        // double operator[](const double value) const;
        // double& operator[](const double value);
        //
        // const std::string ascii_table() const;
        // Histogram& operator+=(const Histogram &other);
        // friend std::ostream& operator<<(std::ostream& os, const Histogram &obj);
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
        int idx = (coordinate[i] - lower) / (upper - lower) * bins;

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
