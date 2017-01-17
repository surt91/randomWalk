#pragma once

#include <vector>
#include <deque>
#include <unordered_set>
#include <unordered_map>
#include <algorithm>

#include "Logging.hpp"

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

        bool connected(int s, int t);

    protected:
        std::vector<int> m_node_set;
        std::vector<std::vector<int>> m_adj_list;
};
