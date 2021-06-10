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

		void RequestHandler::SetRoutingSettings(const transport_router::RoutingSettings& settings) {
			router_.SetRoutingSettings(settings);
		}

		void RequestHandler::BuildRouter() {
			const auto stops{ db_.GetStops() };
			router_.InitGraph(stops.size() * 2);
			for (const auto* stop : stops) {
				const std::string name = stop->name;
				router_.AddWaitEdge(stop->name);
			}
			const auto buses{ db_.GetBuses() };
			for (const auto* bus : buses) {
				for (std::size_t from_index = 0u; from_index + 1u < bus->stops.size(); ++from_index) {
					std::size_t distance = 0u;
					std::size_t distance_reverse = 0u;
					for (std::size_t to_index = from_index + 1u; to_index < bus->stops.size(); ++to_index) {
						distance += db_.GetDistanceBetweenStops(bus->stops[to_index - 1u], bus->stops[to_index]);
						distance_reverse += db_.GetDistanceBetweenStops(bus->stops[bus->stops.size() - to_index], bus->stops[bus->stops.size() - to_index - 1u]);
						router_.AddBusEdge(
							transport_router::BusRoute{
								bus->name,
								bus->stops[from_index]->name,
								bus->stops[to_index]->name,
								distance,
								to_index - from_index
							}
						);
						if (bus->type == domain::BusType::DIRECT) {
							router_.AddBusEdge(
								transport_router::BusRoute{
									bus->name,
									bus->stops[bus->stops.size() - from_index - 1u]->name,
									bus->stops[bus->stops.size() - to_index - 1u]->name,
									distance_reverse,
									to_index - from_index
								}
							);
						}
					}
				}
			}
			router_.BuildRouter();
		}

		std::optional<domain::RouteStat> RequestHandler::GetRoute(const std::string_view from, const std::string_view to) const {
			return router_.GetRoute(from, to);
		}

	}

}
