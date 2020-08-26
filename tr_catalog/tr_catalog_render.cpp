#include "transport_catalog.h"
#include "numbers_smart_comparison.h"
using namespace std;

#ifdef RENDER
using svg::Color;

const svg::Document& TransportCatalog::GetMap() const {
	if (!render_cache) {
		render_cache = make_unique<svg::Document>(render_map());
	}
	return *render_cache;
}

svg::Document TransportCatalog::render_map() const {
	svg::Document doc;
	const auto& layer_sequence{ render_settings->layer_sequence };
	for (const auto& layer : layer_sequence) {
		layer_renderers.at(layer)(this, addressof(doc));
	}
	return doc;
}

unique_ptr<TransportCatalog::Step> TransportCatalog::calculate_step_settings(MapIndex max_index) const {
	const auto& [width, height, padding]{ render_settings->map };
	return make_unique<Step>(
		Step{
		.x_step = !max_index.x_idx  ? 0 : (width - 2 * padding) / max_index.x_idx,
		.y_step = !max_index.y_idx  ? 0 : (height - 2 * padding) / max_index.y_idx
		}
	);
}

vector<TransportCatalog::StopHandleIt> 
TransportCatalog::collect_stops_location(StopsDatabase& stops) {
	vector<StopHandleIt> stops_location;
	stops_location.reserve(stops.size());
	for (auto it = stops.begin(); it != stops.end(); ++it) {
		stops_location.push_back(it);
	}
	return stops_location;
}

TransportCatalog::MapIndex TransportCatalog::distribute_stops_on_map() {
	auto stops_location{ collect_stops_location(stops) };
	MapIndex max_index;


	/*longitude compression*/
	compress_coordinates_in_place(
		addressof(stops_location),
		[](StopIt left, StopIt right) {
			return left->second.coordinates.longitude < right->second.coordinates.longitude;
		},
		[](MapIndex* idx, size_t new_value) {
			idx->x_idx = new_value; 
		});
	max_index.x_idx = stops_location.empty() ? 0 : stops_location.back()->second.index_on_map.x_idx;

	/*latitude compression*/
	compress_coordinates_in_place(
		addressof(stops_location),
		[](StopIt left, StopIt right) {
			return left->second.coordinates.latitude < right->second.coordinates.latitude;
		},
		[](MapIndex* idx, size_t new_value) {
			idx->y_idx = new_value;
		});
	max_index.y_idx = stops_location.empty() ? 0 : stops_location.back()->second.index_on_map.y_idx;

	return max_index;
}

svg::Point TransportCatalog::scale_coordinates(MapIndex idx) const noexcept {
	const auto& [_, height, padding] { render_settings->map };
	return svg::Point{
		.x = idx.x_idx * step_info->x_step + padding,
		.y = height - padding - idx.y_idx * step_info->y_step
	};
}

function<const Color&()> TransportCatalog::get_color_selector() const noexcept {
	const auto& palette{ render_settings->palette };
	return {
		[it = palette.begin(), begin = palette.begin(), end = palette.end()]
		() mutable -> const Color& {
			if (it == end) {
				it = begin;
			}
			return *it++;
		}
	};
}

void TransportCatalog::print_stops(svg::Document* doc) const {
	for (const auto& [_, stop_info] : stops) {
		draw_stop(doc, stop_info);
	}
}

void TransportCatalog::print_stop_labels(svg::Document* doc) const {
	for (const auto& [stop_id, stop_info] : stops) {
		draw_stop_label(doc, stop_id, stop_info);
	}
}

void TransportCatalog::print_bus_labels(svg::Document* doc) const {
	/*Color selection*/
	auto color_selector{ get_color_selector() };

	for (const auto& [bus_id, bus_info] : buses) {
		draw_bus_label(doc, bus_id, bus_info, color_selector());
	}
}

void TransportCatalog::print_bus_routes(svg::Document* doc) const {
	/*Color selection*/
	auto color_selector{ get_color_selector()};

	for (const auto& [_, bus_info] : buses) {
		draw_route(doc, bus_info, color_selector());
	}
}

void TransportCatalog::draw_stop(svg::Document* doc, const StopHolder& stop) const {
	doc->Add(
		svg::Circle{}
		.SetCenter(
			scale_coordinates(stop.index_on_map)
		)
		.SetRadius(render_settings->route.stop_radius)
		.SetFillColor("white")
	);
}

void TransportCatalog::draw_stop_label(svg::Document* doc, StopId stop_id, const StopHolder& stop_info) const {
	emplace_stop_label_on_map(
		doc,
		stop_id,
		scale_coordinates(stop_info.index_on_map)
	);
}

