#include "input_reader.h"
#include "stat_reader.h"
#include "transport_catalogue.h"

using namespace std;

int main() {	
	TransportCatalogue transport_catalogue;
	const InputQueries iq = ReadInputQueries();
	UpdateDatabase(iq, transport_catalogue);
	ProcessStatQueries(transport_catalogue);
	return 0;
}