#pragma once

#include <cstddef>
#include <deque>
#include <iostream>
#include <list>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <unordered_map>

#include "geo.h"

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
	std::list<std::string> stop_names;
};

struct BusInfo {
	std::size_t stops_on_route;
	std::size_t unique_stops;
	int route_length;
	double curvature;
};

class TransportCatalogue {
public:
	explicit TransportCatalogue() = default;
	void AddStop(const Stop& stop);
	void AddBus(const Bus& bus);
	void SetDistanceBetweenStops(const std::string_view from, const std::string_view to, const int distance);
	const Stop* FindStop(const std::string_view stop_name) const;
	const Bus* FindBus(const std::string_view bus_number) const;
	std::optional<BusInfo> GetBusInfo(const std::string_view bus_number) const;
	const std::set<std::string_view>* GetBusListForStop(const std::string_view stop_name) const;
	int GetDistanceBetweenStops(const Stop* from, const Stop* to) const;
private:
	struct StopPairHasher {
		std::size_t operator()(const std::pair<const Stop*, const Stop*>& stop_pair) const {
			return ptr_hasher_(stop_pair.first) + ptr_hasher_(stop_pair.second) * PRIME_FACTOR;
		}
	private:
		static const std::uint64_t PRIME_FACTOR = 37;
		std::hash<const void*> ptr_hasher_;
	};

	std::deque<Stop> stops_;
	std::deque<Bus> buses_;
	std::unordered_map<std::string_view, const Stop*> stop_name_to_stop_;
	std::unordered_map<std::string_view, const Bus*> bus_number_to_bus_;	
	std::unordered_map<const Stop*, std::set<std::string_view>> stop_to_bus_numbers_;
	std::unordered_map <std::pair<const Stop*, const Stop*>, int, StopPairHasher> stop_pair_to_distance_;

	double ComputeGeoRouteLength(const Bus* bus) const;
	int ComputeActualRouteLength(const Bus* bus) const;
};

std::ostream& operator<<(std::ostream& os, const BusInfo& bus_info);
