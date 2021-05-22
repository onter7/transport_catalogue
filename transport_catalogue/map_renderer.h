#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <optional>
#include <vector>
#include <utility>

#include "domain.h"
#include "geo.h"
#include "svg.h"

namespace transport_catalogue {

	namespace renderer {

		inline const double EPSILON = 1e-6;
		bool IsZero(double value);

		class SphereProjector {
		public:
			template <typename PointInputIt>
			SphereProjector(PointInputIt points_begin, PointInputIt points_end, double max_width,
				double max_height, double padding)
				: padding_(padding) {
				if (points_begin == points_end) {
					return;
				}

				const auto [left_it, right_it]
					= std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
						return lhs.lng < rhs.lng;
					});
				min_lon_ = left_it->lng;
				const double max_lon = right_it->lng;

				const auto [bottom_it, top_it]
					= std::minmax_element(points_begin, points_end, [](auto lhs, auto rhs) {
						return lhs.lat < rhs.lat;
					});
				const double min_lat = bottom_it->lat;
				max_lat_ = top_it->lat;

				std::optional<double> width_zoom;
				if (!IsZero(max_lon - min_lon_)) {
					width_zoom = (max_width - 2 * padding) / (max_lon - min_lon_);
				}

				std::optional<double> height_zoom;
				if (!IsZero(max_lat_ - min_lat)) {
					height_zoom = (max_height - 2 * padding) / (max_lat_ - min_lat);
				}

				if (width_zoom && height_zoom) {
					zoom_coeff_ = std::min(*width_zoom, *height_zoom);
				}
				else if (width_zoom) {
					zoom_coeff_ = *width_zoom;
				}
				else if (height_zoom) {
					zoom_coeff_ = *height_zoom;
				}
			}

			svg::Point operator()(geo::Coordinates coords) const;

		private:
			double padding_;
			double min_lon_ = 0;
			double max_lat_ = 0;
			double zoom_coeff_ = 0;
		};

		struct RenderSettings {
			double width = 0.0;
			double height = 0.0;
			double padding = 0.0;
			double line_width = 0.0;
			double stop_radius = 0.0;
			std::uint32_t bus_label_font_size = 0;
			svg::Point bus_label_offset;
			std::uint32_t stop_label_font_size = 0;
			svg::Point stop_label_offset;
			svg::Color underlayer_color;
			double underlayer_width = 0.0;
			std::vector<svg::Color> color_palette;
		};

		class MapRenderer {
		public:
			explicit MapRenderer() = default;
			svg::Document RenderMap(std::vector<std::pair<const domain::Stop*, std::size_t>>& stops_to_bus_counts, std::vector<const domain::Bus*>& buses) const;
			void SetRenderSettings(const RenderSettings& settings);
		private:
			std::optional<RenderSettings> settings_;

			void RenderBusRoutes(svg::Document& doc, const SphereProjector& projector, const std::vector<const domain::Bus*>& buses) const;
			void RenderBusNames(svg::Document& doc, const SphereProjector& projector, const std::vector<const domain::Bus*>& buses) const;
			void RenderStopCircles(svg::Document& doc, const SphereProjector& projector, const std::vector<std::pair<const domain::Stop*, std::size_t>>& stops_to_bus_counts) const;
			void RenderStopNames(svg::Document& doc, const SphereProjector& projector, const std::vector<std::pair<const domain::Stop*, std::size_t>>& stops_to_bus_counts) const;
		};

	}

}