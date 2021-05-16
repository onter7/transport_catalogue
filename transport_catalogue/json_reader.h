#pragma once

#include <iostream>
#include <list>

#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"

namespace transport_catalogue {

	namespace json_reader {

		class JsonReader final {
		public:
			explicit JsonReader(std::istream& input = std::cin);			
			void ProcessStat(std::ostream& output = std::cout);
		private:
			const json::Document doc_;
			TransportCatalogue db_;

			void UpdateDatabase();
			void AddStops(const std::list<const json::Node*> stop_nodes);
			void AddBuses(const std::list<const json::Node*> bus_nodes);
			json::Dict GetStopStat(const request_handler::RequestHandler& handler, const json::Dict& stop_request);
			json::Dict GetBusStat(const request_handler::RequestHandler& handler, const json::Dict& bus_request);
		};

	}

}