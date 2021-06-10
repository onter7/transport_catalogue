#pragma once

#include "domain.h"
#include "graph.h"
#include "router.h"

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <unordered_map>
#include <variant>
#include <vector>

namespace transport_catalogue {

	namespace transport_router {

		struct RoutingSettings {
			std::uint32_t bus_wait_time = 6u;
			double bus_velocity = 40.0;
		};

		struct BusRoute {
			std::string_view bus_name;
			std::string_view from;
			std::string_view to;
			std::size_t distance = 0u;
			std::size_t span_count = 0u;
		};

		struct RouterParams {
			std::vector<std::string_view> stop_names;
			std::vector<BusRoute> bus_routes;
		};

		class TransportRouter {
		private:
			using Graph = graph::DirectedWeightedGraph<double>;

			static constexpr double TO_MINUTES = 0.06;

			enum class Type {
				Bus,
				Wait
			};

			struct VertexInfo {
				std::size_t start_waiting_id;
				std::size_t stop_waiting_id;
			};

			struct EdgeInfo {
				Type type;
				graph::Edge<double> edge;
				std::string_view from;
				std::string_view to;
				std::optional<std::string_view> bus_name;
				std::size_t span_count = 0u;
			};
		public:
			explicit TransportRouter() = default;
			void SetRoutingSettings(const RoutingSettings& settings);
			void BuildRouter();
			void InitGraph(const std::size_t vertex_count);
			void AddWaitEdge(const std::string_view stop_name);
			void AddBusEdge(const BusRoute& bus_route);
			std::optional<domain::RouteStat> GetRoute(const std::string_view from, const std::string_view to) const;
		private:
			RoutingSettings settings_;
			std::unordered_map<std::string_view, VertexInfo> stop_name_to_vertex_info_;
			std::vector<EdgeInfo> edge_infos_;
			std::optional<Graph> graph_;
			std::optional<graph::Router<double>> router_;
		};

	}

}