#include "request_handler.h"

namespace transport_catalogue {

	namespace request_handler {

		RequestHandler::RequestHandler(const TransportCatalogue& db)
			: db_(db) {
		}

		std::optional<domain::BusStat> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
			return db_.GetBusStat(bus_name);
		}

		const std::unordered_set<const domain::Bus*>* RequestHandler::GetBusesByStop(const std::string_view& stop_name) const {
			return db_.GetBusesByStop(stop_name);
		}

	}

}
