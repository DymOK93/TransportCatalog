#pragma once
#include "geographic.h"
#include <cmath>

namespace geographic {
	/*Constants section*/
	inline constexpr double
		pi_value{ 3.1415926535 },
		pi_degrees{ 180.0 },
		earth_radius{ 6'371'000.0 },
		kpmh_mpmin_coefficient{ 60.0 / 1000.0 };

	/*Auxiliary functions for unit conversion*/
	inline constexpr double degrees_to_radians(double degrees) noexcept {
		return degrees * pi_value / pi_degrees;
	}
	inline constexpr double kmph_to_mpmin(double kmph) noexcept {
		return kmph / kpmh_mpmin_coefficient;
	}
	inline constexpr double travel_time(double distance_in_metres, double mpmin) noexcept {
		return distance_in_metres / mpmin;
	}

	/*Distance calculation*/
	double calc_distance(
		Coordinates first,
		Coordinates second
	);
}