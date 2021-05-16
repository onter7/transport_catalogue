#pragma once

#include <optional>
#include <string_view>
#include <unordered_set>

#include "domain.h"
#include "transport_catalogue.h"

namespace transport_catalogue {

	namespace request_handler {

		class RequestHandler {
		public:
			RequestHandler(const TransportCatalogue& db/*, const renderer::MapRenderer& renderer*/);

			std::optional<domain::BusStat> GetBusStat(const std::string_view& bus_name) const;

			const std::unordered_set<const domain::Bus*>* GetBusesByStop(const std::string_view& stop_name) const;

			//svg::Document RenderMap() const;

		private:
			const TransportCatalogue& db_;
			//const renderer::MapRenderer& renderer_;
		};

	}

}


