#pragma once
#include "svg_primitives.h"

#include <variant>
#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include <optional>
#include <string>
#include <string_view>

namespace svg {
	template <typename Content>
	class FigureHolder {
	public:
		std::string Serialize() const {
			std::string result;
			result
				+= std::string("fill=")
				+= escape_string(fill_color.ToString())
				+= std::string(" stroke=")
				+= escape_string(stroke_color.ToString())
				+= std::string(" stroke-width=")
				+= escape_string(std::to_string(stroke_width));
			if (stroke_line_cap) {
				result
					+= std::string(" stroke-linecap=")
					+= escape_string(*stroke_line_cap);
			}
			if (stroke_line_join) {
				result
					+= std::string(" stroke-linejoin=")
					+= escape_string(*stroke_line_join);
			}
			return result;
		}
	public:
		/*Setting color*/
		Content& SetFillColor(const Color& new_fill_color)& {
			return set_fill_color_helper(new_fill_color);
		}
		Content&& SetFillColor(const Color& new_fill_color)&& {
			return std::move(set_fill_color_helper(new_fill_color));
		}
		Content& SetStrokeColor(const Color& new_stroke_color)& {
			return set_stroke_color_helper(new_stroke_color);
		}
		Content&& SetStrokeColor(const Color& new_stroke_color)&& {
			return std::move(set_stroke_color_helper(new_stroke_color));
		}

		/*Setting stroke width*/
		Content& SetStrokeWidth(double new_stroke_width)& {
			return set_stroke_width_helper(new_stroke_width);
		}
		Content&& SetStrokeWidth(double new_stroke_width)&& {
			return std::move(set_stroke_width_helper(new_stroke_width));
		}

		/*Setting line parameters*/
		Content& SetStrokeLineCap(const std::string& new_line_cap)& {
			return set_line_cap_helper(new_line_cap);
		}
		Content&& SetStrokeLineCap(const std::string& new_line_cap)&& {
			return std::move(set_line_cap_helper(new_line_cap));
		}
		Content& SetStrokeLineJoin(const std::string& new_line_join)& {
			return set_line_join_helper(new_line_join);
		}
		Content&& SetStrokeLineJoin(const std::string& new_line_join)&& {
			return std::move(set_line_join_helper(new_line_join));
		}
	private:
		Content& set_fill_color_helper(const Color& new_fill_color) {
			fill_color = new_fill_color;
			return get_content();
		}
		Content& set_stroke_color_helper(const Color& new_stroke_color) {
			stroke_color = new_stroke_color;
			return get_content();
		}
		Content& set_stroke_width_helper(double new_stroke_width) {
			stroke_width = new_stroke_width;
			return get_content();
		}
		Content& set_line_cap_helper(const std::string& new_line_cap) {
			stroke_line_cap = new_line_cap;
			return get_content();
		}
		Content& set_line_join_helper(const std::string& new_line_join) {
			stroke_line_join = new_line_join;
			return get_content();
		}

		Content& get_content() noexcept {
			return static_cast<Content&>(*this);
		}
		const Content& get_content() const noexcept {
			return static_cast<const Content&>(*this);
		}
	protected:
		const FigureHolder<Content>& get_base() const noexcept {
			return *this;
		}
		static std::string escape_string(std::string value) {
#ifndef SVG_DEBUG
			return "\\\"" + value + "\\\"";
#else
			return "\"" + value + "\"";
#endif
		}
	private:
		Color fill_color{ NoneColor }, stroke_color{ NoneColor };
		double stroke_width{ 1.0 };
		std::optional<std::string> stroke_line_cap, stroke_line_join;
	};

	class Circle : public FigureHolder<Circle> {
	public:
		std::string Serialize() const;
	public:
		Circle& SetCenter(Point new_center)& {
			return set_center_helper(new_center);
		}
		Circle&& SetCenter(Point new_center)&& {
			return std::move(set_center_helper(new_center));
		}
		Circle& SetRadius(double new_radius)& {
			return set_radius_helper(new_radius);
		}
		Circle&& SetRadius(double new_radius)&& {
			return std::move(set_radius_helper(new_radius));
		}
	private:
		Circle& set_center_helper(Point new_center) {
			center = new_center;
			return *this;
		}
		Circle& set_radius_helper(double new_radius) {
			radius = new_radius;
			return *this;
		}
	private:
		Point center;
		double radius{ 1.0 };
	};

