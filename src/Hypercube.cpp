#include "Hypercube.hpp"

bool Hypercube::bestfs(Step<int> source, Step<int> target, std::unordered_set<Step<int>>& occupied)
{
    std::priority_queue<thingy> q;
    std::unordered_set<Step<int>> visited;

    q.emplace(source, target.dist(source));

    while(!q.empty())
    {
        Step<int> c = q.top().value;
        q.pop();

        double D = target.dist(c);
        if(c == target)
        {
            return true;
        }
        for(auto i : c.neighbors())
        {
            if(!visited.count(i) && !occupied.count(i))
            {
                visited.insert(i);
                double d = target.dist(i);
                if(d < D)
                {
                    q.emplace(c, D);
                    q.emplace(i, d);
                    continue;
                }
                else
                {
                    q.emplace(i, d);
                }
            }
        }
    }
    return false;
}
