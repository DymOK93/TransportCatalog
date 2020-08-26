#include "stats.h"

namespace stats {
	Distance& Distance::operator+=(Distance other) {
		geographic += other.geographic;
		real += other.real;
		return *this;
	}

	Distance& Distance::operator*=(double factor) {
		geographic *= factor;
		real *= factor;
		return *this;
	}
}