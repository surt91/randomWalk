#include "BranchingGauss.hpp"

BranchingGauss::BranchingGauss(int d, int numSteps, const UniformRNG &rng_in, hull_algorithm_t hull_algo, bool amnesia)
    : SpecWalker<double>(d, numSteps, rng_in, hull_algo, amnesia),
      branch_prob(0.05),
      perish_prob(0.04)
{
    // we need d gaussian random numbers per step, for each direction
    reconstruct();
}

/// Get new random numbers and reconstruct the walk
void BranchingGauss::reconstruct()
{
    std::set<int> dead;
    branches.clear();
    branches.emplace_back(std::vector<Step<double>>());
    branches[0].push_back(Step<double>(d));
    for(int i=0; i<numSteps; ++i)
    {
        int cur_branch = branches.size();
        for(int j=0; j<cur_branch; ++j)
        {
            if(dead.find(j) != dead.end())
                continue;
            if(rng() < perish_prob)
            {
                if(dead.size() == branches.size() - 1)
                {
                    LOG(LOG_WARNING) << "all branches would be dead";
                }
                else
                {
                    dead.insert(j);
                }
                continue;
            }
            if(rng() < branch_prob)
            {
                branches.emplace_back(std::vector<Step<double>>());
                branches[branches.size()-1].push_back(branches[j].back());
                ++i;
                // LOG(LOG_INFO) << "new branch" << (branches.size()-1) << " " << branches[branches.size()-1].back();
            }

            std::vector<double> tmp(d);
            for(int k=0; k<d; ++k)
                tmp[k] = rng.gaussian();

            Step<double> step(tmp);
            LOG(LOG_INFO) << i << " " << j;
            LOG(LOG_INFO) << branches[j].size();
            LOG(LOG_INFO) << branches[j].back();
            step += branches[j].back();

            branches[j].push_back(step);
            m_points.push_back(step);
        }
    }
}


void BranchingGauss::updateSteps()
{
    LOG(LOG_ERROR) << "not implemented";
}

void BranchingGauss::change(UniformRNG &/*rng*/, bool /*update*/)
{
    LOG(LOG_ERROR) << "not implemented";
}

void BranchingGauss::undoChange()
{
    LOG(LOG_ERROR) << "not implemented";
}

void BranchingGauss::degenerateMaxVolume()
{
    LOG(LOG_ERROR) << "not implemented";
}

void BranchingGauss::svg(const std::string filename, const bool with_hull) const
{
    SVG pic(filename);
    int min_x=0, max_x=0, min_y=0, max_y=0;
    int idx = 0;
    for(const auto &p : branches)
    {
        ++idx;
        std::vector<std::vector<double>> points;

        for(auto i : p)
        {
            auto x1 = i[0], y1 = i[1];
            std::vector<double> point {(double) x1, (double) y1};

            pic.circle(x1, y1, true, COLOR[idx%COLOR.size()]);

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
        pic.polyline(points, false, COLOR[idx%COLOR.size()]);

        points.clear();
    }

    if(d > 2)
        pic.text(min_x, max_y-20, "projected from d=" + std::to_string(d), "red");

    if(with_hull)
    {
        std::vector<std::vector<double>> points;
        const auto h = m_convex_hull.hullPoints();
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
