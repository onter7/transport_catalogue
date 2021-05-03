#include "transport_catalogue.h"
#include "input_reader.h"
#include "stat_reader.h"

using namespace std;

int main() {
	const transport_catalogue::input::Queries input_queries = transport_catalogue::input::ReadInput();
	transport_catalogue::TransportCatalogue catalogue;
	transport_catalogue::input::UpdateDatabase(input_queries, catalogue);
	transport_catalogue::stat::ProcessStat(catalogue);

	return 0;
}