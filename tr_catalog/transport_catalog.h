#pragma once

/*Data holders and helpers*/
#include "earth.h"
#include "geographic.h"
#include "stats.h"
#include "routing.h"

/*Routing*/
#include "navigator.h"
#include "graph.h"

/*Standart headers*/
#ifdef RENDER
#include <map>
#else
#include <unordered_map>
#endif
#include <unordered_set>
#include <string>
#include <string_view>
#include <optional>
#include <tuple>
#include <vector>
#include <cmath>
#include <utility>
#include <memory>
#include <algorithm>

#ifdef RENDER
/*SVG Graphics*/
#include "render.h"
#include "svg.h"

/*Color selection lamba*/
#include <functional>
#endif


#ifdef MULTITHREADING
/*Thread safety*/
#include <mutex>	
#endif

class TransportCatalog {
private:
	/*Type alias section #1*/
	using BusId = std::string_view;
	using StopId = std::string_view;
	using VertexId = Graph::VertexId;
	using BusList = std::set<BusId>;
	using BusListIt = BusList::const_iterator;
	using Waybill = std::vector<StopId>;

#ifdef RENDER
	/*Coordinates compression*/
	struct MapIndex {
		size_t
			x_idx{ 0 },
			y_idx{ 0 };
	};
#endif

	struct Navigation {
		std::optional<VertexId> root_vertex_id;
		size_t bus_passes_count{ 0 };											//The bus can go through the stop several times
	};

	/*Stops, buses and graph edge data holders*/
	struct StopHolder {
		geographic::Coordinates coordinates;
		geographic::DistanceList distance_list;
		BusList bus_list;
		Navigation navigation;
#ifdef RENDER
		MapIndex index_on_map;
#endif
	};

	struct BusHolder {
		Waybill waybill;													
		bool is_roundtrip;
#ifdef MULTITHREADING
		std::optional<stats::Route> stats;
#else	/*Lazy calculation*/
		mutable std::optional<stats::Route> stats;
#endif
	};

	struct EdgeData {
		routing::Point::Type type;
		std::string_view name;
	};

	/*Render step coefs*/
	struct Step {
		double
			x_step{ 0 },
			y_step{ 0 };
	};
	
	/*Type alias section #2 (databases)*/
#ifdef RENDER
	/*Buses and stops must be sorted alphabetically*/
	template <class Key, class Value>
	using Database = std::map<Key, Value>;
#else
	/*Versions without render*/
	template <class Key, class Value>
	using Database = std::unordered_map<Key, Value>;
#endif
	using StopsDatabase = Database<StopId, StopHolder>;
	using BusDatabase = Database<BusId, BusHolder>;
	using StopHandleIt = StopsDatabase::iterator;
	using StopIt = StopsDatabase::const_iterator;
	using BusHandleIt = BusDatabase::iterator;
	using BusIt = BusDatabase::const_iterator;

	/*Type alias section #3 (navigation)*/
	using Weight = double;
	using TransportGraph = Graph::DirectedWeightedGraph<Weight, std::optional<EdgeData>>;
	using TransportGraphHolder = std::unique_ptr<TransportGraph>;
	using Edge = TransportGraph::Edge;
	using Navigator = Graph::Navigator<TransportGraph>;
	using NavigatorHolder = std::unique_ptr<Navigator>;
	using TransportGraphRoute = Navigator::Route;

#ifdef MULTITHREADING
	/*Type alias section #4 (thread-safety)*/
	using Guard = std::lock_guard<std::mutex>;
#endif

public:
	/*Database update methods*/
	TransportCatalog& AddStop(geographic::Stop stop_);		
	TransportCatalog& AddBus(geographic::Bus bus_);
	void SetRoutingSettings(const routing::Parameters& routing_settings_);
#ifdef RENDER
	void SetRenderSettings(render::Settings render_settings_);
#endif

	/*Synchronization and statistics collection*/
	void Synchronize();

	/*Database search methods*/
	std::optional<stats::Route> GetBusInfo(std::string_view bus_name_) const;
	std::optional<stats::Stop<BusListIt>> GetStopInfo(std::string_view bus_name_) const;
	std::optional<routing::OnMap> GetRouting(const routing::Bounds& segment) const;
#ifdef RENDER
	/*SVG rendering methods*/
	const svg::Document& GetMap() const;
#endif

private:
	/*Calculation of travel statistics*/
	void calculate_all_routes_stats();
	stats::Route calculate_single_route_stats(const Waybill& waybill, bool is_roundtrip) const;

	/*Multi-thread and single-thread versions are different*/
	std::optional<stats::Route> get_bus_route_stats(BusIt bus) const;

	/*Sync stops and buses info*/
	void tie_stops_with_buses();

	/*Stop root_vertex_index initialization to build a graph*/
	static size_t initialize_root_vertex_index(StopsDatabase* stops_base) noexcept;

