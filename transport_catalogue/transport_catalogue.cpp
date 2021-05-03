#include <iomanip>
#include <iterator>
#include <stdexcept>
#include <unordered_set>

#include "transport_catalogue.h"

void TransportCatalogue::AddStop(const Stop& stop) {
	stops_.push_back(stop);
	const Stop* added_stop = &stops_.back();
	stop_name_to_stop_[added_stop->name] = added_stop;
	stop_to_bus_numbers_[added_stop];
}

void TransportCatalogue::AddBus(const Bus& bus) {
	buses_.push_back(bus);
	const Bus* added_bus = &buses_.back();
	bus_number_to_bus_[added_bus->number] = added_bus;
	for (const std::string_view stop_name : added_bus->stop_names) {
		const Stop* stop = FindStop(stop_name);
		stop_to_bus_numbers_[stop].insert(added_bus->number);
	}
}

void TransportCatalogue::SetDistanceBetweenStops(const std::string_view from, const std::string_view to, const int distance) {
	const Stop* from_stop = FindStop(from);
	const Stop* to_stop = FindStop(to);
	stop_pair_to_distance_[std::make_pair(from_stop, to_stop)] = distance;
}

const Stop* TransportCatalogue::FindStop(const std::string_view stop_name) const {
	return stop_name_to_stop_.count(stop_name) ? stop_name_to_stop_.at(stop_name) : nullptr;
}

const Bus* TransportCatalogue::FindBus(const std::string_view bus_number) const {
	return bus_number_to_bus_.count(bus_number) ? bus_number_to_bus_.at(bus_number) : nullptr;
}

int TransportCatalogue::GetDistanceBetweenStops(const Stop* from, const Stop* to) const {
	const auto pair = std::make_pair(from, to);
	if (stop_pair_to_distance_.count(pair)) {
		return stop_pair_to_distance_.at(pair);
	}
	return stop_pair_to_distance_.at(std::make_pair(to, from));
}

std::optional<BusInfo> TransportCatalogue::GetBusInfo(const std::string_view bus_number) const {
	const Bus* bus = FindBus(bus_number);
	if (!bus) {
		return std::nullopt;
	}
	const bool is_circular = bus->type == BusType::CIRCULAR;
	const double geo_route_length = ComputeGeoRouteLength(bus);
	const std::size_t stops_on_route = is_circular
		? bus->stop_names.size()
		: bus->stop_names.size() * 2 - 1;
	const std::size_t unique_stops = std::unordered_set<std::string_view>{ bus->stop_names.begin(), bus->stop_names.end() }.size();
	const int actual_route_length = ComputeActualRouteLength(bus);
	return BusInfo{ stops_on_route, unique_stops, actual_route_length, actual_route_length / geo_route_length };
}

const std::set<std::string_view>* TransportCatalogue::GetBusListForStop(const std::string_view stop_name) const {
	const Stop* stop = FindStop(stop_name);
	if (!stop) {
		return nullptr;
	}
	return &stop_to_bus_numbers_.at(stop);
}

double TransportCatalogue::ComputeGeoRouteLength(const Bus* bus) const {
	double result = 0.0;
	const auto last_it = bus->stop_names.empty()
		? bus->stop_names.end()
		: std::prev(bus->stop_names.end());
	for (auto it = bus->stop_names.begin(); it != last_it; ++it) {
		const Stop* from = stop_name_to_stop_.at(*it);
		const Stop* to = stop_name_to_stop_.at(*std::next(it));
		result += ComputeDistance(from->coordinates, to->coordinates);
	}
	return bus->type == BusType::CIRCULAR ? result : result * 2;
}

int TransportCatalogue::ComputeActualRouteLength(const Bus* bus) const {
	std::size_t result = 0;
	const bool is_direct_route = bus->type == BusType::DIRECT;
	const auto last_it = bus->stop_names.empty()
		? bus->stop_names.end()
		: std::prev(bus->stop_names.end());
	for (auto it = bus->stop_names.begin(); it != last_it; ++it) {
		const Stop* from = stop_name_to_stop_.at(*it);
		const Stop* to = stop_name_to_stop_.at(*std::next(it));
		result += GetDistanceBetweenStops(from, to);
		if (is_direct_route) {
			result += GetDistanceBetweenStops(to, from);
		}
	}
	return result;
}

std::ostream& operator<<(std::ostream& os, const BusInfo& bus_info) {
	using namespace std::literals;
	os << std::setprecision(6);
	return os << bus_info.stops_on_route << " stops on route, "s
		<< bus_info.unique_stops << " unique stops, "s
		<< bus_info.route_length << " route length, "s
		<< bus_info.curvature << " curvature"s;
}