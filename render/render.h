#pragma once
/*Graphical primitives and color palette*/
#include "svg_primitives.h"

#include <vector>
#include <string>
#include <cassert>

namespace render {
	struct Map {
		double
			width{ 0 }, 
			height{ 0 }, 
			padding{ 0 };
	};
	struct Route {
		double stop_radius{ 0 },
			line_width{ 0 };	
	};

	struct Label {
		uint32_t font_size{ 0 };
		svg::Point offset;	
	};

	struct Substrate {
		double underlayer_width{ 0 };
		svg::Color underlayer_color;
	};

	using BusLabel = Label;

	using Palette = std::vector<svg::Color>;
	using Layers = std::vector<std::string_view>;

	struct Settings {
		Map map;
		Route route;
		Label stop_label, bus_label;
		Substrate substrate;
		Palette palette;
		Layers layer_sequence;
	};
}