#include <algorithm>

#include "transport_catalogue.h"

namespace transport_catalogue {

	void TransportCatalogue::AddStop(const std::string_view stop_name, const geo::Coordinates& coordinates) {
		stops_.push_back({ std::string(stop_name), coordinates });
		const domain::Stop& stop = stops_.back();
		stop_name_to_stop_[stop.name] = &stop;
		stop_to_buses_[&stop];
	}

	void TransportCatalogue::AddBus(const domain::BusType type, const std::string_view bus_name, const std::vector<std::string_view>& stop_names) {
		buses_.push_back({ type, std::string(bus_name), {} });
		domain::Bus& bus = buses_.back();
		bus.stops.reserve(stop_names.size());
		for (const auto& stop_name : stop_names) {
			const domain::Stop* stop = stop_name_to_stop_.at(stop_name);
			bus.stops.push_back(stop);
			stop_to_buses_[stop].insert(&bus);
		}
		bus_name_to_bus_[bus.name] = &bus;
	}

	const domain::Stop* TransportCatalogue::GetStop(const std::string_view stop_name) const {
		return stop_name_to_stop_.count(stop_name) ? stop_name_to_stop_.at(stop_name) : nullptr;
	}

	const domain::Bus* TransportCatalogue::GetBus(const std::string_view bus_name) const {
		return bus_name_to_bus_.count(bus_name) ? bus_name_to_bus_.at(bus_name) : nullptr;
	}

	std::vector<std::pair<const domain::Stop*, std::size_t>> TransportCatalogue::GetStopsToBusCounts() const {
		std::vector<std::pair<const domain::Stop*, std::size_t>> result;
		result.reserve(stops_.size());
		for (const auto [stop, buses] : stop_to_buses_) {
			result.push_back(std::make_pair(stop, buses.size()));
		}
		return result;
	}

	std::vector<const domain::Bus*> TransportCatalogue::GetBuses() const {
		std::vector<const domain::Bus*> result;
		result.reserve(buses_.size());
		for (const auto& bus : buses_) {
			result.push_back(&bus);
		}
		return result;
	}

	std::optional<domain::BusStat> TransportCatalogue::GetBusStat(const std::string_view bus_name) const {
		if (!bus_name_to_bus_.count(bus_name)) {
			return std::nullopt;
		}
		const domain::Bus& bus = *bus_name_to_bus_.at(bus_name);
		const std::size_t stops_on_route = bus.type == domain::BusType::CIRCULAR
			? bus.stops.size()
			: bus.stops.size() * 2 - 1;
		std::unordered_set<std::string_view> unique_stops;
		for (const domain::Stop* stop : bus.stops) {
			unique_stops.insert(std::string_view(stop->name));
		}
		const double geo_route_length = ComputeGeoRouteLength(bus);
		const std::size_t actual_route_length = ComputeActualRouteLength(bus);
		return std::make_optional<domain::BusStat>({ stops_on_route, unique_stops.size(), actual_route_length, actual_route_length / geo_route_length });
	}

	const std::unordered_set<const domain::Bus*>* TransportCatalogue::GetBusesByStop(const std::string_view stop_name) const {
		if (!stop_name_to_stop_.count(stop_name)) {
			return nullptr;
		}
		return &stop_to_buses_.at(stop_name_to_stop_.at(stop_name));
	}

	void TransportCatalogue::SetDistanceBetweenStops(const std::string_view from, const std::string_view to, const std::size_t distance) {
		const domain::Stop* from_stop = GetStop(from);
		const domain::Stop* to_stop = GetStop(to);
		stop_pair_to_distance_[std::make_pair(from_stop, to_stop)] = distance;
	}

	std::size_t TransportCatalogue::GetDistanceBetweenStops(const domain::Stop* from, const domain::Stop* to) const {
		const auto pair = std::make_pair(from, to);
		if (stop_pair_to_distance_.count(pair)) {
			return stop_pair_to_distance_.at(pair);
		}
		return stop_pair_to_distance_.at(std::make_pair(to, from));
	}

	double TransportCatalogue::ComputeGeoRouteLength(const domain::Bus& bus) const {
		double result = 0.0;
		for (std::size_t i = 0; i + 1 < bus.stops.size(); ++i) {
			result += ComputeDistance(bus.stops[i]->coordinates, bus.stops[i + 1]->coordinates);
		}
		return bus.type == domain::BusType::CIRCULAR ? result : result * 2;
	}

	std::size_t TransportCatalogue::ComputeActualRouteLength(const domain::Bus& bus) const {
		std::size_t result = 0;
		const bool is_direct = bus.type == domain::BusType::DIRECT;
		for (std::size_t i = 0; i + 1 < bus.stops.size(); ++i) {
			result += GetDistanceBetweenStops(bus.stops[i], bus.stops[i + 1]);
			if (is_direct) {
				result += GetDistanceBetweenStops(bus.stops[i + 1], bus.stops[i]);
			}
		}
		return result;
	}

	std::size_t detail::StopPairHasher::operator()(const std::pair<const domain::Stop*, const domain::Stop*>& stop_pair) const {
		return hasher(stop_pair.first) + hasher(stop_pair.second) * 37;
	}
}