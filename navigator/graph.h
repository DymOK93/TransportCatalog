#pragma once
#include "routing.h"
#include "range.h"

#include <cstdlib>
#include <vector>
#include <tuple>
#include <optional>
#include <unordered_map>

namespace Graph {

    using VertexId = size_t;
    using EdgeId = size_t;

    template <class EdgeWeight, class EdgeData>
    class DirectedWeightedGraph {
    public:
        /*Type alias section #1*/
        using Weight = EdgeWeight;
        using Data = EdgeData;


        /*Graph edge*/
        struct Edge {
            VertexId from;
            VertexId to;
            Weight weight;
            Data item;
        };

        /*Type alias section #2 - incidence list storage*/
        using IncidenceList = std::vector<std::pair<VertexId, EdgeId>>;
        using IncidenceMap = std::unordered_map<VertexId, EdgeId>;
        using IncidentRange = Range<IncidenceList::const_iterator>;
    public:
        DirectedWeightedGraph(size_t vertex_count);

        size_t GetVertexCount() const;
        size_t GetEdgeCount() const;

        EdgeId AddEdge(const Edge& edge);
        bool HasEdge(VertexId from, VertexId to) const;
        EdgeId GetEdgeId(VertexId from, VertexId to) const;  
        const Edge& GetEdge(EdgeId edge_id) const;
        IncidentRange GetIncidentRange(VertexId from) const;

    private:
        std::vector<Edge> edges;
        std::vector<std::pair<IncidenceList, IncidenceMap>> incidence;
    };


    template <class Weight, class EdgeData>
    DirectedWeightedGraph<Weight, EdgeData>::DirectedWeightedGraph(size_t vertex_count) : incidence(vertex_count) {}

    template <class Weight, class EdgeData>
    EdgeId DirectedWeightedGraph<Weight, EdgeData>::AddEdge(const Edge& edge) {
        edges.push_back(edge);
        const EdgeId id = edges.size() - 1;
        auto& incident_bucket{ incidence[edge.from] };
        incident_bucket.first.push_back({ edge.to, id });
        incident_bucket.second.insert({ edge.to, id });
        return id;
    }

    template <class Weight, class EdgeData>
    size_t DirectedWeightedGraph<Weight, EdgeData>::GetVertexCount() const {
        return incidence.size();
    }

    template <class Weight, class EdgeData>
    size_t DirectedWeightedGraph<Weight, EdgeData>::GetEdgeCount() const {
        return edges.size();
    }

    template <class Weight, class EdgeData>
    const typename DirectedWeightedGraph<Weight, EdgeData>::Edge&
        DirectedWeightedGraph<Weight, EdgeData>::GetEdge(EdgeId edge_id) const {
        return edges[edge_id];
    }

    template <class Weight, class EdgeData>
    bool DirectedWeightedGraph<Weight, EdgeData>::HasEdge(VertexId from, VertexId to) const {
        return incidence[from].second.count(to);
    }

    template <class Weight, class EdgeData>
    EdgeId DirectedWeightedGraph<Weight, EdgeData>::GetEdgeId(VertexId from, VertexId to) const {
        return incidence[from].second.at(to);
    }

    template <class Weight, class EdgeData>
    typename DirectedWeightedGraph<Weight, EdgeData>::IncidentRange
        DirectedWeightedGraph<Weight, EdgeData>::GetIncidentRange(VertexId from) const {
        const auto& incident_list{ incidence[from].first };
        return {
            std::begin(incident_list),
            std::end(incident_list)
        };
    }
}
