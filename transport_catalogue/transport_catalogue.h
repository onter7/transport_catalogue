#pragma once

#include <cstddef>
#include <deque>
#include <functional>
#include <list>
#include <optional>
#include <string_view>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "geo.h"
#include "domain.h"

namespace transport_catalogue {

	namespace detail {

		struct StopPairHasher {
			std::size_t operator()(const std::pair<const domain::Stop*, const domain::Stop*>& stop_pair) const;
			std::hash<const void*> hasher;
		};

	}

	class TransportCatalogue {
	public:
		void AddStop(const std::string_view stop_name, const geo::Coordinates& coordinates);
		void AddBus(const domain::BusType type, const std::string_view bus_name, const std::vector<std::string_view>& stop_names);
		const domain::Stop* GetStop(const std::string_view stop_name) const;
		const domain::Bus* GetBus(const std::string_view bus_name) const;
		std::vector<std::pair<const domain::Stop*, std::size_t>> GetStopsToBusCounts() const;
		std::vector<const domain::Stop*> GetStops() const;
		std::vector<const domain::Bus*> GetBuses() const;
		std::optional<domain::BusStat> GetBusStat(const std::string_view bus_name) const;
		const std::unordered_set<const domain::Bus*>* GetBusesByStop(const std::string_view stop_name) const;
		void SetDistanceBetweenStops(const std::string_view from, const std::string_view to, const std::size_t distance_m);
		std::size_t GetDistanceBetweenStops(const domain::Stop* from, const domain::Stop* to) const;
	private:
		std::deque<domain::Stop> stops_;
		std::deque<domain::Bus> buses_;
		std::unordered_map<std::string_view, const domain::Stop*> stop_name_to_stop_;
		std::unordered_map<std::string_view, const domain::Bus*> bus_name_to_bus_;
		std::unordered_map<const domain::Stop*, std::unordered_set<const domain::Bus*>> stop_to_buses_;
		std::unordered_map<std::pair<const domain::Stop*, const domain::Stop*>, std::size_t, detail::StopPairHasher> stop_pair_to_distance_;

		double ComputeGeoRouteLength(const domain::Bus& bus) const;
		std::size_t ComputeActualRouteLength(const domain::Bus& bus) const;
	};

}