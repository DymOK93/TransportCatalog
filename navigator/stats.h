#pragma once

/*Iterator range*/
#include "range.h"

namespace stats {
	struct Distance {
		double geographic{ 0 },
			real{ 0 };

		Distance& operator+=(Distance other);
		Distance& operator*=(double factor);
	};

	struct Route {
		size_t
			stops{ 0 },
			unique_stops{ 0 };
		Distance distance;
	};

	template <typename It>
	struct Stop {
		Range<It> range;
	};
}