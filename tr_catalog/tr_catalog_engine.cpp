#include "transport_catalog.h"

using geographic::Stop;
using geographic::Bus;
using stats::Distance;
using stats::Route;
using namespace std;

TransportCatalog& TransportCatalog::AddStop(Stop stop_) {
#ifdef MULTITHREADING
	Guard stops_guard{ stops_mtx };	//lock_quard is faster than unique_lock
#endif
	stops.insert(
		make_pair(
			stop_.name,
			StopHolder{
				stop_.coordinates,
				move(stop_.distances),
				BusList{},
				Navigation{},
#ifdef RENDER
				MapIndex{}
#endif
			}
		)
	);
	return *this;
}

TransportCatalog& TransportCatalog::AddBus(Bus bus_) {
	if (bus_.is_roundtrip) {
		bus_.stops.pop_back();							//Remove duplicate final stop
	}
#ifdef MULTITHREADING
	Guard buses_guard{ buses_mtx };				//lock_quard is faster than unique_lock	
#endif
	buses.insert(		
		make_pair(
			bus_.name,
			BusHolder{
				move(bus_.stops),                              //The bus can go through stops that aren't yet in the database
				bus_.is_roundtrip,
				nullopt
			}
		)
	);		
	return *this;
}

Distance TransportCatalog::calc_distance(const StopsDatabase::value_type& first, const StopsDatabase::value_type& second) {
	double geographic{
		geographic::calc_distance(first.second.coordinates, second.second.coordinates)
	};
	return {
		geographic,
		calc_real_distance(first, second).value_or(geographic)
	};
}

optional<double> TransportCatalog::calc_real_distance(const StopsDatabase::value_type& first, const StopsDatabase::value_type& second) {
	const auto& first_distance_list{ first.second.distance_list },
		second_distance_list{ second.second.distance_list };
	if (auto it = first_distance_list.find(second.first);
		it != first_distance_list.end()) {
		return static_cast<double>(it->second);
	}
	if (auto it = second_distance_list.find(first.first);
		it != second_distance_list.end()) {
		return static_cast<double>(it->second);
	}
	return nullopt;
}

optional<Route> TransportCatalog::GetBusInfo(string_view bus_name_) const {
	auto bus_it{ buses.find(bus_name_) };
	if (bus_it == buses.end()) {
		return nullopt;
	}
	return get_bus_route_stats(bus_it);
}

optional<stats::Stop<TransportCatalog::BusListIt>> TransportCatalog::GetStopInfo(string_view stop_name_) const {
	auto stop_it{ stops.find(stop_name_) };
	if (stop_it == stops.end()) {
		return nullopt;
	}
	return stats::Stop<BusListIt>{
		Range(
			stop_it->second.bus_list.begin(),
			stop_it->second.bus_list.end()
		)
	};
}

void TransportCatalog::SetRoutingSettings(const routing::Parameters& routing_settings_) {
	routing_settings = make_unique<routing::Parameters>(routing_settings_);
}

#ifdef RENDER
void TransportCatalog::SetRenderSettings(render::Settings render_settings_) {
	render_settings = make_unique<render::Settings>(move(render_settings_));
}
#endif

void TransportCatalog::Synchronize() {
#ifdef MULTITHREADING
	Guard stops_guard{ stops_mtx };
	Guard buses_guard{ buses_mtx };
#endif
	tie_stops_with_buses();
	const size_t graph_size{ 
		initialize_root_vertex_index(addressof(stops)) 
	};

	graph = make_graph(graph_size);
	navigator = make_unique<Navigator>(*graph);
#ifdef MULTITHREADING
	calculate_all_routes_stats();
#endif
#ifdef RENDER
	const auto max_idx{ 
		distribute_stops_on_map()								//The graph must be initialized to allocate stops on map
	};
	step_info = calculate_step_settings(max_idx);
#endif
}


void TransportCatalog::tie_stops_with_buses() {
	for (const auto& [id, bus_info] : buses) {
		for (const auto& stop : bus_info.waybill) {
			auto& stop_info{ stops[stop] };
			stop_info.bus_list.insert(id);
			++stop_info.navigation.bus_passes_count;
		}
	}
}

