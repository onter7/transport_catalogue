#include <stdexcept>

#include "stat_reader.h"

namespace transport_catalogue {

	namespace stat {

		void ProcessStat(const TransportCatalogue& catalogue, std::istream& is, std::ostream& os) {
			using namespace std::literals;
			std::string query_count;
			std::getline(is, query_count);
			int n = std::stoi(query_count);
			std::vector<std::string> raw_query_lines(n);
			for (auto& query_line : raw_query_lines) {
				std::getline(is, query_line);
			}
			for (std::string_view query : raw_query_lines) {
				auto pos = query.find(' ');
				const std::string_view query_type = query.substr(0, pos);
				query.remove_prefix(pos + 1);
				if (query_type == "Bus"sv) {
					PrintBusInfo(query, catalogue, os);
				}
				else if (query_type == "Stop"sv) {
					PrintStopInfo(query, catalogue, os);
				}
				else {
					throw std::invalid_argument("Unknown query type: "s + std::string(query_type));
				}
			}

		}

		void PrintBusInfo(std::string_view bus_number, const TransportCatalogue& catalogue, std::ostream& os) {
			using namespace std::literals;
			os << "Bus "s << bus_number << ": "s;
			const auto bus_info = catalogue.GetBusInfo(bus_number);
			if (bus_info.has_value()) {
				os << bus_info.value();
			}
			else {
				os << "not found"s;
			}
			os << '\n';
		}

		void PrintStopInfo(std::string_view stop_name, const TransportCatalogue& catalogue, std::ostream& os) {
			using namespace std::literals;
			os << "Stop "s << stop_name << ": "s;
			const auto stop_info = catalogue.GetStopInfo(stop_name);
			if (stop_info.has_value()) {
				os << stop_info.value();
			}
			else {
				os << "not found"s;
			}
			os << '\n';
		}

	}

}