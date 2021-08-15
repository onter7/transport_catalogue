#include "request_handler.h"

#include <algorithm>

namespace transport_catalogue {

	namespace request_handler {

		RequestHandler::RequestHandler(TransportCatalogue& db, renderer::MapRenderer& renderer, transport_router::TransportRouter& router)
			: db_(db)
			, renderer_(renderer)
			, router_(router) {
		}

		std::optional<domain::BusStat> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
			return db_.GetBusStat(bus_name);
		}

		const std::unordered_set<const domain::Bus*>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
			return db_.GetBusesByStop(stop_name);
		}

		svg::Document RequestHandler::RenderMap(std::vector<std::pair<const domain::Stop*, std::size_t>>& stops_to_bus_counts, std::vector<const domain::Bus*>& buses) const {
			return renderer_.RenderMap(stops_to_bus_counts, buses);
		}

		std::optional<domain::RouteStat> RequestHandler::GetRoute(const std::string_view from, const std::string_view to) const {
			return router_.GetRoute(from, to);
		}

	}

}
