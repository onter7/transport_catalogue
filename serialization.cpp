#include <fstream>
#include <ios>
#include <stdexcept>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "serialization.h"
#include "transport_catalogue.h"

namespace serialization {

	void ColorSetter::operator()(std::monostate) const {
		using namespace std::literals;
		ColorSetter::proto_color.set_color_str("none"s);
	}

	void ColorSetter::operator()(const std::string& color) const {
		ColorSetter::proto_color.set_color_str(color);
	}

	void ColorSetter::operator()(const svg::Rgb& color) const {
		transport_catalogue_serialize::Rgb proto_rgb;
		proto_rgb.set_red(color.red);
		proto_rgb.set_green(color.green);
		proto_rgb.set_blue(color.blue);
		*proto_color.mutable_rgb() = std::move(proto_rgb);
	}

	void ColorSetter::operator()(const svg::Rgba& color) const {
		transport_catalogue_serialize::Rgb proto_rgb;
		proto_rgb.set_red(color.red);
		proto_rgb.set_green(color.green);
		proto_rgb.set_blue(color.blue);
		transport_catalogue_serialize::Rgba proto_rgba;
		*proto_rgba.mutable_rgb() = std::move(proto_rgb);
		proto_rgba.set_opacity(color.opacity);
		*proto_color.mutable_rgba() = std::move(proto_rgba);
	}

	Serializer::Serializer(const std::string& file_name, transport_catalogue::request_handler::RequestHandler& handler)
		: file_name_(file_name)
		, handler_(handler) {
	}

	void Serializer::SerializeTransportCatalogue() const {
		std::ofstream out(file_name_, std::ios::binary);
		transport_catalogue_serialize::TransportCatalogue tc;
		const std::vector<const transport_catalogue::domain::Stop*> stops = handler_.GetStops();
		tc.mutable_stops()->Reserve(stops.size());
		std::unordered_map<const transport_catalogue::domain::Stop*, std::size_t> stop_to_id;
		for (std::size_t i = 0; i < stops.size(); ++i) {
			transport_catalogue_serialize::Stop* new_proto_stop = tc.add_stops();
			stop_to_id[stops[i]] = i;
			SetProtoStop(*new_proto_stop, *stops[i], i);
		}
		const std::vector<const transport_catalogue::domain::Bus*> buses = handler_.GetBuses();
		tc.mutable_buses()->Reserve(buses.size());
		for (const auto bus : buses) {
			transport_catalogue_serialize::Bus* new_proto_bus = tc.add_buses();
			SetProtoBus(*new_proto_bus, *bus, stop_to_id);
		}
		const transport_catalogue::TransportCatalogue::StopPairsToDistances& stop_pairs_to_distances = handler_.GetStopPairsToDistances();
		tc.mutable_distances()->Reserve(stop_pairs_to_distances.size());
		for (const auto& [stop_pair, distance] : stop_pairs_to_distances) {
			transport_catalogue_serialize::Distance* new_proto_distance = tc.add_distances();
			new_proto_distance->set_from_stop_id(stop_to_id[stop_pair.first]);
			new_proto_distance->set_to_stop_id(stop_to_id[stop_pair.second]);
			new_proto_distance->set_distance_m(distance);
		}
		*tc.mutable_settings() = GetProtoRenderSettings();
		tc.SerializeToOstream(&out);
	}

	void Serializer::DeserializeTransportCatalogue() {
		using namespace std::literals;
		std::ifstream in(file_name_, std::ios::binary);
		transport_catalogue_serialize::TransportCatalogue tc;
		if (!tc.ParseFromIstream(&in)) {
			throw std::runtime_error("Couldn't deserialize transport catalogue from file: "s + file_name_);
		}
		std::unordered_map<std::size_t, std::string_view> id_to_stop_name;
		for (const auto& stop : tc.stops()) {
			handler_.AddStop(stop.name(), { stop.coordinates().lat(), stop.coordinates().lng() });
			id_to_stop_name[stop.id()] = stop.name();
		}
		for (const auto& distance : tc.distances()) {
			handler_.SetDistanceBetweenStops(id_to_stop_name[distance.from_stop_id()], id_to_stop_name[distance.to_stop_id()], distance.distance_m());
		}
		for (const auto& bus : tc.buses()) {
			std::vector<std::string_view> stop_names;
			stop_names.reserve(bus.stop_ids().size());
			for (const auto& stop_id : bus.stop_ids()) {
				stop_names.push_back(id_to_stop_name[stop_id]);
			}
			handler_.AddBus(static_cast<transport_catalogue::domain::BusType>(bus.type()), bus.name(), stop_names);
		}
		handler_.SetRenderSettings(GetRenderSettings(tc.settings()));
	}

