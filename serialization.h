#pragma once

#include <cstddef>
#include <unordered_map>

#include <transport_catalogue.pb.h>
#include "request_handler.h"
#include "domain.h"

namespace serialization {
	void SerializeTransportCatalogue(const std::string& file_name, const transport_catalogue::request_handler::RequestHandler& handler_);
	void DeserializeTransportCatalogue(const std::string& file_name, transport_catalogue::request_handler::RequestHandler& handler_);
	transport_catalogue_serialize::Stop GetProtoStop(const transport_catalogue::domain::Stop* stop, const std::size_t id);
	transport_catalogue_serialize::Bus GetProtoBus(const transport_catalogue::domain::Bus* bus, const std::unordered_map<const transport_catalogue::domain::Stop*, std::size_t>& stop_to_id);

}