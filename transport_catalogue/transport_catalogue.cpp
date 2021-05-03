#include <algorithm>
#include <cstddef>
#include <unordered_set>

#include "transport_catalogue.h"

namespace transport_catalogue {

	namespace detail {

		std::ostream& operator<<(std::ostream& os, const BusInfo& bus_info) {
			using namespace std::literals;
			os << std::setprecision(6);
			return os << bus_info.stops_on_route << " stops on route, "s
				<< bus_info.unique_stops << " unique stops, "s
				<< bus_info.route_length << " route length, "s
				<< bus_info.curvature << " curvature"s;
		}

		std::ostream& operator<<(std::ostream& os, const StopInfo& stop_info) {
			using namespace std::literals;
			if (stop_info.bus_numbers.empty()) {
				os << "no buses"s;
			}
			else {
				os << "buses"s;
				for (const auto& bus_number : stop_info.bus_numbers) {
					os << " "s << bus_number;
				}
			}
			return os;
		}

	}

	void TransportCatalogue::AddStop(const std::string_view stop_name, const Coordinates& coordinates) {
		stops_.push_back({ std::string(stop_name), coordinates });
		const detail::Stop& stop = stops_.back();
		stop_name_to_stop_[stop.name] = &stop;
		stop_to_bus_numbers_[&stop];
	}

	void TransportCatalogue::AddBus(const detail::BusType type, const std::string_view bus_number, const std::list<std::string_view>& stop_names) {
		buses_.push_back({});
		detail::Bus& bus = buses_.back();
		bus.type = type;
		bus.number = std::string(bus_number);
		bus.stops_on_route = type == detail::BusType::CIRCULAR
			? stop_names.size()
			: stop_names.size() * 2 - 1;
		bus.unique_stops = std::unordered_set<std::string_view>(stop_names.begin(), stop_names.end()).size();
		bus.stops.reserve(stop_names.size());
		for (const auto& stop_name : stop_names) {
			const detail::Stop* stop = stop_name_to_stop_.at(stop_name);
			bus.stops.push_back(stop);
			stop_to_bus_numbers_[stop].insert(bus.number);
		}
		bus.geo_route_length = ComputeGeoRouteLength(bus);
		bus.actual_route_length = ComputeActualRouteLength(bus);
		bus_number_to_bus_[bus.number] = &bus;
	}

	const detail::Stop* TransportCatalogue::GetStop(const std::string_view stop_name) const {
		return stop_name_to_stop_.count(stop_name) ? stop_name_to_stop_.at(stop_name) : nullptr;
	}

	const detail::Bus* TransportCatalogue::GetBus(const std::string_view bus_number) const {
		return bus_number_to_bus_.count(bus_number) ? bus_number_to_bus_.at(bus_number) : nullptr;
	}

	std::optional<detail::BusInfo> TransportCatalogue::GetBusInfo(const std::string_view bus_number) const {
		if (!bus_number_to_bus_.count(bus_number)) {
			return std::nullopt;
		}
		const detail::Bus& bus = *bus_number_to_bus_.at(bus_number);
		return std::make_optional<detail::BusInfo>({ bus.stops_on_route, bus.unique_stops, bus.actual_route_length, bus.actual_route_length / bus.geo_route_length });
	}

	std::optional<detail::StopInfo> TransportCatalogue::GetStopInfo(const std::string_view stop_name) const {
		if (!stop_name_to_stop_.count(stop_name)) {
			return std::nullopt;
		}
		const detail::Stop* stop = stop_name_to_stop_.at(stop_name);
		const std::set<std::string_view> bus_numbers = stop_to_bus_numbers_.at(stop);
		detail::StopInfo result;
		result.bus_numbers.reserve(bus_numbers.size());
		for (const auto& bus_number : bus_numbers) {
			result.bus_numbers.push_back(bus_number);
		}
		return result;
	}

	void TransportCatalogue::SetDistanceBetweenStops(const std::string_view from, const std::string_view to, const std::size_t distance) {
		const detail::Stop* from_stop = GetStop(from);
		const detail::Stop* to_stop = GetStop(to);
		stop_pair_to_distance_[std::make_pair(from_stop, to_stop)] = distance;
	}

	std::size_t TransportCatalogue::GetDistanceBetweenStops(const detail::Stop* from, const detail::Stop* to) const {
		const auto pair = std::make_pair(from, to);
		if (stop_pair_to_distance_.count(pair)) {
			return stop_pair_to_distance_.at(pair);
		}
		return stop_pair_to_distance_.at(std::make_pair(to, from));
	}

	double TransportCatalogue::ComputeGeoRouteLength(const detail::Bus& bus) const {
		double result = 0.0;
		for (std::size_t i = 0; i + 1 < bus.stops.size(); ++i) {
			result += ComputeDistance(bus.stops[i]->coordinates, bus.stops[i + 1]->coordinates);
		}
		return bus.type == detail::BusType::CIRCULAR ? result : result * 2;
	}

	std::size_t TransportCatalogue::ComputeActualRouteLength(const detail::Bus& bus) const {
		std::size_t result = 0;
		const bool is_direct = bus.type == detail::BusType::DIRECT;
		for (std::size_t i = 0; i + 1 < bus.stops.size(); ++i) {
			result += GetDistanceBetweenStops(bus.stops[i], bus.stops[i + 1]);
			if (is_direct) {
				result += GetDistanceBetweenStops(bus.stops[i + 1], bus.stops[i]);
			}
		}
		return result;
	}

	std::size_t TransportCatalogue::StopPairHasher::operator()(const std::pair<const detail::Stop*, const detail::Stop*>& stop_pair) const {
		return hasher(stop_pair.first) + hasher(stop_pair.second) * PRIME_FACTOR;
	}
}