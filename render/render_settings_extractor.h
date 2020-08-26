#pragma once
/*Render data*/
#include "render.h"

/*For points and colors*/
#include "svg_primitives.h"

/*For extracting map parameters*/
#include "json.h"

namespace render {
	svg::Color ExtractColor(const Json::Node& color_holder);
	svg::Point PointFromVector(const Json::array_t& point_holder);

	Map GetMapSettings(const Json::map_t& render_settings_map);
	Route GetRouteSettings(const Json::map_t& render_settings_map);
	Label GetStopLabelSettings(const Json::map_t& render_settings_map);
	Label GetBusLabelSettings(const Json::map_t& render_settings_map);
	Substrate GetSubstrateSettings(const Json::map_t& render_settings_map);
	Palette GetPalette(const Json::map_t& render_settings_map);
	Layers GetLayersSequence(const Json::map_t& render_settings_map);
}
