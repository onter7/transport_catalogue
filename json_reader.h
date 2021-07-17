#pragma once

#include <iostream>
#include <list>
#include <optional>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "json.h"
#include "request_handler.h"
#include "transport_router.h"
#include <transport_catalogue.pb.h>

namespace transport_catalogue {

	namespace json_reader {

		struct Response;
		struct NotFound;
		struct Map;
		struct StopStat;
		struct BusStat;
		struct RouteStat;

		struct RouteItemConverter {
			json::Dict operator()(const domain::BusRouteItem& bus) const;
			json::Dict operator()(const domain::WaitRouteItem& wait) const;
		};

		using JsonResponse = std::variant<NotFound, Map, StopStat, BusStat, RouteStat>;

		struct Response {
			int request_id = 0;
		};

		struct NotFound : public Response {};

		struct Map : public Response {
			svg::Document doc;
		};

		struct StopStat : public Response {
			const std::unordered_set<const domain::Bus*>* buses;
		};

		struct BusStat : public Response {
			std::optional<domain::BusStat> bus_stat;
		};

		struct RouteStat : public Response {
			std::optional<domain::RouteStat> route_stat;
		};

		struct ResponseConverter {
			json::Dict operator()(const NotFound& response) const;
			json::Dict operator()(const Map& response) const;
			json::Dict operator()(const StopStat& response) const;
			json::Dict operator()(const BusStat& response) const;
			json::Dict operator()(const RouteStat& response) const;
		};

		class JsonReader final {
		public:
			explicit JsonReader(request_handler::RequestHandler& handler);
			void ProcessRequests(std::istream& input = std::cin, std::ostream& output = std::cout);
			void MakeBase(std::istream& input = std::cin);
		private:
			request_handler::RequestHandler& handler_;

			void UpdateDatabase(const json::Document& doc);
			void AddStops(const std::list<const json::Node*>& stop_nodes);
			void AddBuses(const std::list<const json::Node*>& bus_nodes);
			renderer::RenderSettings GetRenderSettings(const json::Dict& settings_dict) const;
			json::Dict GetMap(const json::Dict& stop_request) const;
			json::Dict GetStopStat(const json::Dict& stop_request) const;
			json::Dict GetBusStat(const json::Dict& bus_request) const;
			json::Dict GetRoute(const json::Dict& route_request) const;
			static svg::Color GetColor(const json::Node& color_node);
			transport_router::RoutingSettings GetRoutingSettings(const json::Dict& settings_dict) const;
			void SerializeTransportCatalogue(const std::string& file_name) const;
			void DeserializeTransportCatalogue(const std::string& file_name) const;
		};

	}

}