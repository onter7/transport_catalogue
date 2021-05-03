#pragma once

#include <iostream>
#include <list>
#include <string>
#include <vector>

#include "transport_catalogue.h"

namespace transport_catalogue {

	namespace stat {

		void ProcessStat(const TransportCatalogue& catalogue, std::istream& is = std::cin, std::ostream& os = std::cout);

		void PrintBusInfo(std::string_view bus_number, const TransportCatalogue& catalogue, std::ostream& os = std::cout);

		void PrintStopInfo(std::string_view stop_name, const TransportCatalogue& catalogue, std::ostream& os = std::cout);

	}

}
