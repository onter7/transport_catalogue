#pragma once

#include <cstddef>
#include <optional>
#include <string_view>
#include <unordered_set>
#include <utility>
#include <vector>

#include "domain.h"
#include "map_renderer.h"
#include "transport_catalogue.h"
#include "transport_router.h"

namespace transport_catalogue {

	namespace request_handler {

		class RequestHandler {
		public:
			RequestHandler(TransportCatalogue& db, renderer::MapRenderer& renderer, transport_router::TransportRouter& router);
			std::optional<domain::BusStat> GetBusStat(const std::string_view& bus_name) const;
			const std::unordered_set<const domain::Bus*>* GetBusesByStop(const std::string_view& stop_name) const;
			svg::Document RenderMap(std::vector<std::pair<const domain::Stop*, std::size_t>>& stops, std::vector<const domain::Bus*>& buses) const;
			std::optional<domain::RouteStat> GetRoute(const std::string_view from, const std::string_view to) const;
		private:
			TransportCatalogue& db_;
			renderer::MapRenderer& renderer_;
			transport_router::TransportRouter& router_;
		};

	}

}
