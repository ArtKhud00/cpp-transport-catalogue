#pragma once
#include <string>
#include <vector>

#include "geo.h"


namespace data{
	const size_t PRIME_NUMBER = 37;

	struct Stop {
		std::string stopname_;
		geo::Coordinates coordinates_;
		int id;
	};

	struct Bus {
		std::string busname_;
		std::vector<const Stop*> stops_;
		bool is_circle = false;
	};

	struct BusInfo {
		std::string busname_;
		int stops_count = 0;
		int unique_stops = 0;
		double route_length = 0;
		double curvature = 0;
	};

	struct StopInfo {
		std::string stopname_;
		bool is_exist = false;
		std::vector<Bus*> buses_;
	};
} // namespace data