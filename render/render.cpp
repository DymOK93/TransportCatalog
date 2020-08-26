#include "render_settings_extractor.h"
using namespace std;

namespace render {
	Map GetMapSettings(const Json::map_t& render_settings_map) {
		return Map{
			.width = render_settings_map.at("width").AsNumber(),
			.height = render_settings_map.at("height").AsNumber(),
			.padding = render_settings_map.at("padding").AsNumber()
		};
	}
	Route GetRouteSettings(const Json::map_t& render_settings_map) {	
		return Route{
			.stop_radius = render_settings_map.at("stop_radius").AsNumber(),
			.line_width = render_settings_map.at("line_width").AsNumber()
		};
	}

	template <class NodeTy>
	svg::Color ExtractColor(const NodeTy&) {
		return svg::NoneColor;
	}

	template <>
	svg::Color ExtractColor<string>(const string& color_str) {
		return svg::Color(color_str);
	}

	template <>
	svg::Color ExtractColor<Json::array_t>(const Json::array_t& color_arr) {
		bool has_alpha_channel{ color_arr.size() > 3 };
		svg::Rgba color{
			svg::Rgb{
				.red = static_cast<uint8_t>(color_arr[0].AsNumber().GetWhole()),
				.green = static_cast<uint8_t>(color_arr[1].AsNumber().GetWhole()),
				.blue = static_cast<uint8_t>(color_arr[2].AsNumber().GetWhole()),
			},
			has_alpha_channel ? color_arr[3].AsNumber() : static_cast<double>(0),
		};
		if (has_alpha_channel) {
			return svg::Color(color);
		} 
		else {
			return svg::Color(static_cast<svg::Rgb>(color));
		}
	}

	Label GetStopLabelSettings(const Json::map_t& render_settings_map) {
		const auto& stop_label_offset_info{ render_settings_map.at("stop_label_offset").AsArray() };
		return {
			.font_size = static_cast<uint32_t>(render_settings_map.at("stop_label_font_size").AsNumber().GetWhole()),	//We need only whole part
			.offset = PointFromVector(stop_label_offset_info)
		};
	}


	BusLabel GetBusLabelSettings(const Json::map_t& render_settings_map) {
		const auto& bus_label_offset_info{ render_settings_map.at("bus_label_offset").AsArray() };
		return {
			.font_size = static_cast<uint32_t>(render_settings_map.at("bus_label_font_size").AsNumber().GetWhole()),	//We need only whole part
			.offset = PointFromVector(bus_label_offset_info)
		};
	}

	Substrate GetSubstrateSettings(const Json::map_t& render_settings_map) {
		return {
			.underlayer_width = render_settings_map.at("underlayer_width").AsNumber(),
			.underlayer_color = ExtractColor(render_settings_map.at("underlayer_color"))
		};
	}

	svg::Color ExtractColor(const Json::Node& color_holder) {
		svg::Color extracted_color;
		visit(
			[&extracted_color](const auto& color_data) {
				extracted_color = ExtractColor(color_data);
			},
			color_holder.GetBase()
		);
		return extracted_color;
	}

	svg::Point PointFromVector(const Json::array_t& point_holder) {
		return svg::Point{
				point_holder.front().AsNumber(),
				point_holder.back().AsNumber()
		};
	}

	Palette GetPalette(const Json::map_t& render_settings_map) {
		const auto& raw_palette{ render_settings_map.at("color_palette").AsArray() };
		Palette palette;
		for (const auto& color_node : raw_palette) {
			palette.push_back(ExtractColor(color_node));
		}
		return palette;
	}

	Layers GetLayersSequence(const Json::map_t& render_settings_map) {
		Layers layers_sequence;
		const auto& raw_sequence{ render_settings_map.at("layers").AsArray() };
		layers_sequence.reserve(raw_sequence.size());

		for (const auto& layer : raw_sequence) {
			layers_sequence.emplace_back(layer.AsString());
		}

		return layers_sequence;
	}
}