	transport_catalogue_serialize::RenderSettings Serializer::GetProtoRenderSettings() const {
		transport_catalogue_serialize::RenderSettings proto_settings;
		const auto& settings = handler_.GetRenderSettings();
		proto_settings.set_width(settings.width);
		proto_settings.set_height(settings.height);
		proto_settings.set_padding(settings.padding);
		proto_settings.set_line_width(settings.line_width);
		proto_settings.set_stop_radius(settings.stop_radius);
		proto_settings.set_bus_label_font_size(settings.bus_label_font_size);
		transport_catalogue_serialize::Point proto_bus_label_offset;
		proto_bus_label_offset.set_x(settings.bus_label_offset.x);
		proto_bus_label_offset.set_y(settings.bus_label_offset.y);
		*proto_settings.mutable_bus_label_offset() = std::move(proto_bus_label_offset);
		proto_settings.set_stop_label_font_size(settings.stop_label_font_size);
		transport_catalogue_serialize::Point proto_stop_label_offset;
		proto_stop_label_offset.set_x(settings.stop_label_offset.x);
		proto_stop_label_offset.set_y(settings.stop_label_offset.y);
		*proto_settings.mutable_stop_label_offset() = std::move(proto_stop_label_offset);
		transport_catalogue_serialize::Color& underlayer_color = *proto_settings.mutable_underlayer_color();
		std::visit(ColorSetter{ underlayer_color }, settings.underlayer_color);
		proto_settings.set_underlayer_width(settings.underlayer_width);
		proto_settings.mutable_color_palette()->Reserve(settings.color_palette.size());
		for (const auto& color : settings.color_palette) {
			transport_catalogue_serialize::Color* new_proto_color = proto_settings.add_color_palette();
			std::visit(ColorSetter{ *new_proto_color }, color);
		}
		return proto_settings;
	}

	transport_catalogue::renderer::RenderSettings Serializer::GetRenderSettings(const transport_catalogue_serialize::RenderSettings& proto_settings) const {
		transport_catalogue::renderer::RenderSettings settings;
		settings.width = proto_settings.width();
		settings.height = proto_settings.height();
		settings.padding = proto_settings.padding();
		settings.line_width = proto_settings.line_width();
		settings.stop_radius = proto_settings.stop_radius();
		settings.bus_label_font_size = proto_settings.bus_label_font_size();
		settings.bus_label_offset.x = proto_settings.bus_label_offset().x();
		settings.bus_label_offset.y = proto_settings.bus_label_offset().y();
		settings.stop_label_font_size = proto_settings.stop_label_font_size();
		settings.stop_label_offset.x = proto_settings.stop_label_offset().x();
		settings.stop_label_offset.y = proto_settings.stop_label_offset().y();
		svg::Color underlayer_color;
		SetColor(underlayer_color, proto_settings.underlayer_color());
		settings.underlayer_color = underlayer_color;
		settings.color_palette.reserve(proto_settings.color_palette().size());
		settings.color_palette.clear();
		for (const auto& proto_color : proto_settings.color_palette()) {
			svg::Color color;
			SetColor(color, proto_color);
			settings.color_palette.push_back(color);
		}
		return settings;
	}

	void Serializer::SetColor(svg::Color& color, const transport_catalogue_serialize::Color& proto_color) {
		if (proto_color.has_rgb()) {
			color = svg::Rgb{
				static_cast<uint8_t>(proto_color.rgb().red()),
				static_cast<uint8_t>(proto_color.rgb().green()),
				static_cast<uint8_t>(proto_color.rgb().blue())
			};
		}
		else if (proto_color.has_rgba()) {
			color = svg::Rgba{
				static_cast<uint8_t>(proto_color.rgba().rgb().red()),
				static_cast<uint8_t>(proto_color.rgba().rgb().green()),
				static_cast<uint8_t>(proto_color.rgba().rgb().blue()),
				proto_color.rgba().opacity()
			};
		}
		else if (!proto_color.color_str().empty()) {
			color = proto_color.color_str();
		}
		else {
			color = std::monostate{};
		}
	}

	void Serializer::SetProtoStop(transport_catalogue_serialize::Stop& proto_stop, const transport_catalogue::domain::Stop& stop, const std::size_t id) {
		transport_catalogue_serialize::Coordinates coordintates;
		coordintates.set_lat(stop.coordinates.lat);
		coordintates.set_lng(stop.coordinates.lng);
		*proto_stop.mutable_coordinates() = std::move(coordintates);
		proto_stop.set_id(id);
		proto_stop.set_name(stop.name);
	}

	void Serializer::SetProtoBus(transport_catalogue_serialize::Bus& proto_bus, const transport_catalogue::domain::Bus& bus, const std::unordered_map<const transport_catalogue::domain::Stop*, std::size_t>& stop_to_id) {
		proto_bus.set_type(static_cast<transport_catalogue_serialize::Bus_BusType>(bus.type));
		proto_bus.set_name(bus.name);
		proto_bus.mutable_stop_ids()->Reserve(bus.stops.size());
		for (const auto stop : bus.stops) {
			proto_bus.add_stop_ids(stop_to_id.at(stop));
		}
	}

}