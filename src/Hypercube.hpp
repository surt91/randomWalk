#pragma once

#include <vector>
#include <cmath>
#include <deque>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <queue>

#include "Step.hpp"

class Hypercube
{
    public:
        Hypercube() {}

        template<class T>
        bool bestfs(Step<int> source, Step<int> target, T& occupied);

};

struct thingy {
    thingy(Step<int> v, int k) : key(k), value(v) {}
    int key;
    Step<int> value;
    bool operator<(const thingy &other) const
    {
        return key > other.key;
    }
};

template<class T>
bool Hypercube::bestfs(Step<int> source, Step<int> target, T& occupied)
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
