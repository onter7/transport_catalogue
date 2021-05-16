#include "transport_catalogue.h"
#include "json_reader.h"

using namespace std;
using namespace transport_catalogue;

int main() {

	json_reader::JsonReader reader;
	TransportCatalogue db;
	reader.UpdateDatabase(db);
	request_handler::RequestHandler rh{ db };
	reader.ProcessStat(rh);

	return 0;
}