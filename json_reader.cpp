#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <fstream>
#include <ios>
#include <sstream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>
#include <utility>

#include "json_reader.h"
#include "json_builder.h"
#include "serialization.h"

using namespace std::literals;

namespace transport_catalogue {

	namespace json_reader {

		json::Dict RouteItemConverter::operator()(const domain::BusRouteItem& bus) const {
			return
				json::Builder{}
				.StartDict()
				.Key("type"s).Value("Bus"s)
				.Key("bus"s).Value(std::string(bus.bus_name))
				.Key("span_count"s).Value(static_cast<int>(bus.span_count))
				.Key("time"s).Value(bus.time)
				.EndDict()
				.Build().AsDict();
		}

		json::Dict RouteItemConverter::operator()(const domain::WaitRouteItem& wait) const {
			return
				json::Builder{}
				.StartDict()
				.Key("type"s).Value("Wait"s)
				.Key("stop_name"s).Value(std::string(wait.stop_name))
				.Key("time"s).Value(wait.time)
				.EndDict()
				.Build().AsDict();
		}

		using JsonResponse = std::variant<NotFound, Map, StopStat, BusStat, RouteStat>;

		json::Dict ResponseConverter::operator()(const NotFound& response) const {
			return
				json::Builder{}
				.StartDict()
				.Key("request_id"s).Value(response.request_id)
				.Key("error_message"s).Value("not found"s)
				.EndDict()
				.Build().AsDict();
		}

		json::Dict ResponseConverter::operator()(const Map& response) const {
			std::stringstream ss;
			response.doc.Render(ss);
			return
				json::Builder{}
				.StartDict()
				.Key("map"s).Value(std::move(ss.str()))
				.Key("request_id"s).Value(response.request_id)
				.EndDict()
				.Build().AsDict();
		}

		json::Dict ResponseConverter::operator()(const StopStat& response) const {
			if (response.buses) {
				json::Array bus_names;
				bus_names.reserve(response.buses->size());
				for (const auto bus : *response.buses) {
					bus_names.push_back(bus->name);
				}
				std::sort(bus_names.begin(), bus_names.end(),
					[](const json::Node& lhs, const json::Node& rhs) {
						return lhs.AsString() < rhs.AsString();
					}
				);
				return
					json::Builder{}
					.StartDict()
					.Key("buses"s).Value(bus_names)
					.Key("request_id"s).Value(response.request_id)
					.EndDict()
					.Build().AsDict();
			}
			else {
				return std::visit(ResponseConverter{}, JsonResponse{ NotFound{ response.request_id } });
			}
		}

		json::Dict ResponseConverter::operator()(const BusStat& response) const {
			if (response.bus_stat) {
				return
					json::Builder{}
					.StartDict()
					.Key("curvature"s).Value((*response.bus_stat).curvature)
					.Key("request_id"s).Value(response.request_id)
					.Key("route_length"s).Value(static_cast<int>((*response.bus_stat).route_length_m))
					.Key("stop_count"s).Value(static_cast<int>((*response.bus_stat).stops_on_route))
					.Key("unique_stop_count"s).Value(static_cast<int>((*response.bus_stat).unique_stops))
					.EndDict()
					.Build().AsDict();
			}
			else {
				return std::visit(ResponseConverter{}, JsonResponse{ NotFound{ response.request_id } });
			}
		}

		json::Dict ResponseConverter::operator()(const RouteStat& response) const {
			if (response.route_stat) {
				json::Array items_array;
				const auto& items = response.route_stat.value().items;
				items_array.reserve(items.size());
				for (const auto& item : items) {
					items_array.push_back(std::visit(RouteItemConverter{}, item));
				}
				return
					json::Builder{}
					.StartDict()
					.Key("request_id"s).Value(response.request_id)
					.Key("total_time"s).Value(response.route_stat.value().total_time_min)
					.Key("items"s).Value(items_array)
					.EndDict()
					.Build().AsDict();
			}
			else {
				return std::visit(ResponseConverter{}, JsonResponse{ NotFound{ response.request_id } });
			}
		}

		JsonReader::JsonReader(request_handler::RequestHandler& handler)
			: handler_(handler) {
		}

