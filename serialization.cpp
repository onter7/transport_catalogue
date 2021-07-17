#include <fstream>
#include <ios>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

#include "serialization.h"
#include "transport_catalogue.h"

namespace serialization {

	void SerializeTransportCatalogue(const std::string& file_name, const transport_catalogue::request_handler::RequestHandler& handler_) {
		std::ofstream out(file_name, std::ios::binary);
		transport_catalogue_serialize::TransportCatalogue tc;
		const std::vector<const transport_catalogue::domain::Stop*> stops = handler_.GetStops();
		tc.mutable_stops()->Reserve(stops.size());
		std::unordered_map<const transport_catalogue::domain::Stop*, std::size_t> stop_to_id;
		for (std::size_t i = 0; i < stops.size(); ++i) {
			stop_to_id[stops[i]] = i;
			tc.mutable_stops()->Add(serialization::GetProtoStop(stops[i], i));
		}
		const std::vector<const transport_catalogue::domain::Bus*> buses = handler_.GetBuses();
		tc.mutable_buses()->Reserve(buses.size());
		for (const auto bus : buses) {
			tc.mutable_buses()->Add(serialization::GetProtoBus(bus, stop_to_id));
		}
		const transport_catalogue::TransportCatalogue::StopPairsToDistances& stop_pairs_to_distances = handler_.GetStopPairsToDistances();
		tc.mutable_distances()->Reserve(stop_pairs_to_distances.size());
		for (const auto& [stop_pair, distance] : stop_pairs_to_distances) {
			transport_catalogue_serialize::Distance proto_distance;
			proto_distance.set_from_stop_id(stop_to_id[stop_pair.first]);
			proto_distance.set_to_stop_id(stop_to_id[stop_pair.second]);
			proto_distance.set_distance_m(distance);
			tc.mutable_distances()->Add(std::move(proto_distance));
		}
		tc.SerializeToOstream(&out);
	}

	void DeserializeTransportCatalogue(const std::string& file_name, transport_catalogue::request_handler::RequestHandler& handler_) {
		using namespace std::literals;
		std::ifstream in(file_name, std::ios::binary);
		transport_catalogue_serialize::TransportCatalogue tc;
		if (!tc.ParseFromIstream(&in)) {
			throw std::runtime_error("Couldn't deserialize transport catalogue from file: "s + file_name);
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
	}

	transport_catalogue_serialize::Stop GetProtoStop(const transport_catalogue::domain::Stop* stop, const std::size_t id) {
		transport_catalogue_serialize::Coordinates coordintates;
		coordintates.set_lat(stop->coordinates.lat);
		coordintates.set_lng(stop->coordinates.lng);
		transport_catalogue_serialize::Stop proto_stop;
		*proto_stop.mutable_coordinates() = std::move(coordintates);
		proto_stop.set_id(id);
		proto_stop.set_name(stop->name);
		return proto_stop;
	}

	transport_catalogue_serialize::Bus GetProtoBus(const transport_catalogue::domain::Bus* bus, const std::unordered_map<const transport_catalogue::domain::Stop*, std::size_t>& stop_to_id) {
		transport_catalogue_serialize::Bus proto_bus;
		proto_bus.set_type(static_cast<transport_catalogue_serialize::Bus_BusType>(bus->type));
		proto_bus.set_name(bus->name);
		proto_bus.mutable_stop_ids()->Reserve(bus->stops.size());
		for (const auto stop : bus->stops) {
			proto_bus.add_stop_ids(stop_to_id.at(stop));
		}
		return proto_bus;
	}

}