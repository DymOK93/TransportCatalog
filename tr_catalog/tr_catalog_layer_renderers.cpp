#include "transport_catalog.h"
using namespace std;

#ifdef RENDER
const unordered_map<string_view, function<void(const TransportCatalog*, svg::Document*)>> 
TransportCatalog::layer_renderers {
	{"bus_lines", &TransportCatalog::print_bus_routes},
	{"bus_labels", &TransportCatalog::print_bus_labels},
	{"stop_points", &TransportCatalog::print_stops},
	{"stop_labels", &TransportCatalog::print_stop_labels}
};
#endif