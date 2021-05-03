#pragma once

#include <iostream>
#include <list>
#include <string_view>
#include <utility>

#include "transport_catalogue.h"

struct StopQuery {
	Stop stop;
	std::list<std::pair<std::string, std::size_t>> distances;

};

struct InputQueries {
	std::list<StopQuery> stop_queries;
	std::list<Bus> bus_queries;
};

InputQueries ReadInputQueries(std::istream& is = std::cin);

StopQuery ParseStopQuery(std::string_view stop_query);

Bus ParseBusQuery(std::string_view bus_query);

std::pair<std::string, std::size_t> ParseDistance(std::string_view stop_name_distance);

void UpdateDatabase(const InputQueries& input_queries, TransportCatalogue& transport_catalogue);
