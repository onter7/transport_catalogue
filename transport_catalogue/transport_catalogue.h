#pragma once

#include <cstddef>
#include <deque>
#include <iostream>
#include <iomanip>
#include <list>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <vector>
#include <unordered_map>

#include "geo.h"

namespace transport_catalogue {

	namespace detail {

		enum class BusType {
			CIRCULAR,
			DIRECT
		};

		struct Stop {
			std::string name;
			Coordinates coordinates;
		};

		struct Bus {
			BusType type;
			std::string number;
			std::vector<const Stop*> stops;
			std::size_t stops_on_route;
			std::size_t unique_stops;
			double geo_route_length;
			std::size_t actual_route_length;
		};

		struct BusInfo {
			std::size_t stops_on_route;
			std::size_t unique_stops;
			std::size_t route_length;
			double curvature;
		};

		struct StopInfo {
			std::vector<std::string_view> bus_numbers;
		};

		std::ostream& operator<<(std::ostream& os, const BusInfo& bus_info);

		std::ostream& operator<<(std::ostream& os, const StopInfo& stop_info);

	}

	class TransportCatalogue {
	public:
		void AddStop(const std::string_view stop_name, const Coordinates& coordinates);
		void AddBus(const detail::BusType type, const std::string_view bus_number, const std::list<std::string_view>& stop_names);
		const detail::Stop* GetStop(const std::string_view stop_name) const;
		const detail::Bus* GetBus(const std::string_view bus_number) const;
		std::optional<detail::BusInfo> GetBusInfo(const std::string_view bus_number) const;
		std::optional<detail::StopInfo> GetStopInfo(const std::string_view stop_name) const;
		void SetDistanceBetweenStops(const std::string_view from, const std::string_view to, const std::size_t distance);
		std::size_t GetDistanceBetweenStops(const detail::Stop* from, const detail::Stop* to) const;
	private:
		struct StopPairHasher {
			std::size_t operator()(const std::pair<const detail::Stop*, const detail::Stop*>& stop_pair) const;
			static const std::uint64_t PRIME_FACTOR = 37;
			std::hash<const void*> hasher;
		};

		std::deque<detail::Stop> stops_;
		std::deque<detail::Bus> buses_;
		std::unordered_map<std::string_view, const detail::Stop*> stop_name_to_stop_;
		std::unordered_map<std::string_view, const detail::Bus*> bus_number_to_bus_;
		std::unordered_map<const detail::Stop*, std::set<std::string_view>> stop_to_bus_numbers_;
		std::unordered_map <std::pair<const detail::Stop*, const detail::Stop*>, std::size_t, StopPairHasher> stop_pair_to_distance_;

		double ComputeGeoRouteLength(const detail::Bus& bus) const;
		std::size_t ComputeActualRouteLength(const detail::Bus& bus) const;
	};

}