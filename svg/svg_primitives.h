#pragma once
#include <cstdint>
#include <string>
#include <variant>
#include <optional>

namespace svg {
	struct Point {
		double
			x{ 0 },
			y{ 0 };
	};

	struct Rgb {
		uint8_t
			red{ 0 },
			green{ 0 },
			blue{ 0 };
	};

	struct Rgba : Rgb {
		double alpha{ 0 };
	};

	std::string rgb_to_string(Rgb rgba);
	
	std::string SerializeColor(const Rgb& rgb);
	std::string SerializeColor(const Rgba& rgba);
	std::string SerializeColor(const std::string& color);
	std::string SerializeColor(const std::monostate& color);

	class Color : std::variant
		<std::monostate, std::string, Rgb, Rgba> {
	public:
		using variant::variant;
		std::string ToString() const;

		const variant& get_base() const noexcept {
			return *this;
		};
	};

	inline const Color NoneColor;
}