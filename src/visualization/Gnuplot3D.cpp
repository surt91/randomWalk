#include "Gnuplot3D.hpp"

Gnuplot3D::Gnuplot3D(const std::string &f)
    : filename(f)
{
    /* Write Header */
    filename_animate = filename + ".animate.gp";
    filename_points = filename + ".points.dat";
    filename_hull = filename + ".hull.dat";
    filename += ".gp";

    buffer << "unset key\n"
              "unset border\n"
              "unset colorbox\n"
              "unset tics\n\n"
              "set cbrange [0.9:1]\n"
              "set palette defined (1 '#ce4c7d')\n"
              "set style fill transparent solid 0.3 border\n"
              "set style line 1 lc rgb '#b90046' lt 1 lw 0.3\n"
              "set style line 2 lc rgb '#000077' lt 1 lw 2\n"
              "set pm3d depthorder hidden3d 1\n"
              "set pm3d implicit\n"
              "unset hidden3d\n\n"
              "splot '" + filename_points + "' w lp ls 2 pt 7 ps 0.5,\\\n"
              "      '" + filename_hull + "' u 1:2:3:(1) w lp ls 1 pt 4 ps 0.6\n\n"
              "pause mouse close\n";

    buffer_animate << "set term pngcairo size 1920,1080\n"
                      "\n"
                      "unset key\n"
                      "unset border\n"
                      "unset colorbox\n"
                      "unset tics\n"
                      "\n"
                      "set cbrange [0.9:1]\n"
                      "set palette defined (1 '#ce4c7d')\n"
                      "set style fill transparent solid 0.3 border\n"
                      "set style line 1 lc rgb '#b90046' lt 1 lw 0.3\n"
                      "set style line 2 lc rgb '#000077' lt 1 lw 2\n"
                      "set pm3d depthorder hidden3d 1\n"
                      "set pm3d implicit\n"
                      "unset hidden3d\n"
                      "\n"
                      "do for [ii=0:360:1] {\n"
                      "    set output sprintf(\"animate%03d.png\", ii)\n"
                      "    set view 60,ii\n"
                      "    splot '" + filename_points + "' w lp ls 2 pt 7 ps 0.5,\\\n"
                      "          '" + filename_hull + "' u 1:2:3:(1) w lp ls 1 pt 4 ps 0.6\n"
                      "}\n"
                      "\n"
                      "# make a video with\n"
                      "# ffmpeg -f image2 -pattern_type glob -i \"animate*.png\" -vcodec libx264 out.mp4\n";
}

void Gnuplot3D::polyline(const std::vector<std::vector<double>> &points)
{
    for(size_t i=1; i<points.size(); ++i)
        buffer_points << points[i][0] << " " << points[i][1] << " " << points[i][2] << "\n";
}

void Gnuplot3D::facet(const std::vector<double> &a, const std::vector<double> &b, const std::vector<double> &c)
{
    buffer_hull << a[0] << " " << a[1] << " " << a[2] << "\n"
                << b[0] << " " << b[1] << " " << b[2] << "\n\n"
                << c[0] << " " << c[1] << " " << c[2] << "\n"
                << c[0] << " " << c[1] << " " << c[2] << "\n"
                << "\n\n";
}

void Gnuplot3D::save()
{
    std::ofstream oss(filename);
    std::ofstream oss_a(filename_animate);
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
    oss_a << buffer_animate.str();
    oss_p << buffer_points.str();
    oss_h << buffer_hull.str();
}
