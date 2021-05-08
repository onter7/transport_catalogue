#pragma once

#include <cstddef>
#include <iostream>
#include <list>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "geo.h"
#include "transport_catalogue.h"

namespace transport_catalogue {

	namespace input {

		struct Stop {
			std::string_view name;
			geo::Coordinates coordintes;
			std::list<std::pair<std::string_view, std::size_t>> distances;
		};

		struct Bus {
			detail::BusType type;
			std::string_view number;
			std::list<std::string_view> stop_names;
		};

		struct Queries {
			std::list<Stop> stops;
			std::list<Bus> buses;
			std::vector<std::string> raw_query_lines;
		};

		Queries ReadInput(std::istream& is = std::cin);

		Stop ParseStop(std::string_view stop_query);

		Bus ParseBus(std::string_view bus_query);

		std::pair<std::string_view, std::size_t> ParseDistance(std::string_view distance_query);

		void UpdateDatabase(const Queries& queries, TransportCatalogue& catalogue);

	}

}
