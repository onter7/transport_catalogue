#include <exception>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "stat_reader.h"

void ProcessStatQueries(const TransportCatalogue& transport_catalogue, std::istream& is, std::ostream& os) {
	using namespace std::literals;
	std::string query_count;
	std::getline(is, query_count);
	int n = std::stoi(query_count);
	std::vector<std::string> queries(n);
	for (auto& query : queries) {
		std::getline(is, query);
	}
	for (std::string_view query : queries) {
		const auto space_pos = query.find(' ');
		const std::string_view query_type = query.substr(0, space_pos);
		query.remove_prefix(space_pos + 1);
		if (query_type == "Bus"sv) {
			os << "Bus "s << query << ": "s;
			const std::optional<BusInfo> bus_info = transport_catalogue.GetBusInfo(query);
			if (bus_info.has_value()) {				
				os << bus_info.value();
			}
			else {
				os << "not found"s;
			}
			os << '\n';
		}
		else if (query_type == "Stop"sv) {
			os << "Stop "s << query << ": "s;
			const auto bus_numbers = transport_catalogue.GetBusListForStop(query);
			if (bus_numbers) {
				if (bus_numbers->empty()) {
					os << "no buses"s;
				}
				else {
					os << "buses"s;
					for (const auto bus_number : *bus_numbers) {
						os << " "s << bus_number;
					}
				}
			}
			else {
				os << "not found"s;
			}
			os << '\n';
		}
		else {
			throw std::invalid_argument("Unkown query type: "s + std::string(query_type));
		}
	}
}