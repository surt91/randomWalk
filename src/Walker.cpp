#include "Walker.hpp"

const std::vector<Step> Walker::steps() const
{
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

    if(m_points.size() != (size_t) numSteps + 1)
    {
        m_points.resize(numSteps + 1);
        m_points[0] = Step(std::vector<int>(d, 0));
    }

    for(int i=start; i<=numSteps; ++i)
        m_points[i] = m_points[i-1] + m_steps[i-1];

    return m_points;
}

void Walker::setHullAlgo(hull_algorithm_t a)
{
    hullDirty = true;
    hull_algo = a;
}

const ConvexHull& Walker::convexHull() const
{
    if(hullDirty)
    {
        bool akl = false;
        switch(hull_algo)
        {
            case CH_QHULL_AKL:
                akl = true;
            case CH_QHULL:
                m_convex_hull = std::unique_ptr<ConvexHull>(new ConvexHullQHull(points(), akl));
                break;
            case CH_ANDREWS_AKL:
                akl = true;
            case CH_ANDREWS:
                m_convex_hull = std::unique_ptr<ConvexHull>(new ConvexHullAndrew(points(), akl));
                break;
            case CH_JARVIS_AKL:
                akl = true;
            case CH_JARVIS:
                m_convex_hull = std::unique_ptr<ConvexHull>(new ConvexHullJarvis(points(), akl));
                break;
            default:
                LOG(LOG_ERROR) << "Algorithm not implemented, yet: " << CH_LABEL[hull_algo];
                throw std::invalid_argument("this is not implemented");
        }
        hullDirty = false;
    }

    return *m_convex_hull;
}

void Walker::change(UniformRNG &rng)
{
    steps(); // steps need to be initialized
    int idx = rng() * nRN();
    undo_index = idx;
    undo_value = random_numbers[idx];
    random_numbers[idx] = rng();

    Step newStep(d, random_numbers[idx]);
    // test if something changes
    if(newStep == m_steps[idx])
        return;

    //~ undo_points = m_points;
    //~ undo_hull = *m_convex_hull;

    m_steps[idx] = newStep;
    points(idx+1);
    hullDirty = true;

    return;
}

void Walker::undoChange()
{
    random_numbers[undo_index] = undo_value;
    Step newStep(d, undo_value);
    // test if something changes
    if(newStep == m_steps[undo_index])
        return;

    m_steps[undo_index] = newStep;
    points(undo_index+1);
    hullDirty = true;
    //~ m_points = std::move(undo_points);
    //~ *m_convex_hull = std::move(undo_hull);
}

int Walker::nSteps() const
{
    if(stepsDirty)
        steps();
    return numSteps;
}

int Walker::nRN() const
{
    return random_numbers.size();
}

// set the random numbers such that we get an L shape
void Walker::degenerate()
{
    for(int i=0; i<numSteps; ++i)
        random_numbers[i] = .99 / ceil((double) d * (i+1)/numSteps);

    stepsDirty = true;
    pointsDirty = true;
    hullDirty = true;
}

// set the random numbers such that we get an L shape in d-1 dimensions
void Walker::degenerateMaxSurface()
{
    for(size_t i=0; i<random_numbers.size(); ++i)
        random_numbers[i] = .99 / ceil((double) (d-1) * (i+1)/random_numbers.size());

    stepsDirty = true;
    pointsDirty = true;
    hullDirty = true;
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

    if(d > 2)
        pic.text(min_x, max_y-20, "projected from d=" + std::to_string(d), "red");

    points.clear();
    if(with_hull)
    {
        const std::vector<Step> h = convexHull().hullPoints();
        for(auto &i : h)
        {
            std::vector<double> point {(double) i[0], (double) i[1]};
            points.push_back(point);
        }
        pic.polyline(points, true, std::string("red"));
    }
    pic.setGeometry(min_x -1, min_y - 1, max_x + 1, max_y + 1);
    pic.save();
}

void Walker::pov(const std::string filename, const bool with_hull) const
{
    Povray pic(filename);
    const std::vector<Step> p = points();
    std::vector<std::vector<double>> points;
    for(auto i : p)
    {
        int x = i[0], y = i[1], z = 0;
        if(d > 2)
            z = i[2];
        std::vector<double> point {(double) x, (double) y, (double) z};

        points.push_back(point);
    }
    pic.polyline(points);

    points.clear();
    if(with_hull && d > 2)
    {
        const std::vector<std::vector<Step>> h = convexHull().hullFacets();
        for(auto &i : h)
        {
            std::vector<double> p1 {(double) i[0][0], (double) i[0][1], (double) i[0][2]};
            std::vector<double> p2 {(double) i[1][0], (double) i[1][1], (double) i[1][2]};
            std::vector<double> p3 {(double) i[2][0], (double) i[2][1], (double) i[2][2]};
            pic.facet(p1, p2, p3);
        }
    }

    pic.save();
}

void Walker::saveConfiguration(const std::string &filename, bool append)
{
    std::string data(serialize());

    auto mode = append ? std::ofstream::binary | std::ofstream::app : std::ofstream::binary;
    std::ofstream oss(filename, mode);
    if(!oss.good())
    {
        LOG(LOG_ERROR) << "File can not be opened: " << filename;
        return;
    }

    binary_write(oss, data.size());
    binary_write_string(oss, data);

    // save checksum?
    LOG(LOG_DEBUG) << "Save file   : " << filename;
}

void Walker::loadConfiguration(const std::string &filename, int index)
{
    std::ifstream iss(filename, std::ifstream::binary);
    if(!iss.good())
    {
        LOG(LOG_ERROR) << "File can not be opened: " << filename;
        return;
    }

    size_t len = 0;
    int ctr = 0;
    while(ctr < index)
    {
        binary_read(iss, len);
        iss.seekg(len, iss.cur);
        ++ctr;
    }

    binary_read(iss, len);
    std::string data(binary_read_string(iss, len));

    // verify checksum?
    LOG(LOG_INFO) << "Read file   : " << filename;
    LOG(LOG_INFO) << "nsteps      : " << numSteps;
    LOG(LOG_INFO) << "dimension   : " << d;
    LOG(LOG_INFO) << "# random Num: " << random_numbers.size();
}

std::string Walker::serialize()
{
    std::stringstream ss;

    // write header data: nsteps, d, number of random numbers, state of rng
    binary_write(ss, numSteps);
    binary_write(ss, d);
    //~ std::string rng_state(rng.serialize_rng());
    //~ binary_write(ss, rng_state.size());
    //~ binary_write_string(ss, rng_state);
    binary_write(ss, random_numbers.size());
    for(const double i : random_numbers)
        binary_write(ss, i);

    return ss.str();
}

void Walker::deserialize(std::string s)
{
    size_t rn_size;
    std::stringstream ss;
    ss << s;

    binary_read(ss, numSteps);
    binary_read(ss, d);
    //~ size_t len_rng_state;
    //~ binary_read(iss, len_rng_state);
    //~ std::string rng_state(binary_read_string(iss, len_rng_state));
    //~ rng.deserialize_rng(rng_state);
    binary_read(ss, rn_size);

    random_numbers.clear();
    for(size_t i=0; i<rn_size; ++i)
    {
        double rn;
        binary_read(ss, rn);
        random_numbers.push_back(rn);
    }
}
