#include "Svg.hpp"

SVG::SVG(const std::string &filename, const double scale)
    : scale(scale), filename(filename)
{
    /* Write Header */
    header = std::string("<?xml version='1.0' encoding='UTF-8'?> \n\
                <!DOCTYPE svg PUBLIC '-//W3C//DTD SVG 1.1//EN' 'http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd'>\n\
                <svg xmlns='http://www.w3.org/2000/svg'\n\
                xmlns:xlink='http://www.w3.org/1999/xlink' xmlns:ev='http://www.w3.org/2001/xml-events'\n\
                version='1.1' baseProfile='full' width='800px' height='800px' >\n");

    radius = 0.2;
    stroke = 0.1;
}

/** Write a circle at the given coordinate to the output SVG file.
 *
 * \param x       x-coordinate of the center of the circle
 * \param y       y-coordinate of the center of the circle
 * \param filled  fill the circle if 1, don't if 0
*/
void SVG::circle(const double x, const double y, const int filled)
{
    buffer << "<circle cx='" << x << "' cy='" << y << "' r='" << radius*scale << "' stroke='black' stroke-width='0'";
    if(filled == 1)
        buffer << " fill='black'";
    else if(filled == -1)
        buffer << " fill='white'";
    else
        buffer << " stroke-dasharray='2,3' fill='whites'";

    buffer << "/>\n";
}

/** Schreibt einen Kreis in die gegebene Datei im SVG Format
 *
 * \param x1      x-start-coordinate of the line
 * \param x2      x-end-coordinate of the line
 * \param y1      y-start-coordinate of the line
 * \param y2      y-end-coordinate of the line
 * \param color   color of the line
*/
void SVG::line(const double x1, const double x2, const double y1, const double y2, const std::string color)
{
    buffer << "<line x1='" << x1 << "' x2='" << x2 << "' y1='" << y1 << "' y2='" << y2 << "' stroke='" << color << "' stroke-width='" << stroke*scale << "'/>\n";
}

void SVG::polyline(const std::vector<std::vector<double>> points, const bool closed, const std::string color)
{
    if(closed)
        buffer << "<polygon ";
    else
        buffer << "<polyline ";

    buffer << "fill='none' points='";
    for(const auto i : points)
        buffer << i[0] << "," << i[1] << " ";
    buffer << "' stroke='" << color << "' stroke-width='" << stroke*scale << "' />\n";
}

void SVG::text(const double x, const double y, const std::string &t, const std::string color)
{
    buffer << "<text x='" << x << "' y='" << y << "' fill='" << color << "'>" << t << "</text>";
}

void SVG::setGeometry(const double min, const double max, const bool border)
{
    setGeometry(min, min, max, max, border);
}

void SVG::setGeometry(const double min_x, const double min_y, const double max_x, const double max_y, const bool border)
{
    std::stringstream ss;
    ss << min_x << " " << min_y << " " << (max_x-min_x) << " " << (max_y-min_y) ;
    std::string area = ss.str();

    ss.str("");
    ss << "<rect x='" << min_x << "' y='" << min_y << "' width='" << (max_x - min_x) << "' height='" << (max_y-min_y) << "' fill='white' />\n";

    header.insert(header.find_last_of(">"), "viewBox='" + area + "'");
    header += ss.str();
    if(border)
    {
        line(min_x, min_x, min_y, max_y);
        line(min_x, max_x, max_y, max_y);
        line(max_x, max_x, min_y, max_y);
        line(max_x, min_x, min_y, min_y);
    }
}

void SVG::setScale(const double scaleIn)
{
    scale = scaleIn;
}

void SVG::save()
{
    std::ofstream oss(filename);

    if(!oss.good())
    {
        LOG(LOG_ERROR) << "File can not be opened: " << filename;
        throw std::invalid_argument("cannot be opened");
    }

    oss << header;                  // header
    oss << buffer.str();            // content
    oss << "</svg>" << std::endl;   // footer
}
