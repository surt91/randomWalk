#include "Gnuplot2D.hpp"

Gnuplot2D::Gnuplot2D(const std::string &f)
    : filename(f)
{
    /* Write Header */
    filename_points = filename + ".points.dat";
    filename_hull = filename + ".hull.dat";
    filename += ".gp";

    buffer << "unset key\n"
              "unset border\n"
              "unset colorbox\n"
              "unset tics\n\n"
              "set size ratio -1\n"
              "set palette defined (1 '#ce4c7d')\n"
              "set style fill transparent solid 0.3 border\n"
              "set style line 1 lc rgb '#b90046' lt 1 lw 3\n"
              "set style line 2 lc rgb '#000077' lt 1 lw 2\n"
              "plot '" + filename_points + "' w lp ls 2 pt 7 ps 0.5,\\\n"
              "     '" + filename_hull + "' u 1:2 w lp ls 1 pt 4 ps 0.8\n\n"
              "pause mouse close\n";
}

void Gnuplot2D::polyline(const std::vector<std::vector<double>> &points, bool hull)
{
    if(!hull)
    {
        for(size_t i=0; i<points.size(); ++i)
            buffer_points << points[i][0] << " " << points[i][1] << "\n";
    }
    else
    {
        for(size_t i=0; i<points.size(); ++i)
            buffer_hull << points[i][0] << " " << points[i][1] << "\n";
        buffer_hull << points[0][0] << " " << points[0][1] << "\n";
    }
}

void Gnuplot2D::save()
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

    oss << buffer.str();
    oss_p << buffer_points.str();
    oss_h << buffer_hull.str();
}
