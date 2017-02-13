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

        bool bestfs(Step<int> source, Step<int> target, std::unordered_set<Step<int>>& occupied);

};

struct thingy {
    thingy(Step<int> v, int k) : key(k), value(v) {};
    int key;
    Step<int> value;
    bool operator<(const thingy &other) const
    {
        return key > other.key;
    };
};
