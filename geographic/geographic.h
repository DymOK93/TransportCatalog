#pragma once
#include "range.h"

#include <unordered_map>
#include <variant>
#include <string>
#include <vector>
#include <set>

namespace geographic {
	struct Coordinates {
			double
				latitude{ 0 },
				longitude{ 0 };
	};
		
	using DistanceList = std::unordered_map<std::string_view, uint64_t>;
	
	struct Stop {
		std::string_view name;
		Coordinates coordinates;
		DistanceList distances;
	};

	struct Bus {
		std::string_view name;
		std::vector<std::string_view> stops;
		bool is_roundtrip{ false };							//Circle route
	};

}