size_t TransportCatalog::initialize_root_vertex_index(StopsDatabase* stops_base) noexcept {
	VertexId current_root{ 0 };
	for (auto& [stop_id, stop_info] : *stops_base) {
		stop_info.navigation.root_vertex_id = current_root;
		current_root += stop_info.navigation.bus_passes_count + 1;					//Root vertex and one vertex for each bus
	}
	return current_root;
}

void TransportCatalog::calculate_all_routes_stats() {
	for (auto& [bus_id, bus_info] : buses) {
		bus_info.stats = calculate_single_route_stats(bus_info.waybill, bus_info.is_roundtrip);
	}
}

#ifdef MULTITHREADING
/*NOT synchronized*/
optional<Route> TransportCatalog::get_bus_route_stats(BusIt bus) const {
	return bus->second.stats;
}
#else	/*Lazy calculation*/
optional<Route> TransportCatalog::get_bus_route_stats(BusIt bus) const {
	auto& bus_info{ bus->second };
	if (!bus_info.stats) {
		bus_info.stats = calculate_single_route_stats(bus_info.waybill, bus_info.is_roundtrip);
	}
	return bus_info.stats;
}
#endif


Route TransportCatalog::calculate_single_route_stats(const Waybill& waybill, bool is_roundtrip) const {
	Route route;
	unordered_set<string_view> unique_stops_counter;

	for (auto stop_it = waybill.begin(); stop_it != waybill.end(); ++route.stops, ++stop_it) {
		auto stop_data_it{ stops.find(*stop_it) };										//Iterator to the first stop
		unique_stops_counter.insert(*stop_it);

		auto next_stop_it = next(stop_it);
		if (next_stop_it != waybill.end()) {
			auto next_stop_data_it{ stops.find(*next_stop_it) };						//Iterator to the second stop
			route.distance += calc_distance(*stop_data_it, *next_stop_data_it);
			if (!is_roundtrip) {														//Roundtrip  = circle route
				route.distance += calc_distance(*next_stop_data_it, *stop_data_it);		//The distance when the bus moves back may be different
			}
		}
	}

	route.unique_stops = unique_stops_counter.size();
	if (!is_roundtrip) {
		(route.stops <<= 1) -= 1;														 //If route isn't roundtrip final stop isn't unique
	}
	else {
		++route.stops;
		auto final_it{ stops.find(waybill.front()) };
		auto previous_it{ stops.find(waybill.back()) };
		route.distance += calc_distance(*previous_it, *final_it);
	}

	return route;
}

TransportCatalog::TransportGraphHolder TransportCatalog::make_graph(size_t vertex_count) {
	using routing::Point;
	TransportGraphHolder graph_holder(make_unique<TransportGraph>(vertex_count));
	unordered_map<string_view, size_t> current_vertex;

	add_transitional_stops(graph_holder.get());

	for (const auto& [bus_id, bus_info] : buses) {
		const auto& waybill{ bus_info.waybill };

		for (auto stop_it = waybill.begin(); stop_it != waybill.end(); ++stop_it) {
			auto stop_data_it{ stops.find(*stop_it) };
			const VertexId first_root_vertex{ *stop_data_it->second.navigation.root_vertex_id };	//No value check required

			auto next_stop_it = next(stop_it);
			if (next_stop_it != waybill.end()) {
				auto next_stop_data_it{ stops.find(*next_stop_it) };						
				const VertexId second_root_vertex{ *next_stop_data_it->second.navigation.root_vertex_id },
					first_bus_vertex{ first_root_vertex + current_vertex[*stop_it] + 1 },
					second_bus_vertex{ second_root_vertex + current_vertex[*next_stop_it] + 1 };

				add_route_edges(
					graph_holder.get(),

					/*Connect two bus vertexex with edge*/
					pair{ first_bus_vertex, second_bus_vertex },

					calc_distance(*stop_data_it, *next_stop_data_it).real,
					bus_id
				);

				if (!bus_info.is_roundtrip) {
					add_route_edges(
						graph_holder.get(),

						/*If the stop is final, connect the edge to the root vertex*/
						pair{ second_bus_vertex, stop_it != waybill.begin() ? first_bus_vertex : first_root_vertex },

						calc_distance(*next_stop_data_it, *stop_data_it).real,
						bus_id
					);
				}		
			}
			++current_vertex[*stop_it];
		}

		if (bus_info.is_roundtrip) {
			auto final_it{ stops.find(bus_info.waybill.front()) };
			auto previous_it{ stops.find(bus_info.waybill.back()) };
			add_route_edges(
				graph_holder.get(),

				/*Connect penultimate and final stops*/
				pair{ 
					*previous_it->second.navigation.root_vertex_id 
				+ current_vertex.at(
					bus_info.waybill.back()
				), 
				*final_it->second.navigation.root_vertex_id 
				},

				calc_distance(*previous_it, *final_it).real,
				bus_id
			);
		}
	} 
	return graph_holder;
}

