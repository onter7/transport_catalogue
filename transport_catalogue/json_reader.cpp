#include <algorithm>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "json_reader.h"

using namespace std::literals;

namespace transport_catalogue {

	namespace json_reader {

		JsonReader::JsonReader(std::istream& input)
			: doc_(json::Load(input)) {
		}

		void JsonReader::UpdateDatabase(TransportCatalogue& db) {
			const json::Dict& all_requests = doc_.GetRoot().AsMap();
			std::list<const json::Node*> stops;
			std::list<const json::Node*> buses;
			if (all_requests.count("base_requests"s)) {
				for (const auto& base_request : all_requests.at("base_requests"s).AsArray()) {
					const std::string& type = base_request.AsMap().at("type"s).AsString();
					if (type == "Stop"sv) {
						stops.push_back(&base_request);
					}
					else if (type == "Bus"sv) {
						buses.push_back(&base_request);
					}
					else {
						throw std::invalid_argument("Unknown base_request type: "s + type);
					}
				}
			}
			AddStops(db, stops);
			AddBuses(db, buses);
		}

		void JsonReader::AddStops(TransportCatalogue& db, const std::list<const json::Node*> stops) {
			for (const auto stop : stops) {
				const json::Dict& stop_dict = stop->AsMap();
				db.AddStop(stop_dict.at("name"s).AsString(), { stop_dict.at("latitude"s).AsDouble(), stop_dict.at("longitude"s).AsDouble() });
			}
			for (const auto stop : stops) {
				const json::Dict& stop_dict = stop->AsMap();
				const std::string_view from = stop_dict.at("name"s).AsString();
				if (stop_dict.count("road_distances"s)) {
					for (const auto& [to, distance] : stop_dict.at("road_distances"s).AsMap()) {
						db.SetDistanceBetweenStops(from, to, static_cast<std::size_t>(distance.AsInt()));
					}
				}
			}
		}

		void JsonReader::AddBuses(TransportCatalogue& db, const std::list<const json::Node*> buses) {
			for (const auto bus : buses) {
				const json::Dict& bus_dict = bus->AsMap();
				const json::Array& stops = bus_dict.at("stops"s).AsArray();
				std::vector<std::string_view> stop_names;
				stop_names.reserve(stops.size());
				for (const auto& stop : stops) {
					stop_names.push_back(stop.AsString());
				}
				const domain::BusType bus_type = bus_dict.at("is_roundtrip"s).AsBool()
					? domain::BusType::CIRCULAR
					: domain::BusType::DIRECT;
				db.AddBus(bus_type, bus_dict.at("name"s).AsString(), stop_names);
			}
		}

		void JsonReader::ProcessStat(const request_handler::RequestHandler& handler, std::ostream& output) {
			const json::Dict& all_requests = doc_.GetRoot().AsMap();
			json::Array response;
			if (all_requests.count("stat_requests"s)) {
				const json::Array& requests = all_requests.at("stat_requests"s).AsArray();
				response.reserve(requests.size());
				for (const auto& base_request : requests) {
					const json::Dict& request_dict = base_request.AsMap();
					const std::string& type = request_dict.at("type"s).AsString();
					if (type == "Stop"s) {
						response.push_back(GetStopStat(handler, request_dict));
					}
					else if (type == "Bus"s) {
						response.push_back(GetBusStat(handler, request_dict));
					}
					else {
						throw std::invalid_argument("Unknown stat_request type: "s + type);
					}
				}
			}
			json::Print(json::Document{ response }, output);
		}

		json::Dict JsonReader::GetStopStat(const request_handler::RequestHandler& handler, const json::Dict& stop_request) {
			const auto buses = handler.GetBusesByStop(stop_request.at("name"s).AsString());
			const int request_id = stop_request.at("id"s).AsInt();
			if (buses) {
				json::Array bus_names;
				bus_names.reserve(buses->size());
				for (const auto bus : *buses) {
					bus_names.push_back(bus->name);
				}
				std::sort(bus_names.begin(), bus_names.end(), [](const json::Node& lhs, const json::Node& rhs) {
					return lhs.AsString() < rhs.AsString();
				});
				return { { "buses"s, bus_names }, { "request_id"s, request_id } };
			}
			else {
				return { { "request_id"s, request_id }, { "error_message"s, "not found"s } };
			}
		}

		json::Dict JsonReader::GetBusStat(const request_handler::RequestHandler& handler, const json::Dict& bus_request) {
			const auto bus_stat = handler.GetBusStat(bus_request.at("name"s).AsString());
			const int request_id = bus_request.at("id"s).AsInt();
			if (bus_stat) {
				return
				{
					{ "curvature"s, (*bus_stat).curvature },
					{ "request_id"s, request_id },
					{ "route_length"s, static_cast<int>((*bus_stat).route_length) },
					{ "stop_count"s, static_cast<int>((*bus_stat).stops_on_route) },
					{ "unique_stop_count"s, static_cast<int>((*bus_stat).unique_stops) }
				};
			}
			else {
				return { { "request_id"s, request_id }, { "error_message"s, "not found"s } };
			}
		}

	}

}
