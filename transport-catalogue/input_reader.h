#pragma once
#include <string>
#include <vector>
#include "transport_catalogue.h"

namespace inputtxt {
	void ParseStopsAndAdd(std::vector<std::string>& stops, catalogue::TransportCatalogue& tc);
	void ParseDistancesAndAdd(std::unordered_map<std::string, std::vector<std::string>>& stopname_to_distinfo, catalogue::TransportCatalogue& tc);

	std::vector<const catalogue::Stop*> SplitStops(std::string route, char delimeter, catalogue::TransportCatalogue& tc);

	void ParseBusesAndAdd(std::vector<std::string>& buses, catalogue::TransportCatalogue& tc);

	void read_input(std::istream& input, catalogue::TransportCatalogue& tc);
}