void TransportCatalog::add_route_edges(
	TransportGraph* graph,
	pair<VertexId, VertexId> from_to,
	double distance,
	std::string_view bus_name
) {
	using routing::Point;

	graph->AddEdge(Edge{
				from_to.first,
				from_to.second,
				geographic::travel_time(
					distance,
					geographic::kmph_to_mpmin(routing_settings->bus_velocity)
				),
				EdgeData{ Point::Type::BUS, bus_name }
		});
}


void TransportCatalog::add_transitional_stops(TransportGraph* graph) {	//One stop for each route
	for (const auto& [stop_id, stop_info] : stops) {
		const VertexId root_vertex{ *stop_info.navigation.root_vertex_id };
		for (size_t i = 0; i < stop_info.navigation.bus_passes_count; ++i) {
			connect_transitional_stops(
				graph,
				pair{ root_vertex, root_vertex + i + 1 },
				static_cast<double>(routing_settings->bus_wait_time),
				stop_id
			);
		}
	}
}

void TransportCatalog::connect_transitional_stops(	//Connect root vertex to all route vertices for each stop using pair of edges
	TransportGraph* graph,
	std::pair<size_t, size_t> stop_vertex,
	double wait_time,
	string_view stop_name
) {
	using routing::Point;
	graph->AddEdge(Edge{
		stop_vertex.first,
		stop_vertex.second,
		wait_time,
		EdgeData{Point::Type::WAIT, stop_name}
	});
	graph->AddEdge(Edge{
		stop_vertex.second,
		stop_vertex.first,
		0,
		nullopt
	});
}

optional<routing::OnMap> TransportCatalog::GetRouting(const routing::Bounds& segment) const {
	using routing::Point;
	using routing::OnMap;


	const auto& first_stop{ stops.at(segment.from) },
		last_stop{ stops.at(segment.to) };

	auto routing{ navigator->BuildRoute(
		*first_stop.navigation.root_vertex_id,
		*last_stop.navigation.root_vertex_id)
	};

	if (!routing) {
		return nullopt;
	}
	return collect_route_points(*routing);	//Building a route from the edges of a graph
}

routing::OnMap TransportCatalog::collect_route_points(const TransportGraphRoute& graph_route) const {
	using routing::Point;
	using routing::OnMap;

	routing::Way route;
	double total_time{ 0 };

	for (const auto& edge_id : graph_route) {
		const auto& edge{ graph->GetEdge(edge_id)};

		if (edge.item) {
			Point point{ make_routing_point(edge) };

			if (route.empty() ||									//Insert new point
				route.back().type == Point::Type::WAIT ||
				route.back().type != point.type ||
				route.back().name != point.name
				) {
				route.push_back(point);
			}
			else {
				++*route.back().span_count;							//Update last point
				route.back().time += point.time;
			}
			total_time += edge.weight;
		}	
	}
	return OnMap{
		total_time,
		move(route)
	};
}

routing::Point TransportCatalog::make_routing_point(const TransportCatalog::Edge& edge) noexcept {
	using routing::Point;

	const bool waiting{ edge.item->type == Point::Type::WAIT };

	return routing::Point{
		edge.item->type,
		edge.item->name,
		edge.weight,
		waiting ? nullopt : optional<uint64_t>(1)
	};
}