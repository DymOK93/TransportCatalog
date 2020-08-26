#pragma once
#include "graph.h"

/*Standart headers*/
#include <set>
#include <vector>
#include <optional>
#include <utility>
#include <unordered_map>
#include <algorithm>

#ifdef MULTITHREADING
/*Thread safety*/
#include <mutex>	
#endif

namespace Graph {

	template <typename Graph>
	class Navigator {
	public:
		/*Type alias section #1 - graph data*/
		using Weight = typename Graph::Weight;
		using Path = std::vector<VertexId>;
		using Route = std::vector<EdgeId>;
		using ParentsList = std::vector<std::optional<VertexId>>;
		
		/*Type alias section #2 - single/multithread versions*/
#ifdef MULTITHREADING
		using ParentListCacheData = std::pair<std::optional<ParentsList>, std::mutex>;	
#else
		using ParentListCacheData = ParentsList;
#endif
		using ParentListCache = std::unordered_map<VertexId, ParentListCacheData>;

	private:
		/*Type alias section #3 - navigator internal data*/
		using DistanceInfo = std::vector<std::optional<Weight>>;
		using DijkstraPair = std::pair<Weight, VertexId>;
		using SearchHeap = std::set<DijkstraPair>;
	public:
		Navigator(const Graph& graph_) noexcept;

		std::optional<Route> BuildRoute(VertexId from, VertexId to) const;
	private:
		/*Get parent list from cache*/
		const ParentsList& get_parent_vertex_list(VertexId from) const;

		/*Dijkstra algorithm*/
		ParentsList relax_routes(VertexId from) const;

		/*Making route (sequence of edges) from vector of route vertices*/
		Route make_route(const Path& path) const;

		/*Making path (vector of route vertices) from parent list*/
		static std::optional<Path> collect_reversed_path(const ParentsList& parents, VertexId from, VertexId to);

#ifdef MULTITHREADING
		ParentListCacheData& get_parent_list_bucket(VertexId from) const;
#endif
		
	private:
		/*Data*/
		const Graph& graph;
		const size_t vertex_count;
		
		/*Cache*/
		mutable ParentListCache parent_list_cache;

#ifdef MULTITHREADING		
		mutable std::mutex mtx;
#else	/*Shared containers*/
		mutable DistanceInfo distances;
		mutable SearchHeap search_heap;
#endif
	};


	template <typename Graph>
	Navigator<Graph>::Navigator(const Graph& graph_) noexcept
		: graph(graph_), 
		vertex_count{graph_.GetVertexCount()} {

#ifndef MULTITHREADING
		distances.reserve(vertex_count);
#endif
	}

	template <typename Weight>
	std::optional<typename Navigator<Weight>::Route>
	Navigator<Weight>::BuildRoute(VertexId from, VertexId to) const {
		const auto reversed_path{ 
			collect_reversed_path(
				get_parent_vertex_list(from),
				from, to
			) };

		if(!reversed_path) {
			return std::nullopt;
		}

		return make_route(*reversed_path);
	}


#ifdef MULTITHREADING
	template <typename Weight>
	const typename Navigator<Weight>::ParentsList& 
	Navigator<Weight>::get_parent_vertex_list(VertexId from) const {
		auto& parent_list_bucket{ get_parent_list_bucket(from) };

		/*If two threads simultaneously want to access the basket, 
		one of them will be forced to wait for its update*/
		std::lock_guard parent_list_guard{ parent_list_bucket.second };	

		if (!parent_list_bucket.first) {
			parent_list_bucket.first = relax_routes(from);
		}

		return *parent_list_bucket.first;
	}

	template <typename Weight>
	typename Navigator<Weight>::ParentListCacheData& 
	Navigator<Weight>::get_parent_list_bucket(VertexId from) const {
		std::lock_guard bucket_access_lock(mtx);
		return parent_list_cache[from];
	}
#else
	template <typename Weight>
	const typename Navigator<Weight>::ParentsList&
		Navigator<Weight>::get_parent_vertex_list(VertexId from) const {
		auto it{ parent_list_cache.find(from) };

		if (it == parent_list_cache.end()) {
			distances.assign(vertex_count, std::nullopt);	//Reset distances cache
			it = parent_list_cache.insert({
				from,
				relax_routes(from)
				}).first;
		}
		return it->second;
	}
#endif

	template <typename Weight>
	typename Navigator<Weight>::ParentsList Navigator<Weight>::relax_routes(VertexId from) const {
		ParentsList parents(vertex_count, std::nullopt);

#ifdef MULTITHREADING	/*We can't use shared containers safely*/
		DistanceInfo distances(vertex_count, std::nullopt);
		SearchHeap search_heap;
#endif

		search_heap.insert({ 0, from });
		distances[from] = static_cast<Weight>(0);

		while (!search_heap.empty()) {
			auto min_it{ search_heap.begin() };
			const VertexId from_id{ min_it->second };
			search_heap.erase(min_it);

				const auto incidence_list{ graph.GetIncidentRange(from_id) };
				for (const auto& [possibly_to_id, edge_id] : incidence_list) {
					const Weight next_weight{ graph.GetEdge(edge_id).weight };

					if (!distances[possibly_to_id] || *distances[from_id] + next_weight < *distances[possibly_to_id]) {
						distances[possibly_to_id] = *distances[from_id] + next_weight;
						parents[possibly_to_id] = from_id;

						search_heap.insert({ next_weight, possibly_to_id });
					}
				}
		}
		return parents;
	}

	template <typename Weight>
	typename Navigator<Weight>::Route Navigator<Weight>::make_route(const Path& path) const {
		Route route;
		route.reserve(path.size());

		for (auto vertex_r_it = path.rbegin(); vertex_r_it != path.rend(); ++vertex_r_it) {
			auto next_vertex_r_it{ std::next(vertex_r_it) };
			if (next_vertex_r_it != path.rend()) {
				route.push_back(graph.GetEdgeId(*vertex_r_it, *next_vertex_r_it));
			}
		}
		return route;
	}

	template <typename Weight>
	std::optional<typename Navigator<Weight>::Path> Navigator<Weight>::collect_reversed_path(
		const ParentsList& parents, VertexId from, VertexId to
	) {
		Path path;
		std::optional<VertexId> vertex_id{ to };

		for (; vertex_id && *vertex_id != from; vertex_id = parents[*vertex_id]) {
			path.push_back(*vertex_id);
		}
		if (!vertex_id) {
			return std::nullopt;
		}

		path.push_back(from);
		return path;
	}
}

