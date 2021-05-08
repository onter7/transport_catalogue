#include <stdexcept>

#include "input_reader.h"

namespace transport_catalogue {

	namespace input {

		Queries ReadInput(std::istream& is) {
			using namespace std::literals;
			std::string query_count;
			std::getline(is, query_count);
			int n = std::stoi(query_count);
			Queries result;
			result.raw_query_lines.resize(n);
			for (auto& query_line : result.raw_query_lines) {
				std::getline(is, query_line);
				std::string_view query(query_line);
				auto pos = query.find(' ');
				const std::string_view query_type = query.substr(0, pos);
				query.remove_prefix(pos + 1);
				if (query_type == "Stop"sv) {
					const Stop stop = ParseStop(query);
					result.stops.push_back(stop);
				}
				else if (query_type == "Bus"sv) {
					const Bus bus = ParseBus(query);
					result.buses.push_back(bus);
				}
				else {
					throw std::invalid_argument("Unknown query type: "s + std::string(query_type));
				}
			}
			return result;
		}

		Stop ParseStop(std::string_view stop_query) {
			Stop stop;
			auto pos = stop_query.find(':');
			const std::string_view name = stop_query.substr(0, pos);
			stop.name = name;
			stop_query.remove_prefix(pos + 2);
			pos = stop_query.find(',');
			const std::string_view lat = stop_query.substr(0, pos);
			stop.coordintes.lat = std::stod(std::string(lat));
			stop_query.remove_prefix(pos + 2);
			pos = stop_query.find(',');
			const std::string_view lng = stop_query.substr(0, pos);
			stop.coordintes.lng = std::stod(std::string(lng));
			if (pos != std::string_view::npos) {
				stop_query.remove_prefix(pos + 2);
				while (!stop_query.empty()) {
					pos = stop_query.find(',');
					auto stop_name_distance = ParseDistance(stop_query.substr(0, pos));
					stop.distances.push_back(std::move(stop_name_distance));
					stop_query.remove_prefix(pos == std::string_view::npos ? stop_query.size() : pos + 2);
				}
			}
			return stop;
		}

		Bus ParseBus(std::string_view bus_query) {
			Bus bus;
			auto pos = bus_query.find(':');
			const std::string_view number = bus_query.substr(0, pos);
			bus.number = number;
			bus_query.remove_prefix(pos + 2);
			const bool is_circular = bus_query.find('>') != std::string_view::npos;
			bus.type = is_circular ? detail::BusType::CIRCULAR : detail::BusType::DIRECT;
			const char delimeter = is_circular ? '>' : '-';
			while (true) {
				pos = bus_query.find(delimeter);
				if (pos == std::string_view::npos) {
					bus.stop_names.push_back(bus_query);
					break;
				}
				bus.stop_names.push_back(bus_query.substr(0, pos - 1));
				bus_query.remove_prefix(pos + 2);
			}
			return bus;
		}

		std::pair<std::string_view, std::size_t> ParseDistance(std::string_view distance_query) {
			auto pos = distance_query.find('m');
			const std::size_t distance = std::stoi(std::string(distance_query.substr(0, pos)));
			distance_query.remove_prefix(pos + 5);
			return { distance_query, distance };
		}

		void UpdateDatabase(const Queries& queries, TransportCatalogue& catalogue) {
			for (const auto& stop : queries.stops) {
				catalogue.AddStop(stop.name, stop.coordintes);
			}
			for (const auto& stop : queries.stops) {
				for (const auto& [to, distance] : stop.distances) {
					catalogue.SetDistanceBetweenStops(stop.name, to, distance);
				}
			}
			for (const auto& bus : queries.buses) {
				catalogue.AddBus(bus.type, bus.number, { bus.stop_names.begin(), bus.stop_names.end() });
			}
		}

	}

}