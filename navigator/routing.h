#pragma once

/*Standart headers*/
#include <optional>
#include <tuple>
#include <vector>
#include <string_view>
#include <cstdint>


namespace routing {
	struct Parameters {
		uint64_t bus_wait_time{ 0 };
		double bus_velocity{ 0 };
	};

	struct Bounds {
		std::string_view from,
			to;
	};

	struct Point {
		enum class Type {
			WAIT,
			BUS
		};

		const Type type;
		std::string_view name;
		double time{ 0 };
		std::optional<uint64_t> span_count;
	};

	using Way = std::vector<Point>;

	struct OnMap {
		double total_time{ 0 };
		Way items;
	};
}

