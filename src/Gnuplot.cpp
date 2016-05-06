#include "Gnuplot.hpp"

Gnuplot::Gnuplot(const std::string &filename)
    : filename(filename)
{
    /* Write Header */
    filename_points = filename + ".points.dat";
    filename_hull = filename + ".hull.dat";
    header = std::string("splot '" + filename_points + "' w lp pt 6 ps 0.3,\\\n"
                         "      '" + filename_hull + "' w lp pt 4 ps 0.4\n");
}

void Gnuplot::polyline(const std::vector<std::vector<double>> &points)
{
    for(size_t i=1; i<points.size(); ++i)
        buffer_points << points[i][0] << " " << points[i][1] << " " << points[i][2] << "\n";
}

void Gnuplot::facet(const std::vector<double> &a, const std::vector<double> &b, const std::vector<double> &c)
{
    buffer_hull << a[0] << " " << a[1] << " " << a[2] << "\n"
                << b[0] << " " << b[1] << " " << b[2] << "\n"
                << c[0] << " " << c[1] << " " << c[2] << "\n"
                << "- - -\n";
}

void Gnuplot::save()
{
    std::ofstream oss(filename);
    std::ofstream oss_p(filename_points);
    std::ofstream oss_h(filename_hull);

    if(!oss.good() || !oss_p.good() || !oss_h.good())
    {
        LOG(LOG_ERROR) << "File can not be opened: " << filename;
        throw std::invalid_argument("cannot be opened");
    }

    LOG(LOG_INFO) << "Gnuplot file: " << filename;
    LOG(LOG_INFO) << "Plot with " << "gnuplot " << filename;

    oss << header;                  // header
    oss << buffer.str();            // content
    oss << std::endl;
    oss_p << buffer_points.str();
    oss_p << std::endl;
    oss_h << buffer_hull.str();
    oss_h << std::endl;
}