		void JsonReader::UpdateDatabase(const json::Document& doc) {
			const json::Dict& all_requests = doc.GetRoot().AsDict();
			std::list<const json::Node*> stops;
			std::list<const json::Node*> buses;
			if (all_requests.count("base_requests"s)) {
				for (const auto& base_request : all_requests.at("base_requests"s).AsArray()) {
					const std::string& type = base_request.AsDict().at("type"s).AsString();
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
			AddStops(stops);
			AddBuses(buses);
		}

		void JsonReader::AddStops(const std::list<const json::Node*>& stop_nodes) {
			for (const auto stop : stop_nodes) {
				const json::Dict& stop_dict = stop->AsDict();
				handler_.AddStop(stop_dict.at("name"s).AsString(), { stop_dict.at("latitude"s).AsDouble(), stop_dict.at("longitude"s).AsDouble() });
			}
			for (const auto stop : stop_nodes) {
				const json::Dict& stop_dict = stop->AsDict();
				const std::string_view from = stop_dict.at("name"s).AsString();
				if (stop_dict.count("road_distances"s)) {
					for (const auto& [to, distance_m] : stop_dict.at("road_distances"s).AsDict()) {
						handler_.SetDistanceBetweenStops(from, to, static_cast<std::size_t>(distance_m.AsInt()));
					}
				}
			}
		}

		void JsonReader::AddBuses(const std::list<const json::Node*>& bus_nodes) {
			for (const auto bus : bus_nodes) {
				const json::Dict& bus_dict = bus->AsDict();
				const json::Array& stops = bus_dict.at("stops"s).AsArray();
				std::vector<std::string_view> stop_names;
				stop_names.reserve(stops.size());
				for (const auto& stop : stops) {
					stop_names.push_back(stop.AsString());
				}
				const domain::BusType bus_type = bus_dict.at("is_roundtrip"s).AsBool()
					? domain::BusType::CIRCULAR
					: domain::BusType::DIRECT;
				handler_.AddBus(bus_type, bus_dict.at("name"s).AsString(), stop_names);
			}
		}

		void JsonReader::MakeBase(std::istream& input) {
			const json::Document doc{ json::Load(input) };
			UpdateDatabase(doc);
			const json::Dict& all_requests = doc.GetRoot().AsDict();
			if (all_requests.count("render_settings"s)) {
				handler_.SetRenderSettings(GetRenderSettings(all_requests.at("render_settings"s).AsDict()));
			}
			if (all_requests.count("routing_settings"s)) {
				handler_.SetRoutingSettings(GetRoutingSettings(all_requests.at("routing_settings"s).AsDict()));
			}
			if (all_requests.count("serialization_settings"s)) {
				const std::string file_name = all_requests.at("serialization_settings"s).AsDict().at("file"s).AsString();
				SerializeTransportCatalogue(file_name);
			}
		}

		void JsonReader::ProcessRequests(std::istream& input, std::ostream& output) {
			const json::Document doc{ json::Load(input) };
			const json::Dict& all_requests = doc.GetRoot().AsDict();
			if (all_requests.count("serialization_settings"s)) {
				const std::string file_name = all_requests.at("serialization_settings"s).AsDict().at("file"s).AsString();
				DeserializeTransportCatalogue(file_name);
			}
			handler_.BuildRouter();
			json::Array response;
			if (all_requests.count("stat_requests"s)) {
				const json::Array& requests = all_requests.at("stat_requests"s).AsArray();
				response.reserve(requests.size());
				for (const auto& base_request : requests) {
					const json::Dict& request_dict = base_request.AsDict();
					const std::string& type = request_dict.at("type"s).AsString();
					if (type == "Stop"s) {
						response.push_back(GetStopStat(request_dict));
					}
					else if (type == "Bus"s) {
						response.push_back(GetBusStat(request_dict));
					}
					else if (type == "Map"s) {
						response.push_back(GetMap(request_dict));
					}
					else if (type == "Route"s) {
						response.push_back(GetRoute(request_dict));
					}
					else {
						throw std::invalid_argument("Unknown stat_request type: "s + type);
					}
				}
			}
			json::Print(json::Document{ response }, output);
		}

		json::Dict JsonReader::GetStopStat(const json::Dict& stop_request) const {
			const auto buses = handler_.GetBusesByStop(stop_request.at("name"s).AsString());
			const int request_id = stop_request.at("id"s).AsInt();
			return std::visit(ResponseConverter{}, JsonResponse{ StopStat{request_id, buses} });
		}

		json::Dict JsonReader::GetBusStat(const json::Dict& bus_request) const {
			const auto bus_stat = handler_.GetBusStat(bus_request.at("name"s).AsString());
			const int request_id = bus_request.at("id"s).AsInt();
			return std::visit(ResponseConverter{}, JsonResponse{ BusStat{request_id, bus_stat} });
		}

		json::Dict JsonReader::GetRoute(const json::Dict& route_request) const {
			const auto route_stat = handler_.GetRoute(route_request.at("from"s).AsString(), route_request.at("to"s).AsString());
			const int request_id = route_request.at("id"s).AsInt();
			return std::visit(ResponseConverter{}, JsonResponse{ RouteStat{ request_id, route_stat } });
		}

		json::Dict JsonReader::GetMap(const json::Dict& map_request) const {
			const int request_id = map_request.at("id"s).AsInt();
			auto stops_to_bus_counts{ handler_.GetStopsToBuses() };
			auto buses{ handler_.GetBuses() };
			return std::visit(ResponseConverter{}, JsonResponse{ Map{ request_id, handler_.RenderMap(stops_to_bus_counts, buses) } });
		}

		renderer::RenderSettings JsonReader::GetRenderSettings(const json::Dict& settings_dict) const {
			const json::Array& bus_label_offset = settings_dict.at("bus_label_offset"s).AsArray();
			const json::Array& stop_label_offset = settings_dict.at("stop_label_offset"s).AsArray();
			const json::Array& color_palette_arr = settings_dict.at("color_palette"s).AsArray();
			std::vector<svg::Color> color_palette;
			color_palette.reserve(color_palette_arr.size());
			for (const auto& color : color_palette_arr) {
				color_palette.push_back(GetColor(color));
			}

			return {
				settings_dict.at("width"s).AsDouble(),
				settings_dict.at("height").AsDouble(),
				settings_dict.at("padding"s).AsDouble(),
				settings_dict.at("line_width"s).AsDouble(),
				settings_dict.at("stop_radius"s).AsDouble(),
				static_cast<std::uint32_t>(settings_dict.at("bus_label_font_size"s).AsInt()),
				{ bus_label_offset[0].AsDouble(), bus_label_offset[1].AsDouble() },
				static_cast<std::uint32_t>(settings_dict.at("stop_label_font_size"s).AsInt()),
				{ stop_label_offset[0].AsDouble(), stop_label_offset[1].AsDouble() },
				GetColor(settings_dict.at("underlayer_color"s)),
				settings_dict.at("underlayer_width"s).AsDouble(),
				std::move(color_palette)
			};
		}

		svg::Color JsonReader::GetColor(const json::Node& color_node) {
			if (color_node.IsString()) {
				return { color_node.AsString() };
			}
			else if (color_node.IsArray()) {
				const json::Array& color_arr = color_node.AsArray();
				switch (color_arr.size())
				{
				case 3:
					return svg::Rgb{
						static_cast<std::uint8_t>(color_arr[0].AsInt()),
						static_cast<std::uint8_t>(color_arr[1].AsInt()),
						static_cast<std::uint8_t>(color_arr[2].AsInt())
					};
				case 4:
					return svg::Rgba{
						static_cast<std::uint8_t>(color_arr[0].AsInt()),
						static_cast<std::uint8_t>(color_arr[1].AsInt()),
						static_cast<std::uint8_t>(color_arr[2].AsInt()),
						color_arr[3].AsDouble()
					};
				default:
					throw std::invalid_argument("Unknow color format with "s + std::to_string(color_arr.size()) + " parameters"s);
				}
			}
			else {
				throw std::invalid_argument("Failed to read color from json"s);
			}
		}

		transport_router::RoutingSettings JsonReader::GetRoutingSettings(const json::Dict& settings_dict) const {
			return {
				static_cast<std::uint32_t>(settings_dict.at("bus_wait_time"s).AsInt()),
				settings_dict.at("bus_velocity"s).AsDouble()
			};
		}

		void JsonReader::SerializeTransportCatalogue(const std::string& file_name) const {
			serialization::Serializer serializer(file_name, handler_);
			serializer.SerializeTransportCatalogue();
		}

		void JsonReader::DeserializeTransportCatalogue(const std::string& file_name) const {
			serialization::Serializer serializer(file_name, handler_);
			serializer.DeserializeTransportCatalogue();
		}

	}

}
