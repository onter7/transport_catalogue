﻿#include "request_handler.h"

namespace transport_catalogue {

	namespace request_handler {

		RequestHandler::RequestHandler(TransportCatalogue& db, renderer::MapRenderer& renderer)
			: db_(db)
			, renderer_(renderer) {
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

		void RequestHandler::SetDistanceBetweenStops(const std::string_view from, const std::string_view to, const std::size_t distance) {
			db_.SetDistanceBetweenStops(from, to, distance);
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

		svg::Document RequestHandler::RenderMap(std::vector<std::pair<const domain::Stop*, std::size_t>>& stops_to_bus_counts, std::vector<const domain::Bus*>& buses) const {
			return renderer_.RenderMap(stops_to_bus_counts, buses);
		}

	}

}
