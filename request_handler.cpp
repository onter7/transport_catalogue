#include "request_handler.h"

#include <algorithm>

namespace transport_catalogue {

	namespace request_handler {

		RequestHandler::RequestHandler(TransportCatalogue& db, renderer::MapRenderer& renderer, transport_router::TransportRouter& router)
			: db_(db)
			, renderer_(renderer)
			, router_(router) {
		}

		void RequestHandler::AddStop(const std::string_view stop_name, const geo::Coordinates& coordinates) {
			db_.AddStop(stop_name, coordinates);
		}

		void RequestHandler::AddBus(const domain::BusType type, const std::string_view bus_name, const std::vector<std::string_view>& stop_names) {
			db_.AddBus(type, bus_name, stop_names);
		}

		std::vector<std::pair<const domain::Stop*, std::size_t>> RequestHandler::GetStopsToBuses() const {
			return db_.GetStopsToBusCounts();
		}

		std::vector<const domain::Bus*> RequestHandler::GetBuses() const {
			return db_.GetBuses();
		}

		std::vector<const domain::Stop*> RequestHandler::GetStops() const {
			return db_.GetStops();
		}

		const TransportCatalogue::StopPairsToDistances& RequestHandler::GetStopPairsToDistances() const {
			return db_.GetStopPairsToDistances();
		}

		void RequestHandler::SetDistanceBetweenStops(const std::string_view from, const std::string_view to, const std::size_t distance_m) {
			db_.SetDistanceBetweenStops(from, to, distance_m);
		}

		std::optional<domain::BusStat> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
			return db_.GetBusStat(bus_name);
		}

		const std::unordered_set<const domain::Bus*>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
			return db_.GetBusesByStop(stop_name);
		}

		void RequestHandler::SetRenderSettings(const renderer::RenderSettings& settings) {
			renderer_.SetRenderSettings(settings);
		}

		const renderer::RenderSettings& RequestHandler::GetRenderSettings() const {
			return renderer_.GetRenderSettings();
		}

		svg::Document RequestHandler::RenderMap(std::vector<std::pair<const domain::Stop*, std::size_t>>& stops_to_bus_counts, std::vector<const domain::Bus*>& buses) const {
			return renderer_.RenderMap(stops_to_bus_counts, buses);
		}

		void RequestHandler::SetRoutingSettings(const transport_router::RoutingSettings& settings) {
			router_.SetRoutingSettings(settings);
		}

		void RequestHandler::BuildRouter() {
			router_.BuildRouter();
		}

		std::optional<domain::RouteStat> RequestHandler::GetRoute(const std::string_view from, const std::string_view to) const {
			return router_.GetRoute(from, to);
		}

	}

}
