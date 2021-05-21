#pragma once

#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "geo.h"

namespace transport_catalogue {

	namespace domain {

		struct Stop;
		struct Bus;		

		enum class BusType {
			CIRCULAR,
			DIRECT
		};

		struct Stop {
			std::string name;
			geo::Coordinates coordinates;
		};

		struct Bus {
			BusType type;
			std::string name;
			std::vector<const Stop*> stops;
		};

		struct BusStat {
			std::size_t stops_on_route;
			std::size_t unique_stops;
			std::size_t route_length;
			double curvature;
		};

	}

}
