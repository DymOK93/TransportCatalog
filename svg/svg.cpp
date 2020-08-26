#include "svg.h"
using namespace std;

namespace svg {
	string Circle::Serialize() const {
		string result{ "<circle " };
		result += get_base().Serialize();
		result
			+= " cx="s
			+= escape_string(to_string(center.x))
			+= " cy="s
			+= escape_string(to_string(center.y))
			+= " r="s
			+= escape_string(to_string(radius));
		return result += "/>";
	}
	
	string Polyline::Serialize() const {
		string result{ "<polyline " };
		result += get_base().Serialize();
		result += " points=";
		result += escape_string(collect_points());
		return result += "/>";
	}
	string Polyline::collect_points() const {
		string path_str;
		for (const auto point : path) {
			path_str
				+= std::to_string(point.x)
				+ ','
				+= std::to_string(point.y)
				+ ' ';
		}
		return path_str;
	}

	string Text::Serialize() const {
		std::string result{ "<text " };
		result += get_base().Serialize();
		result
			+= " x="s
			+= escape_string(std::to_string(base_point.x))
			+= " y="s
			+= escape_string(std::to_string(base_point.y))
			+= " dx="s
			+= escape_string(std::to_string(offset.x))
			+= " dy="s
			+= escape_string(std::to_string(offset.y))
			+= " font-size="s
			+= escape_string(std::to_string(font_size));
		if (font_family) {
			result
				+= " font-family="s
				+= escape_string(*font_family);
		}
		if (font_weight) {
			result
				+= " font-weight="s
				+= escape_string(*font_weight);
		}
		result += ">" + data + "</text>";
		return result;
	}
	std::string Document::Render() const {
		std::string map{ std::string(xml_header) + std::string(svg_header) };
		map += render_picture(picture);
		map += std::string(svg_end);
		return map;
	}
	void Document::Render(std::ostream& out) const {
		out << Render();
	}

	std::string Document::render_picture(const std::vector<Object>& picture) {
		std::string picture_str;
		auto printer{
			[&picture_str](const auto& figure) {
					picture_str += figure.Serialize();
				}
		};
		for (const auto& object : picture) {
			visit(
				printer,
				object
			);
		}
		return picture_str;
	}
}