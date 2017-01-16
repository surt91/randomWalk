#pragma once

#include <vector>
#include <deque>
#include <unordered_set>
#include <unordered_map>

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
        Graph() {};

        const std::unordered_set<int>& nodes() const;
        std::unordered_set<int>& neighbors(int node);

        void add_node(int node);
        void remove_node(int node);
        void add_edge(int s, int t);

        bool connected(int s, int t);

    protected:
        std::vector<int> m_nodes;
        std::unordered_set<int> m_node_set;
        std::unordered_map<int, std::unordered_set<int>> m_adj_list;
};
