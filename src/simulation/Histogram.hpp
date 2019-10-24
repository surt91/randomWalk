#ifndef HISTOGRAM_H
#define HISTOGRAM_H

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
        int num_bins;         ///< total number of bins
        double lower;         ///< lower bound of the histogram
        double upper;         ///< upper bound of the histogram

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
        double& at(int idx);

        int get_num_bins() const;
        int min() const;
        double mean() const;
        int sum() const;
        int count() const;
        void reset();
        void trim();

        const std::vector<double> centers() const;
        const std::vector<double>& borders() const;
        const std::vector<double>& get_data() const;

        double operator[](const double value) const;
        double& operator[](const double value);

        const std::string ascii_table() const;

        friend std::ostream& operator<<(std::ostream& os, const Histogram &obj);
};

#endif