	class Polyline : public FigureHolder<Polyline> {
	public:
		std::string Serialize() const;
	public:
		Polyline& AddPoint(Point point)& {
			return add_point_helper(point);
		}
		Polyline&& AddPoint(Point point)&& {
			return std::move(add_point_helper(point));
		}
		Polyline& MergePoints(Polyline&& other)& {
			return merge_points_helper(std::move(other));
		}
		Polyline&& MergePoints(Polyline&& other)&& {
			return std::move(merge_points_helper(std::move(other)));
		}
	private:
		Polyline& add_point_helper(Point point) {
			path.push_back(point);
			return *this;
		}
		Polyline& merge_points_helper(Polyline&& other) {
			path.reserve(path.size() + other.path.size());
			path.insert(path.end(), other.path.begin(), other.path.end());
			return *this;
		}
		std::string collect_points() const;
	private:
		std::vector<Point> path;
	};

	class Text : public FigureHolder<Text> {
	public:
		std::string Serialize() const;
	public:
		/*Change text coordinates*/
		Text& SetPoint(Point point)& {
			return set_base_helper(point);
		}
		Text&& SetPoint(Point point)&& {
			return std::move(set_base_helper(point));
		}
		Text& SetOffset(Point point)& {
			return set_offset_helper(point);
		}
		Text&& SetOffset(Point point)&& {
			return std::move(set_offset_helper(point));
		}

		/*Setting font*/
		Text& SetFontSize(uint32_t new_font_size)& {
			return set_font_size_helper(new_font_size);
		}
		Text&& SetFontSize(uint32_t new_font_size)&& {
			return std::move(set_font_size_helper(new_font_size));
		}
		Text& SetFontFamily(const std::string& new_font_family)& {
			return set_font_family_helper(new_font_family);
		}
		Text&& SetFontFamily(const std::string& new_font_family)&& {
			return std::move(set_font_family_helper(new_font_family));
		}
		Text& SetFontWeight(const std::string& new_font_weight)& {
			return set_font_weight_helper(new_font_weight);
		}
		Text&& SetFontWeight(const std::string& new_font_weight)&& {
			return std::move(set_font_weight_helper(new_font_weight));
		}
		
		/*Update text data*/
		Text& SetData(const std::string& new_data)& {
			return set_data_helper(new_data);
		}
		Text&& SetData(const std::string& new_data)&& {
			return std::move(set_data_helper(new_data));
		}
	private:
		Text& set_base_helper(Point new_base_point) {
			base_point = new_base_point;
			return *this;
		}
		Text& set_offset_helper(Point new_offset) {
			offset = new_offset;
			return *this;
		}
		Text& set_font_size_helper(uint32_t new_font_size) {
			font_size = new_font_size;
			return *this;
		}
		Text& set_font_family_helper(const std::string& new_font_family) {
			font_family = new_font_family;
			return *this;
		}
		Text& set_font_weight_helper(const std::string& new_font_weight) {
			font_weight = new_font_weight;
			return *this;
		}
		Text& set_data_helper(const std::string& new_data) {
			data = new_data;
			return *this;
		}
	private:
		Point base_point, offset;
		uint32_t font_size{ 1 };
		std::optional<std::string> font_family, font_weight;
		std::string data;
	};


	class Document {
	public:
		using Object = std::variant
			<Circle,
			Polyline,
			Text>;
	public:
		/*Data update*/
		Document& Add(const Object& object)& {
			return add_helper(object);
		}
		Document& Add(Object&& object)& {
			return add_helper(std::move(object));
		}
		Document&& Add(const Object& object)&& {
			return std::move(add_helper(object));
		}
		Document&& Add(Object&& object)&& {
			return std::move(add_helper(std::move(object)));
		}

		/*Rendering*/
		std::string Render() const;
		void Render(std::ostream& out) const;
	private:
		Document& add_helper(Object object) {
			picture.push_back(std::move(object));
			return *this;
		}
		static std::string render_picture(const std::vector<Object>& picture);
	private:
		std::vector<Object> picture;

		static constexpr std::string_view
#ifndef SVG_DEBUG
			xml_header{ "<?xml version=\\\"1.0\\\" encoding=\\\"UTF-8\\\"?>" },
			svg_header{ "<svg xmlns=\\\"http://www.w3.org/2000/svg\\\" version=\\\"1.1\\\">" },
			svg_end{ "</svg>" };


#else
			xml_header{ "<?xml version=\"1.0\" encoding=\"UTF-8\"?>" },
			svg_header{ "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">" },
			svg_end{ "</svg>" };
#endif
			
	};
}