#pragma once

#include <cstddef>
#include <string>
#include <variant>
#include <vector>

#include "geo.h"

namespace transport_catalogue {

	namespace domain {

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

		struct RouteItem {
			double time = 0.0;
		};

		struct BusRouteItem : public RouteItem {
			std::string_view bus_name;
			std::size_t span_count = 0u;
		};

		struct WaitRouteItem : public RouteItem {
			std::string_view stop_name;
		};

		using Item = std::variant<BusRouteItem, WaitRouteItem>;

		struct RouteStat {
			double total_time = 0.0;
			std::vector<Item> items;
		};

	}

}
