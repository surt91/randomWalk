#include "GnuplotContour.hpp"

GnuplotContour::GnuplotContour(const std::string &f)
    : filename(f)
{
    /* Write Header */
    filename_matrix = filename + ".matrix.dat";
    filename_png = filename + ".png";
    filename += ".gp";

    buffer << "unset key\n"
              "unset tics\n"
              "\n"
              "set view map\n"
              "unset surface\n"
              "set contour base\n"
              "\n"
              "set isosam 31,31\n"
              "\n";
}

void GnuplotContour::data(const std::vector<HistogramND> &histograms)
{
    for(size_t i=0; i<histograms.size(); ++i)
    {
        std::string name = filename_matrix + std::to_string(i);

        buffer << "set table '" << name + ".lines" << "'\n";
        buffer << "splot '" << name << "' matrix\n";
        buffer << "unset table\n\n";

        const auto &data = histograms[i].get_data();
        const int bins = histograms[i].num_bins();

        const double max = histograms[i].max();

        std::ofstream oss(name);
        for(int x=0; x<bins; ++x)
        {
            for(int y=0; y<bins; ++y)
                oss << (data[x*bins + y] / max) << " ";
            oss << "\n";
        }
    }

    buffer << "\n"
              "set size ratio -1\n"
              "set term pngcairo size 1080,1080\n"
              "set output '" + filename_png + "'\n";

    buffer << "\nplot";
    for(size_t i=0; i<histograms.size(); ++i)
    {
        std::string name = filename_matrix + std::to_string(i);
        std::string c = COLOR[i%COLOR.size()];
        buffer << "'" + name + "' matrix using 1:2:(0x" << c[1] << c[2] << "):"
                                                  "(0x" << c[3] << c[4] << "):"
                                                  "(0x" << c[5] << c[6] << "):"
                                                  "(255*$3) with rgbalpha, \\\n";
    }

    for(size_t i=0; i<histograms.size(); ++i)
    {
        std::string name = filename_matrix + std::to_string(i);
        buffer << "  '" + name + ".lines" + "' w l lc '" << COLOR[i%COLOR.size()] << "',\\\n";
    }
    buffer << "\n";
}

void GnuplotContour::save()
{
    std::ofstream oss(filename);

    if(!oss.good())
    {
        LOG(LOG_ERROR) << "File can not be opened: " << filename;
        throw std::invalid_argument("cannot be opened");
    }

    LOG(LOG_INFO) << "Gnuplot file: " << filename;
    LOG(LOG_INFO) << "Plot with " << "gnuplot " << filename;

    oss << buffer.str();
}
