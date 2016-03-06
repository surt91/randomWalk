#include "Walker.hpp"

const std::vector<Step> Walker::steps() const
{
    std::vector<Step> ret(numSteps+1);
    ret[0] = Step(std::vector<int>(d, 0));
    for(int i=1; i<=numSteps; ++i)
        ret[i] = Step(d, random_numbers[i]);

    return ret;
}

const std::vector<Step> Walker::points() const
{
    std::vector<Step> ret = steps();
    for(int i=1; i<numSteps; ++i)
        ret[i] += ret[i-1];

    return ret;
}

ConvexHull Walker::convexHull() const
{
    return ConvexHull(points());
}

void Walker::print() const
{
    for(auto i : points())
        std::cout << i << " ";
    std::cout << std::endl;
}

void Walker::svg(const std::string filename, const bool with_hull) const
{
    SVG pic(filename);
    const std::vector<Step> p = points();
    std::vector<std::vector<double>> points;
    int min_x=0, max_x=0, min_y=0, max_y=0;
    for(auto i : p)
    {
        int x1 = i[0], y1 = i[1];
        std::vector<double> point {(double) x1, (double) y1};

        pic.circle(x1, y1, true);

        points.push_back(point);

        if(x1 < min_x)
            min_x = x1;
        if(x1 > max_x)
            max_x = x1;
        if(y1 < min_y)
            min_y = y1;
        if(y1 > max_y)
            max_y = y1;
    }
    pic.polyline(points);

    points.clear();
    if(with_hull)
    {
        const std::vector<Step> h = convexHull().hullPoints();
        for(auto i : h)
        {
            std::vector<double> point {(double) i[0], (double) i[1]};
            points.push_back(point);
        }
        pic.polyline(points, true, std::string("red"));
    }
    pic.setGeometry(min_x -1, min_y - 1, max_x + 1, max_y + 1);
    pic.save();
}
