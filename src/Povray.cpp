#include "Povray.hpp"

Povray::Povray(const std::string &filename)
    : filename(filename)
{
    /* Schreibe Header */
    header = std::string(   "#version 3.6\n"
                            "global_settings{ assumed_gamma 1.0 }\n"
                            "#include \"colors.inc\"\n"
                            "#include \"textures.inc\"\n"
                            "\n"
                            "#declare boxTexture =\n"
                            "    texture {\n"
                            "      pigment {\n"
                            "        color rgb<0.5, 0.5, 0.5>\n"
                            "      }\n"
                            "      finish {\n"
                            "        diffuse 0.4\n"
                            "        ambient 0.2\n"
                            "        phong 1\n"
                            "        phong_size 100\n"
                            "        reflection 0.0\n"
                            "      }\n"
                            "    };\n"
                            "\n"
                            "#declare triTexture =\n"
                            "    texture {\n"
                            "      pigment{ color rgbf<1,0.7,0, 0.6>}\n"
                            "      finish {\n"
                            "        diffuse 0.4\n"
                            "        ambient 0.2\n"
                            "        phong 1\n"
                            "        phong_size 100\n"
                            "        reflection 0.0\n"
                            "      }\n"
                            "    };\n"
                            "\n"
                            "#macro placeBox(boxCenter, boxSize)\n"
                            "box {\n"
                            "    boxCenter - boxSize / 2\n"
                            "    boxCenter + boxSize / 2\n"
                            "    texture { boxTexture }\n"
                            "}\n"
                            "#end\n"
                            "\n"
                            "#macro placeTri(a, b, c)\n"
                            "triangle {\n"
                            "    a, b, c\n"
                            "    texture { triTexture }\n"
                            "}\n"
                            "#end\n"
                            "\n"
                            "light_source {\n"
                            "    <25, 25, 25>\n"
                            "    color White\n"
                            "}\n"
                            "light_source {\n"
                            "    <25, -25, 25>\n"
                            "    color White\n"
                            "}\n"
                            "light_source {\n"
                            "    <25, 25, -25>\n"
                            "    color White\n"
                            "}\n"
                            "light_source {\n"
                            "    <25, -25, -25>\n"
                            "    color White\n"
                            "}\n"
                            "camera {\n"
                            "    location <10, 10, 10>\n"
                            "    look_at  <0, 0, 0>\n"
                            "    rotate <0 ,360*clock, 0>\n"
                            "}\n\n"
                         );
}

void Povray::box(const double x, const double y, const double z, const double dx, const double dy, const double dz)
{
    buffer << "placeBox(<" << x << "," << y << "," << z << ">, <" << dx << "," << dy << "," << dz << ">)\n";
}

void Povray::polyline(const std::vector<std::vector<double>> &points)
{
    for(int i=1; i<points.size(); ++i)
    {
        const double cX1 = points[i-1][0], cX2 = points[i][0];
        const double cY1 = points[i-1][1], cY2 = points[i][1];
        const double cZ1 = points[i-1][2], cZ2 = points[i][2];

        double x = (cX1+cX2)/2;
        double y = (cY1+cY2)/2;
        double z = (cZ1+cZ2)/2;
        double dx = (0.5 + std::abs(cX1-cX2)*2)/2;
        double dy = (0.5 + std::abs(cY1-cY2)*2)/2;
        double dz = (0.5 + std::abs(cZ1-cZ2)*2)/2;

        box(x, y, z, dx, dy, dz);
    }
}

void Povray::facet(const std::vector<double> &x, const std::vector<double> &y, const std::vector<double> &z)
{
    buffer << "placeTri(<" << x[0] << "," << x[1] << "," << x[2] << ">, "
                        "<" << y[0] << "," << y[1] << "," << y[2] << ">, "
                        "<" << z[0] << "," << z[1] << "," << z[2] << ">)\n";

}

void Povray::save()
{
    std::ofstream oss(filename);

    if(!oss.good())
    {
        Logger(LOG_ERROR) << "File can not be opened:" << filename;
        throw std::invalid_argument("cannot be opened");
    }

    Logger(LOG_INFO) << "Povray file: " << filename;
    Logger(LOG_INFO) << "Render with " << "povray +O" << filename << ".png" << " +I" << filename << " +V +W1920 +H1080 +A";

    oss << header;                  // header
    oss << buffer.str();            // content
    oss << std::endl;
}
