#include "Graph.hpp"

Graph::Graph(int N)
{
    m_node_set.reserve(N);
    for(int i=0; i<N; ++i)
    {
        m_node_set.push_back(i);
        m_adj_list.emplace_back();
    }
}

const std::vector<int>& Graph::nodes() const
{
    return m_node_set;
}

std::vector<int>& Graph::neighbors(int node)
{
    return m_adj_list[node];
}

void Graph::add_edge(int s, int t)
{
    m_adj_list[s].push_back(t);
    m_adj_list[t].push_back(s);
}

void Graph::remove_edges(int s)
{
    for(int t : neighbors(s))
        m_adj_list[t].erase(std::find(m_adj_list[t].begin(), m_adj_list[t].end(), s));
    m_adj_list[s].clear();
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
