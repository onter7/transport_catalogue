#include <vector>
#include <stdexcept>
#include <string>
#include <utility>
#include <unordered_set>

#include "input_reader.h"

InputQueries ReadInputQueries(std::istream& is) {
	using namespace std::literals;
	std::string query_count;
	std::getline(is, query_count);
	int n = std::stoi(query_count);
	InputQueries result;
	while (n--) {
		std::string query;
		std::getline(is, query);
		std::string_view query_sv(query);
		const auto space_pos = query_sv.find(' ');
		const std::string_view query_type = query_sv.substr(0, space_pos);
		query_sv.remove_prefix(space_pos + 1);
		if (query_type == "Stop"sv) {
			StopQuery stop_query = ParseStopQuery(query_sv);
			result.stop_queries.push_back(std::move(stop_query));
		}
		else if (query_type == "Bus"sv) {
			Bus bus = ParseBusQuery(query_sv);
			result.bus_queries.push_back(std::move(bus));
		}
		else {
			throw std::invalid_argument("Unknow query type: "s + std::string(query_type));
		}
	}
	return result;
}

std::pair<std::string, std::size_t> ParseDistance(std::string_view stop_name_distance) {
	auto pos = stop_name_distance.find('m');
	const std::size_t distance = std::stoi(std::string(stop_name_distance.substr(0, pos)));
	stop_name_distance.remove_prefix(pos + 5);
	return { std::string(stop_name_distance), distance };
}

StopQuery ParseStopQuery(std::string_view stop_query) {
	Stop stop;
	auto pos = stop_query.find(':');
	stop.name = stop_query.substr(0, pos);
	stop_query.remove_prefix(pos + 2);
	pos = stop_query.find(',');
	stop.coordinates.lat = std::stod(std::string(stop_query.substr(0, pos)));
	stop_query.remove_prefix(pos + 2);
	pos = stop_query.find(',');
	stop.coordinates.lng = std::stod(std::string(stop_query.substr(0, pos)));
	stop_query.remove_prefix(pos == std::string_view::npos ? stop_query.size() : pos + 2);
	std::list<std::pair<std::string, std::size_t>> distances;
	while (!stop_query.empty()) {
		pos = stop_query.find(',');
		auto stop_name_distance = ParseDistance(stop_query.substr(0, pos));
		distances.push_back(std::move(stop_name_distance));
		stop_query.remove_prefix(pos == std::string_view::npos ? stop_query.size() : pos + 2);
	}
	return { std::move(stop), std::move(distances) };
}

Bus ParseBusQuery(std::string_view bus_query) {
	Bus bus;
	auto pos = bus_query.find(':');
	bus.number = bus_query.substr(0, pos);
	bus_query.remove_prefix(pos + 2);
	const bool is_circular = bus_query.find('-') == std::string_view::npos;
	bus.type = is_circular
		? BusType::CIRCULAR
		: BusType::DIRECT;
	const char delimeter = is_circular ? '>' : '-';
	while (true) {
		pos = bus_query.find(delimeter);
		if (pos == std::string_view::npos) {
			bus.stop_names.push_back(std::string(bus_query));
			break;
		}
		const std::string_view stop_name = bus_query.substr(0, pos - 1);
		bus.stop_names.push_back(std::string(stop_name));
		bus_query.remove_prefix(pos + 2);
	}
	return bus;
}

void UpdateDatabase(const InputQueries& input_queries, TransportCatalogue& tranport_catalogue) {
	for (const auto& stop_query : input_queries.stop_queries) {
		tranport_catalogue.AddStop(stop_query.stop);
	}
	for (const auto& stop_query : input_queries.stop_queries) {
		for (const auto& [stop_name, distance] : stop_query.distances) {
			tranport_catalogue.SetDistanceBetweenStops(stop_query.stop.name, stop_name, distance);
		}
	}
	for (const Bus& bus : input_queries.bus_queries) {
		tranport_catalogue.AddBus(bus);
	}
}