	/*Distance calculation*/
	static stats::Distance calc_distance(
		const StopsDatabase::value_type& first,
		const StopsDatabase::value_type& second
	);
	static std::optional<double> calc_real_distance(
		const StopsDatabase::value_type& first,
		const StopsDatabase::value_type& second
	);

	/*Navigation settings*/
	struct GraphBuildSettings {
		const routing::Parameters& routing_settings;
		size_t vertex_count;
	};

	/*Graph construction*/
	TransportGraphHolder make_graph(size_t vertex_count);

	/*Adding dummy stops for each route*/
	void add_transitional_stops(TransportGraph* graph);
	static void connect_transitional_stops(
		TransportGraph* graph,
		std::pair<size_t, size_t> vertices,
		double wait_time,
		std::string_view stop_name
	);

	/*Adding route adges to the graph*/
	void add_route_edges(
		TransportGraph* graph,
		std::pair<VertexId, VertexId> from_to,
		double distance,
		std::string_view bus_name
	);

	/*Assembly of the route from the edges of the graph*/
	routing::OnMap collect_route_points(const TransportGraphRoute& graph_route) const;
	static routing::Point make_routing_point(const TransportCatalog::Edge& edge) noexcept;

#ifdef RENDER
	/*SVG map rendering*/
	svg::Document render_map() const;
	std::unique_ptr<Step> calculate_step_settings(MapIndex max_index) const;

	static std::vector<StopHandleIt> collect_stops_location(StopsDatabase& stops);
	MapIndex distribute_stops_on_map();

	bool are_neighbors_on_route(StopIt left_id, StopIt right_id) const;
	bool can_be_compressed(StopIt left_id, StopIt right_id) const;
	

	void compress_coordinates_in_place(
		std::vector<StopHandleIt>* storage,
		std::function<bool(StopIt left, StopIt right)> pred,
		std::function<void(MapIndex*, size_t)> visitor
	);

	svg::Point scale_coordinates(MapIndex idx) const noexcept;
	std::function<const svg::Color& ()> get_color_selector() const noexcept;

	/*Custom rendering sequence*/
	static const std::unordered_map<
		std::string_view, 
		std::function<void(
			const TransportCatalog*, //*this
			svg::Document*
			)>> layer_renderers;

	void print_stops(svg::Document* doc) const;
	void draw_stop(
		svg::Document* doc, 
		const StopHolder& stop
	) const;

	void print_stop_labels(svg::Document* doc) const;
	void draw_stop_label(
		svg::Document* doc, 
		StopId stop_id,
		const StopHolder& stop_info
	) const;
	void emplace_stop_label_on_map(
		svg::Document* doc,
		StopId stop_id,
		svg::Point point
	)  const;

	void print_bus_labels(svg::Document* doc) const;
	void draw_bus_label(
		svg::Document* doc, 
		BusId bus_id, 
		const BusHolder& bus_info, 
		const svg::Color& color
	) const;
	void emplace_bus_label_on_map(
		svg::Document* doc,
		BusId bus_id,
		svg::Point point,
		const svg::Color& color
	)  const;

	void print_bus_routes(svg::Document* doc) const;
	void draw_route(
		svg::Document* doc, 
		const BusHolder& bus, 
		const svg::Color& color
	) const;

	/*Drawing routes in any direction*/
	template <class WaybillIt>
	svg::Polyline create_route_polyline(
		WaybillIt first,
		WaybillIt last,
		const svg::Color& color
	) const;

#endif
private:
	/*Databases*/
	StopsDatabase stops;
	BusDatabase buses;

	/*Navigation*/
	std::unique_ptr<routing::Parameters> routing_settings;
	TransportGraphHolder graph;
	NavigatorHolder navigator;

#ifdef RENDER
	/*2D Graphics*/
	std::unique_ptr<render::Settings> render_settings;
	std::unique_ptr<Step> step_info;
	mutable std::unique_ptr<svg::Document> render_cache;
#endif

#ifdef MULTITHREADING
private:
	/*Thread guards*/
	mutable std::mutex stops_mtx, buses_mtx;
#endif
};

#ifdef RENDER
template <class WaybillIt>
svg::Polyline TransportCatalog::create_route_polyline(
	WaybillIt first,
	WaybillIt last,
	const svg::Color& color
) const {
	svg::Polyline route_polyline;
	route_polyline
		.SetStrokeLineCap("round")
		.SetStrokeLineJoin("round")
		.SetStrokeColor(color)
		.SetStrokeWidth(render_settings->route.line_width);
	for (; first != last; ++first) {
		const svg::Point stop_on_map{ scale_coordinates(stops.at(*first).index_on_map) };
		route_polyline.AddPoint(stop_on_map);
	}
	return route_polyline;
}
#endif