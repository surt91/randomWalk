#pragma once

#include <vector>
#include <algorithm>

#include "../Logging.hpp"

/** Histogram Class.
 *
 * Supports equidistant bins and also definition by passing the their borders.
 */
class Histogram
{
    protected:
        const int num_bins; ///< total number of bins
        const double lower; ///< lower bound of the histogram
        const double upper; ///< upper bound of the histogram

        int m_cur_min;        ///< current minimum entry in the histogram
        int m_total;          ///< total number of inserted data points
        int m_sum;            ///< sum of all bins

        double above;
        double below;

        std::vector<double> bins; ///< num_bins + 1 bin borders
        std::vector<double> data; ///< data inside the bins

    public:
        Histogram(const int bins, const double lower, const double upper);
        Histogram(const std::vector<double> bins);

        void add(double where, double what=1);

        int min() const;
        double mean() const;
        int sum() const;
        int count() const;
        void reset();

        const std::vector<double> centers() const;
        const std::vector<double>& borders() const;
        const std::vector<double>& get_data() const;

        double operator[](const double value) const;

        Histogram& operator+=(const Histogram &other);

        friend std::ostream& operator<<(std::ostream& os, const Histogram &obj);
};