void TransportCatalog::emplace_stop_label_on_map(
	svg::Document* doc,
	StopId stop_id,
	svg::Point point
)  const {
	const auto& label_settings{ render_settings->stop_label };
	const auto& substrate_settings{ render_settings->substrate };

	svg::Text label_base;
	label_base
		.SetPoint(point)
		.SetOffset(label_settings.offset)
		.SetFontSize(label_settings.font_size)
		.SetFontFamily("Verdana")
		.SetData(string(stop_id));	//StopId - string_view!

	doc->Add(	//Substrate
		svg::Text(label_base)
		.SetFillColor(substrate_settings.underlayer_color)
		.SetStrokeColor(substrate_settings.underlayer_color)
		.SetStrokeWidth(substrate_settings.underlayer_width)
		.SetStrokeLineCap("round")
		.SetStrokeLineJoin("round")
	);

	doc->Add(	//Text
		move(label_base)	//label_base is no longer required
		.SetFillColor("black")
	);
}

void TransportCatalog::draw_bus_label(
	svg::Document* doc, 
	BusId bus_id, 
	const BusHolder& bus_info, 
	const Color& color
) const {
	const auto& waybill{ bus_info.waybill };
	if (!waybill.empty()) {
		const auto& final_stop_indices{ stops.at(waybill.front()).index_on_map };
		emplace_bus_label_on_map(
			doc,
			bus_id,
			scale_coordinates(final_stop_indices),
			color
		);
	}
	/*The bus can repeatedly pass through the final stop*/
	if (!bus_info.is_roundtrip && waybill.back() != waybill.front()) {
		const auto& further_stop_indices{ stops.at(waybill.back()).index_on_map };
		emplace_bus_label_on_map(
			doc,
			bus_id,
			scale_coordinates(further_stop_indices),
			color
		);
	}	
}

void TransportCatalog::emplace_bus_label_on_map(
	svg::Document* doc,
	BusId bus_id,
	svg::Point point,
	const svg::Color& color
)  const {
	const auto& label_settings{ render_settings->bus_label };
	const auto& substrate_settings{ render_settings->substrate };

	svg::Text label_base;
	label_base
		.SetPoint(point)
		.SetOffset(label_settings.offset)
		.SetFontSize(label_settings.font_size)
		.SetFontFamily("Verdana")
		.SetFontWeight("bold")
		.SetData(string(bus_id));	//BusId - string_view!

	doc->Add(
		svg::Text(label_base)
		.SetFillColor(substrate_settings.underlayer_color)
		.SetStrokeColor(substrate_settings.underlayer_color)
		.SetStrokeWidth(substrate_settings.underlayer_width)
		.SetStrokeLineCap("round")
		.SetStrokeLineJoin("round")	
	);
	doc->Add(
		move(label_base)
		.SetFillColor(color)
	);
}

void TransportCatalog::draw_route(svg::Document* doc, const BusHolder& bus, const Color& color) const {
	svg::Polyline route_polyline;
	route_polyline
		.SetStrokeLineCap("round")
		.SetStrokeLineJoin("round")
		.SetStrokeColor(color)
		.SetStrokeWidth(render_settings->route.line_width);
	const auto& waybill{ bus.waybill };

	route_polyline.MergePoints(create_route_polyline(waybill.begin(), waybill.end(), color));
	if (!waybill.empty()) {
		if (!bus.is_roundtrip) {	/*Return trip*/
			route_polyline.MergePoints(create_route_polyline(next(waybill.rbegin()), waybill.rend(), color));
		}
		else { /*Connect final and pre-final stops*/
			const svg::Point final_stop_on_map{ scale_coordinates(stops.at(waybill.front()).index_on_map) };
			route_polyline.AddPoint(final_stop_on_map);
		}
	}
	doc->Add(move(route_polyline));
}

bool TransportCatalog::are_neighbors_on_route(StopIt from, StopIt to) const {
	const auto& [from_vertex_id, from_bus_passes_count] {from->second.navigation};
	const auto& [to_vertex_id, to_bus_passes_count] { to->second.navigation };

	for (size_t from_idx = 1; from_idx <= from_bus_passes_count; ++from_idx) {							//The route can only start from the transitional vertex
		for (size_t to_idx = 0; to_idx <= to_bus_passes_count; ++to_idx) {								//But it can also end in root vertex
			if (graph->HasEdge(from_vertex_id.value() + from_idx, to_vertex_id.value() + to_idx)) {
				return true;
			}
		}	
	}
	return false;
}

bool TransportCatalog::can_be_compressed(StopIt left_id, StopIt right_id) const {
	return !are_neighbors_on_route(left_id, right_id) && !are_neighbors_on_route(right_id, left_id);
}

void TransportCatalog::compress_coordinates_in_place(
	std::vector<StopHandleIt>* storage,
	std::function<bool(StopIt left, StopIt right)> pred,
	std::function<void(MapIndex*, size_t)> visitor
) {
	sort(storage->begin(), storage->end(), pred);
	size_t idx{ 0 };
	if (!storage->empty()) {
		for (auto base_it = storage->begin(), compress_it = next(base_it);
			compress_it != storage->end(); ++compress_it) {
			auto closest_heighbor{ 
				find_if_not(
						base_it,
						compress_it,
						[this, &compress_it](StopIt current_stop_it) {
						return can_be_compressed(*compress_it,  current_stop_it);
					}
				)
			};
			if (closest_heighbor != compress_it) {
				base_it = compress_it;
				++idx;
			}
			visitor(addressof((*compress_it)->second.index_on_map), idx);
		}
	}	
}

#endif