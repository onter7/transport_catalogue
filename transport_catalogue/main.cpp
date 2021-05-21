﻿#include "json_reader.h"

using namespace std;

int main() {
	transport_catalogue::TransportCatalogue db;
	transport_catalogue::renderer::MapRenderer renderer;
	transport_catalogue::request_handler::RequestHandler handler{ db, renderer };
	transport_catalogue::json_reader::JsonReader reader{ handler };
	reader.ProcessRequests();
	return 0;
}