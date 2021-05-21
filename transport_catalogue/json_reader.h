#pragma once

#include <iostream>
#include <list>

#include "json.h"
#include "request_handler.h"

namespace transport_catalogue {

	namespace json_reader {

		class JsonReader final {
		public:
			explicit JsonReader(request_handler::RequestHandler& handler);
			void ProcessRequests(std::istream& input = std::cin, std::ostream& output = std::cout);
		private:
			request_handler::RequestHandler& handler_;

			void UpdateDatabase(const json::Document& doc);
			void AddStops(const std::list<const json::Node*>& stop_nodes);
			void AddBuses(const std::list<const json::Node*>& bus_nodes);
			renderer::RenderSettings GetRenderSettings(const json::Dict& settings_dict) const;
			json::Dict GetMap(const json::Dict& stop_request) const;
			json::Dict GetStopStat(const json::Dict& stop_request) const;
			json::Dict GetBusStat(const json::Dict& bus_request) const;
			static svg::Color GetColor(const json::Node& color_node);
		};

	}

}