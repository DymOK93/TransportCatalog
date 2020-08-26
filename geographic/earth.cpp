#include "earth.h"
using namespace std;

namespace geographic {
	double calc_distance(Coordinates first, Coordinates second) {
		return acos(
			sin(degrees_to_radians(first.latitude)) * sin(degrees_to_radians(second.latitude)) +
			cos(degrees_to_radians(first.latitude)) * cos(degrees_to_radians(second.latitude)) *
			cos(abs(degrees_to_radians(first.longitude) - degrees_to_radians(second.longitude)))
		) * earth_radius;
	}
}