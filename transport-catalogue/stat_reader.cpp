#include "stat_reader.h"
#include <sstream>
#include <iostream>

namespace outtxt {
	using namespace std::literals;
	std::ostream& operator <<(std::ostream& os, catalogue::BusInfo businfo) {
		os << businfo.stops_count << " stops on route, "s << businfo.unique_stops << " unique stops, "s <<
			businfo.route_length << " route length, "s << businfo.curvature << " curvature"s;
		return os;
	}

	std::ostream& operator <<(std::ostream& os, catalogue::StopInfo stopinfo) {
		os << "buses "s;
		bool isFirst = true;
		for (const auto& bus : stopinfo.buses_) {
			if (isFirst) {
				os << bus->busname_;
				isFirst = false;
			}
			else {
				os << " "s << bus->busname_;
			}
		}
		return os;
	}

	void readRequest(std::istream& input, catalogue::TransportCatalogue& tc) {
		std::string count;
		std::getline(input, count);
		int num_of_reqs = 0;
		if (count.size() > 0) {
			num_of_reqs = std::stoi(count);
		}
		std::vector<std::pair<std::string, std::string>> requests;
		for (int i = 0; i < num_of_reqs; ++i) {
			std::string line;
			std::getline(input, line);
			auto pos = line.find_first_not_of(' ');
			line = line.substr(pos);
			pos = line.find(' ');
			std::string first_part = line.substr(0, pos);
			auto pos1 = line.find_first_not_of(' ', pos + 1);
			std::string last_part = line.substr(pos1);
			auto pos2 = last_part.find_last_not_of(' ');
			last_part = last_part.substr(0, pos2 + 1);
			requests.push_back(std::move(std::make_pair(first_part, last_part)));

		}
		for (auto& [Key, Request] : requests) {
			if (Key == "Bus"s) {
				const auto& businfo = tc.GetBusInfo(Request);
				if (businfo.stops_count == 0) {
					std::cout << "Bus "s << Request << ": not found"s << std::endl;
				}
				else {
					std::cout << "Bus "s << Request << ": "s << businfo << std::endl;
				}
			}
			if (Key == "Stop"s) {
				const auto& stopinfo = tc.GetStopInfo(Request);
				if (stopinfo.isExist) {
					if (stopinfo.buses_.size() > 0) {
						std::cout << "Stop "s << Request << ": "s << stopinfo << std::endl;
					}
					else {
						std::cout << "Stop "s << Request << ": no buses"s << std::endl;
					}

				}
				else {
					std::cout << "Stop "s << Request << ": not found"s << std::endl;
				}
			}
		}
	}
}