#include "Walker.hpp"

const std::vector<Step> Walker::steps(int limit) const
{
    if(limit)
    {
        if(limit != numSteps)
            stepsDirty = true;
        numSteps = limit;
    }

    if(!stepsDirty)
        return m_steps;

    m_steps.resize(numSteps);
    for(int i=0; i<numSteps; ++i)
        m_steps[i] = Step(d, random_numbers[i]);

    stepsDirty = false;
    return m_steps;
}

const std::vector<Step>& Walker::points(int start) const
{
    if(stepsDirty)
        steps();
    if(!pointsDirty)
        return m_points;

    if(m_points.size() != numSteps + 1)
    {
        m_points.resize(numSteps + 1);
        m_points[0] = Step(std::vector<int>(d, 0));
    }

    for(int i=start; i<=numSteps; ++i)
        m_points[i] = m_points[i-1] + m_steps[i-1];

    return m_points;
}

const ConvexHull& Walker::convexHull() const
{
    if(hullDirty)
    {
        //~ m_convex_hull = std::unique_ptr<ConvexHull>(new ConvexHullQHull(points()));
        m_convex_hull = std::unique_ptr<ConvexHull>(new ConvexHullAndrew(points()));
        hullDirty = false;
    }

    return *m_convex_hull;
}

double Walker::rnChange(const int idx, const double other)
{
    // I should do this in a far more clever way
    double tmp = random_numbers[idx];
    random_numbers[idx] = other;

    Step newStep(d, other);
    // test if something changes
    if(newStep == m_steps[idx])
        return tmp;

    m_steps[idx] = newStep;
    points(idx+1);
    hullDirty = true;

    return tmp;
}

int Walker::nSteps() const
{
    if(stepsDirty)
        steps();
    return numSteps;
}

std::string Walker::print() const
{
    std::stringstream ss;
    for(auto i : points())
        ss << i << " ";
    ss << "\n";
    return ss.str();
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
