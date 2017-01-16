#include "Graph.hpp"

const std::unordered_set<int>& Graph::nodes() const
{
    return m_node_set;
}

std::unordered_set<int>& Graph::neighbors(int node)
{
    return m_adj_list[node];
}

void Graph::add_node(int node)
{
    if(!m_node_set.count(node))
    {
        m_node_set.insert(node);
    }
    m_adj_list.emplace(node, std::unordered_set<int>());
}

void Graph::remove_node(int node)
{
    m_node_set.erase(node);
    m_adj_list.erase(node);
    for(auto &i : m_node_set)
        m_adj_list[i].erase(node);
}

void Graph::add_edge(int s, int t)
{
    if(m_node_set.count(s) && m_node_set.count(t))
    {
        m_adj_list[s].insert(t);
        m_adj_list[t].insert(s);
    }
}

bool Graph::connected(int s, int t)
{
    std::deque<int> seen;
    std::unordered_set<int> visited;
    seen.push_back(s);

    while(seen.size())
    {
        int c = seen.front();
        seen.pop_front();
        if(c == t)
            return true;
        for(auto i : neighbors(c))
        {
            if(!visited.count(i))
            {
                seen.push_back(i);
                visited.insert(i);
            }
        }
    }
    return false;
}
