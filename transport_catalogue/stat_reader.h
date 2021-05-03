#pragma once

#include <iostream>

#include "transport_catalogue.h"

void ProcessStatQueries(const TransportCatalogue& transport_catalogue, std::istream& is = std::cin, std::ostream& os = std::cout);
