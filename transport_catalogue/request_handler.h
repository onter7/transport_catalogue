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
			void AddStop(const std::string_view stop_name, const geo::Coordinates& coordinates);
			void AddBus(const domain::BusType type, const std::string_view bus_name, const std::vector<std::string_view>& stop_names);
			std::vector<std::pair<const domain::Stop*, std::size_t>> GetStopsToBuses() const;
			std::vector<const domain::Bus*> GetBuses() const;
			void SetDistanceBetweenStops(const std::string_view from, const std::string_view to, const std::size_t distance);
			std::optional<domain::BusStat> GetBusStat(const std::string_view& bus_name) const;
			const std::unordered_set<const domain::Bus*>* GetBusesByStop(const std::string_view& stop_name) const;
			void SetRenderSettings(const renderer::RenderSettings& settings);
			svg::Document RenderMap(std::vector<std::pair<const domain::Stop*, std::size_t>>& stops, std::vector<const domain::Bus*>& buses) const;
			void SetRoutingSettings(const transport_router::RoutingSettings& settings);
			void BuildRouter();
			std::optional<domain::RouteStat> GetRoute(const std::string_view from, const std::string_view to) const;

		private:
			TransportCatalogue& db_;
			renderer::MapRenderer& renderer_;
			transport_router::TransportRouter& router_;
		};

	}

}
