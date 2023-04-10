#pragma once
#include <deque>
#include <string>
#include <unordered_map>
#include <set>
#include <string_view>
#include <vector>
#include "geo.h"

namespace catalogue {
const size_t PRIME_NUMBER = 37;

	struct Stop {
		std::string stopname_;
		double latitude;
		double longitude;
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
		bool isExist = false;
		std::vector<Bus*> buses_;
	};
	namespace detail {
		struct compareBus final {
			bool operator ()(const Bus* bus1, const Bus* bus2) const {
				return bus1->busname_ < bus2->busname_;
			}
		};

		struct dist_hasher {
			size_t operator () (const std::pair<const Stop*, const Stop*> stops) const {
				std::hash<const void*> hasher;
				return hasher(stops.first) + hasher(stops.second) * PRIME_NUMBER;
			}
		};
	}

	using BusSet = std::set<Bus*, detail::compareBus>;
	using stopdistancesinformation = std::unordered_map<std::pair<const Stop*, const Stop*>, double, detail::dist_hasher>;

	class TransportCatalogue {
	public:
		void AddStop(const Stop& stop);

		const Stop* FindStop(std::string_view stop_name);

		void AddBus(const Bus& bus);

		void AddStopsDistances(std::string_view stop_name, std::vector<std::pair<std::string, int>>& distances);

		const Bus* FindBus(std::string_view bus_name);

		const BusInfo GetBusInfo(std::string_view bus_name);

		const StopInfo GetStopInfo(std::string_view stop_name);

	private:
		std::unordered_map<std::string_view, Stop*> stopname_to_stop_;
		stopdistancesinformation stops_distances_;
		std::unordered_map<std::string_view, Bus*> busname_to_bus_;
		std::unordered_map<const Stop*, BusSet> stopname_to_buses_;
		std::deque<Stop> all_stops_;
		std::deque<Bus> all_buses_;

		double CalculateRouteLength(std::string_view bus_name);
		double CalculateRouteLengthGeo(std::string_view bus_name);
		void InsertBusToStop(Bus* bus);
		int DistanceBetweenStops(const Stop* stop1, const Stop* stop2);
		
	};
}