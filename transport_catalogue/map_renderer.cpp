#include <cstddef>

#include "map_renderer.h"

using namespace std::literals;

namespace transport_catalogue {

	namespace renderer {

		bool IsZero(double value) {
			return std::abs(value) < EPSILON;
		}

		svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
			return { (coords.lng - min_lon_) * zoom_coeff_ + padding_,
					(max_lat_ - coords.lat) * zoom_coeff_ + padding_ };
		}

		void MapRenderer::SetRenderSettings(const RenderSettings& settings) {
			settings_ = settings;
		}

		svg::Document MapRenderer::RenderMap(std::vector<std::pair<const domain::Stop*, std::size_t>>& stops_to_bus_counts, std::vector<const domain::Bus*>& buses) const {
			std::sort(stops_to_bus_counts.begin(), stops_to_bus_counts.end(), [](const auto& lhs, const auto& rhs) {
				return std::lexicographical_compare(lhs.first->name.begin(), lhs.first->name.end(), rhs.first->name.begin(), rhs.first->name.end());
			});
			std::sort(buses.begin(), buses.end(), [](const domain::Bus* lhs, const domain::Bus* rhs) {
				return std::lexicographical_compare(lhs->name.begin(), lhs->name.end(), rhs->name.begin(), rhs->name.end());
			});
			std::vector<geo::Coordinates> stop_coordinates;
			stop_coordinates.reserve(stops_to_bus_counts.size());
			for (const auto [stop, bus_count] : stops_to_bus_counts) {
				if (bus_count != 0) {
					stop_coordinates.push_back(stop->coordinates);
				}
			}
			const SphereProjector projector(
				stop_coordinates.begin(),
				stop_coordinates.end(),
				(*settings_).width,
				(*settings_).height,
				(*settings_).padding);
			svg::Document result;
			RenderBusRoutes(result, projector, buses);
			RenderBusNames(result, projector, buses);
			RenderStopCircles(result, projector, stops_to_bus_counts);
			RenderStopNames(result, projector, stops_to_bus_counts);
			return result;
		}

		void MapRenderer::RenderBusRoutes(svg::Document& doc, const SphereProjector& projector, const std::vector<const domain::Bus*>& buses) const {
			std::size_t current_color_index = 0;
			for (const auto bus : buses) {
				if (bus->stops.empty()) {
					continue;
				}
				svg::Polyline line;
				line
					.SetStrokeColor((*settings_).color_palette[current_color_index++])
					.SetFillColor(svg::NoneColor)
					.SetStrokeWidth((*settings_).line_width)
					.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
					.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
				for (const auto& stop : bus->stops) {
					line.AddPoint(projector(stop->coordinates));
				}
				if (bus->type == domain::BusType::DIRECT && bus->stops.size() > 1) {
					for (auto it = std::next(bus->stops.rbegin()); it != bus->stops.rend(); ++it) {
						line.AddPoint(projector((*it)->coordinates));
					}
				}
				doc.Add(std::move(line));
				if (current_color_index == (*settings_).color_palette.size()) {
					current_color_index = 0;
				}
			}
		}

		void MapRenderer::RenderBusNames(svg::Document& doc, const SphereProjector& projector, const std::vector<const domain::Bus*>& buses) const {
			std::size_t current_color_index = 0;
			for (const auto bus : buses) {
				if (bus->stops.empty()) {
					continue;
				}
				svg::Text text;
				text
					.SetPosition(projector(bus->stops.front()->coordinates))
					.SetOffset((*settings_).bus_label_offset)
					.SetFontSize((*settings_).bus_label_font_size)
					.SetFontFamily("Verdana"s)
					.SetFontWeight("bold"s)
					.SetData(bus->name);
				svg::Text substrate = text;
				substrate
					.SetFillColor((*settings_).underlayer_color)
					.SetStrokeColor((*settings_).underlayer_color)
					.SetStrokeWidth((*settings_).underlayer_width)
					.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
					.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
				text
					.SetFillColor((*settings_).color_palette[current_color_index++]);
				if (current_color_index == (*settings_).color_palette.size()) {
					current_color_index = 0;
				}

				svg::Text last_stop_text = text;
				svg::Text last_stop_substrate = substrate;

				doc.Add(std::move(substrate));
				doc.Add(std::move(text));
				if (bus->type == domain::BusType::DIRECT && bus->stops.front()->name != bus->stops.back()->name) {
					svg::Point p{ projector(bus->stops.back()->coordinates) };
					last_stop_text
						.SetPosition(p);
					last_stop_substrate
						.SetPosition(p);
					doc.Add(std::move(last_stop_substrate));
					doc.Add(std::move(last_stop_text));
				}
			}
		}

		void MapRenderer::RenderStopCircles(svg::Document& doc, const SphereProjector& projector, const std::vector<std::pair<const domain::Stop*, std::size_t>>& stops_to_bus_counts) const {
			for (const auto [stop, bus_count] : stops_to_bus_counts) {
				if (bus_count == 0) {
					continue;
				}
				svg::Circle circle;
				circle
					.SetCenter(projector(stop->coordinates))
					.SetRadius((*settings_).stop_radius)
					.SetFillColor("white"s);
				doc.Add(std::move(circle));
			}
		}

		void MapRenderer::RenderStopNames(svg::Document& doc, const SphereProjector& projector, const std::vector<std::pair<const domain::Stop*, std::size_t>>& stops_to_bus_counts) const {
			for (const auto [stop, bus_count] : stops_to_bus_counts) {
				if (bus_count == 0) {
					continue;
				}
				svg::Text text;
				text
					.SetPosition(projector(stop->coordinates))
					.SetOffset((*settings_).stop_label_offset)
					.SetFontSize((*settings_).stop_label_font_size)
					.SetFontFamily("Verdana"s)
					.SetData(stop->name);
				svg::Text substrate = text;
				substrate
					.SetFillColor((*settings_).underlayer_color)
					.SetStrokeColor((*settings_).underlayer_color)
					.SetStrokeWidth((*settings_).underlayer_width)
					.SetStrokeLineCap(svg::StrokeLineCap::ROUND)
					.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
				text
					.SetFillColor("black"s);
				doc.Add(std::move(substrate));
				doc.Add(std::move(text));
			}
		}

	}

}