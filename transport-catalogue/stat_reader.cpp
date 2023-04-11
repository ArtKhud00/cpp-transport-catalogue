#include "stat_reader.h"
#include <sstream>
#include <iostream>

namespace outtxt {
	using namespace std::literals;
	std::ostream& operator <<(std::ostream& os, catalogue::BusInfo bus_info) {
		os << bus_info.stops_count << " stops on route, "s 
		   << bus_info.unique_stops << " unique stops, "s 
		   << bus_info.route_length << " route length, "s 
		   << bus_info.curvature << " curvature"s;
		return os;
	}

	std::ostream& operator <<(std::ostream& os, catalogue::StopInfo stop_info) {
		bool is_first = true;
		os << "buses "s;
		for (const auto& bus : stop_info.buses_) {
			if (is_first) {
				os << bus->busname_;
				is_first = false;
			}
			else {
				os << " "s << bus->busname_;
			}
		}
		return os;
	}

	
	void GetInformationAboutBus(std::string& request_content, catalogue::TransportCatalogue& tc) {
		const auto& bus_info = tc.GetBusInfo(request_content);
		if (bus_info.stops_count == 0) {
			std::cout << "Bus "s << request_content << ": not found"s << std::endl;
		}
		else {
			std::cout << "Bus "s << request_content << ": "s << bus_info << std::endl;
		}
	}
	
	void GetInformationAboutStop(std::string& request_content, catalogue::TransportCatalogue& tc) {
		const auto& stop_info = tc.GetStopInfo(request_content);
		if (stop_info.is_exist) {
			if (stop_info.buses_.size() > 0) {
				std::cout << "Stop "s << request_content << ": "s << stop_info << std::endl;
			}
			else {
				std::cout << "Stop "s << request_content << ": no buses"s << std::endl;
			}

		}
		else {
			std::cout << "Stop "s << request_content << ": not found"s << std::endl;
		}
	}

	void ProcessRequest(RequestsByTypeAndContent& requests, catalogue::TransportCatalogue& tc) {
		if (!requests.empty()) {
			for (auto& [request_type, request_content] : requests) {
				if (request_type == "Bus"s) {
					GetInformationAboutBus(request_content, tc);
				}
				if (request_type == "Stop"s) {
					GetInformationAboutStop(request_content, tc);
				}
			}
		}
	}


	void ReadRequest(std::istream& input, catalogue::TransportCatalogue& tc) {
		std::string count;
		std::getline(input, count);
		int num_of_reqs = 0;
		if (count.size() > 0) {
			num_of_reqs = std::stoi(count);
		}
		RequestsByTypeAndContent requests;
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
		ProcessRequest(requests, tc);
	}
}