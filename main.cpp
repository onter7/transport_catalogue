#include "json_reader.h"

using namespace std;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    transport_catalogue::TransportCatalogue db;
    transport_catalogue::renderer::MapRenderer renderer;
    transport_catalogue::transport_router::TransportRouter transport_router{ db };
    transport_catalogue::request_handler::RequestHandler handler{ db, renderer, transport_router };
    transport_catalogue::json_reader::JsonReader reader{ handler, db, renderer, transport_router };

    if (mode == "make_base"sv) {
        reader.MakeBase();
    }
    else if (mode == "process_requests"sv) {
        reader.ProcessRequests();
    }
    else {
        PrintUsage();
        return 1;
    }
}