#pragma once
#include <string>
#include <vector>
#include "transport_catalogue.h"

namespace inputtxt {
	
	void SetParsedDistancesToStop(std::string stop_name,
		                          std::vector<std::pair<std::string, int>>& stops_to_distances,
		                          catalogue::TransportCatalogue& tc);
	void ParseStopsAndAdd(std::vector<std::string>& stops, catalogue::TransportCatalogue& tc);
	void ParseDistancesAndSet(std::unordered_map<std::string, std::vector<std::string>>& stopname_to_distinfo, 
		                      catalogue::TransportCatalogue& tc);
	void ParseStopsAndAdd(std::vector<std::string>& stops, catalogue::TransportCatalogue& tc);
	std::vector<const catalogue::Stop*> SplitStops(std::string route, char delimeter, catalogue::TransportCatalogue& tc);
	void ParseBusesAndAdd(std::vector<std::string>& buses, catalogue::TransportCatalogue& tc);
	void ReadInput(std::istream& input, catalogue::TransportCatalogue& tc);
}