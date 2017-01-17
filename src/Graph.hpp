#pragma once

#include <vector>
#include <deque>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>
#include <queue>

#include "Logging.hpp"
#include "Step.hpp"

/* A simple graph class.
 *
 * Implements an adjacency list.
 * Nodes are identified by their index (i.e. are just integers).
 *
 */
class Graph
{
    public:
        Graph(int N);

        const std::vector<int>& nodes() const;
        std::vector<int>& neighbors(int node);

        void add_edge(int s, int t);
        void remove_edges(int s);

        template<class T>
        bool connected(int s, int t, std::unordered_map<int, Step<T>>);

    protected:
        std::vector<int> m_node_set;
        std::vector<std::vector<int>> m_adj_list;

        bool bfs(int s, int t);
        template<class T>
        bool bestfs(int s, int t, std::unordered_map<int, Step<T>>);
};

template<class T>
bool Graph::connected(int s, int t, std::unordered_map<int, Step<T>> map)
{
    return bestfs(s, t, map);
}

struct thingy {
    thingy(int v, int k) : key(k), value(v) {};
    int key;
    int value;
    bool operator<(const thingy &other) const
    {
        return key < other.key;
    };
};

template<class T>
bool Graph::bestfs(int s, int t, std::unordered_map<int, Step<T>> map)
{
    std::priority_queue<thingy> q;
    std::unordered_set<int> visited;

    Step<T> target = map[t];

    q.emplace(s, target.dist(map[s]));

    while(!q.empty())
    {
        int c = q.top().value;
        q.pop();

        T D = target.dist(map[c]);
        if(c == t)
            return true;
        for(auto i : neighbors(c))
        {
            if(!visited.count(i))
            {
                visited.insert(i);
                T d = target.dist(map[i]);
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
