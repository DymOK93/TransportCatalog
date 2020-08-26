#include "svg_primitives.h"
using namespace std;

namespace svg {
	string rgb_to_string(Rgb rgba) {
		string rgba_str{
			to_string(rgba.red)
			+ ','
			+ std::to_string(rgba.green)
			+ ','
			+ std::to_string(rgba.blue)
		};
		return rgba_str;
	}

	string Color::ToString() const {
		string result;
		visit(
			[&result](const auto& color) {
				result = SerializeColor(color);
			},
			get_base()
				);
		return result;
	}


	string SerializeColor(const Rgb& rgb) {
		return
			std::string("rgb(")
			+= rgb_to_string(rgb)
			+= ")";
	}
	string SerializeColor(const Rgba& rgba) {
		return
			std::string("rgba(")
			+= rgb_to_string(rgba)
			+ ',' + std::to_string(rgba.alpha)
			+ ")";
	}
	string SerializeColor(const string& color) {
		return color;
	}
	string SerializeColor(const monostate& color) {
		return "none";
	}
}