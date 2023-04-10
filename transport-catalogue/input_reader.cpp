#include "input_reader.h"
#include <sstream>
#include <algorithm>

namespace inputtxt {
	using namespace std::literals;

	void ParseDistancesAndAdd(std::unordered_map<std::string, std::vector<std::string>>& stopname_to_distinfo, catalogue::TransportCatalogue& tc) {
		if (!stopname_to_distinfo.empty()) {
			for (auto& [key, value] : stopname_to_distinfo) {
				std::vector<std::pair<std::string, int>> stops_to_distances;
				for (auto& info : value) {
					auto pos = info.find_first_not_of(' ');
					info = info.substr(pos);
					pos = info.find('m');
					int distance = std::stoi(info.substr(0, pos));
					pos = info.find("to"s);
					auto pos2 = info.find_first_not_of(' ', pos + 2);
					std::string stopname = info.substr(pos2);
					pos = stopname.find_last_not_of(' ');
					stopname = stopname.substr(0, pos + 1);
					stops_to_distances.push_back({ stopname,distance });
				}
				tc.AddStopsDistances(key, stops_to_distances);
			}
		}
	}

	void ParseStopsAndAdd(std::vector<std::string>& stops, catalogue::TransportCatalogue& tc) {
		std::unordered_map<std::string, std::vector<std::string>> stopname_to_distinfo;
		for (auto& stopp : stops) {
			auto pos = stopp.find_first_not_of(' ');
			stopp.substr(pos);
			pos = stopp.find(':');
			int num_of_commas = std::count(stopp.begin(), stopp.end(), ',');

			std::string stop_name = stopp.substr(0, pos);
			auto coord_begin = stopp.find_first_not_of(' ', pos + 1);
			auto comma_pos = stopp.find(',', coord_begin);
			double coord1 = std::stod(stopp.substr(coord_begin, comma_pos));
			coord_begin = stopp.find_first_not_of(' ', comma_pos + 1);
			std::string temp_coord;
			if (num_of_commas == 1) {
				comma_pos = stopp.find_last_not_of(' ', coord_begin);
				temp_coord = stopp.substr(coord_begin);
			}
			else {
				std::vector<std::string> distances;
				for (int i = 0; i < num_of_commas; i++) {
					comma_pos = stopp.find(',', coord_begin);
					if (i == 0) {
						temp_coord = stopp.substr(coord_begin, comma_pos - coord_begin);
					}
					else {
						if (comma_pos == stopp.npos) {
							auto pos1 = stopp.find_first_not_of(' ', coord_begin);
							distances.push_back(stopp.substr(pos1));
						}
						else {
							auto pos1 = stopp.find_first_not_of(' ', coord_begin);
							distances.push_back(stopp.substr(pos1, comma_pos - pos1));
						}
					}
					coord_begin = comma_pos + 1;
				}
				stopname_to_distinfo.insert({ stop_name,distances });
			}
			double coord2 = std::stod(temp_coord);
			catalogue::Stop stop = { stop_name, coord1, coord2 };

			tc.AddStop(std::move(stop));
		}
		ParseDistancesAndAdd(stopname_to_distinfo, tc);
	}


	std::vector<const catalogue::Stop*> SplitStops(std::string route, char delimeter, catalogue::TransportCatalogue& tc) {
		std::stringstream ss(route);
		std::string token;
		std::vector<const catalogue::Stop*> result;
		while (std::getline(ss, token, delimeter)) {
			auto pos = token.find_first_not_of(' ');
			auto pos1 = token.find_last_not_of(' ');
			token = token.substr(pos, pos1 - (pos - 1));
			result.push_back(tc.FindStop(token));
		}
		return result;
	}

	void ParseBusesAndAdd(std::vector<std::string>& buses, catalogue::TransportCatalogue& tc) {
		for (auto& buss : buses) {
			auto pos = buss.find_first_not_of(' ');
			buss = buss.substr(pos);
			pos = buss.find(':');
			std::string bus_name = buss.substr(0, pos);
			auto check = buss.find('>');
			bool isCircular = false;
			if (check != buss.npos) {
				isCircular = true;
			}
			buss = buss.substr(pos + 1);
			std::vector<const catalogue::Stop*> stops;
			char delimeter = isCircular ? '>' : '-';
			stops = SplitStops(buss, delimeter, tc);
			catalogue::Bus bus = { bus_name, stops, isCircular };
			tc.AddBus(std::move(bus));
		}
	}

	void read_input(std::istream& input, catalogue::TransportCatalogue& tc) {
		std::string count;
		std::getline(input, count);
		int num_of_reqs = 0;
		if (count.size() > 0) {
			num_of_reqs = std::stoi(count);
		}
		std::vector<std::string> stops;
		std::vector<std::string> buses;
		for (int i = 0; i < num_of_reqs; ++i) {
			std::string line;
			std::getline(input, line);
			auto pos = line.find_first_not_of(' ');
			line = line.substr(pos);
			pos = line.find(' ');
			std::string type = line.substr(0, pos);
			if (type == "Stop"s) {
				stops.push_back(line.substr(pos + 1));
			}
			if (type == "Bus"s) {
				buses.push_back(line.substr(pos + 1));
			}
		}
		ParseStopsAndAdd(stops, tc);
		ParseBusesAndAdd(buses, tc);
	}
}