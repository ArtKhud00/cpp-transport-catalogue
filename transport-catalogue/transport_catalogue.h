#pragma once
#include <deque>
#include <string>
#include <unordered_map>
#include <set>
#include <string_view>
#include <vector>

#include "geo.h"
#include "domain.h"

namespace catalogue {

	namespace detail {
		struct compareBus final {
			bool operator ()(const data::Bus* bus1, const data::Bus* bus2) const {
				return bus1->busname_ < bus2->busname_;
			}
		};

		struct dist_hasher {
			size_t operator () (const std::pair<const data::Stop*, const data::Stop*> stops) const {
				std::hash<const void*> hasher;
				return hasher(stops.first) + hasher(stops.second) * data::PRIME_NUMBER;
			}
		};
	}

	using BusSet = std::set<data::Bus*, detail::compareBus>;
	using DistancesBetweenStops = std::unordered_map<std::pair<const data::Stop*, const data::Stop*>, double, detail::dist_hasher>;

	class TransportCatalogue {
	public:
		void AddStop(const data::Stop& stop);

		const data::Stop* FindStop(std::string_view stop_name);

		void AddBus(const data::Bus& bus);

		void SetStopsDistances(const data::Stop* from_stop, const data::Stop* to_stop, int distance);
		int GetStopsDistance(const data::Stop* from_stop, const data::Stop* to_stop);

		const data::Bus* FindBus(std::string_view bus_name);

		const data::BusInfo GetBusInfo(std::string_view bus_name);

		const data::StopInfo GetStopInfo(std::string_view stop_name);
	
		std::unordered_map<std::string_view, data::Bus*> GetBusnameToBuses() const;
		std::unordered_map<std::string_view, data::Stop*> GetStopnameToStops() const;

		const std::vector<const data::Stop*> GetStopsAsPtrs();
		const std::vector<const data::Bus*> GetBusesAsPtrs();
	private:
		std::unordered_map<std::string_view, data::Stop*> stopname_to_stop_;
		DistancesBetweenStops stops_distances_;
		std::unordered_map<std::string_view, data::Bus*> busname_to_bus_;
		std::unordered_map<const data::Stop*, BusSet> stopname_to_buses_;
		std::deque<data::Stop> all_stops_;
		std::deque<data::Bus> all_buses_;

		double CalculateRouteLength(std::string_view bus_name);

		double CalculateRouteLengthGeo(std::string_view bus_name);

		void InsertBusToStop(data::Bus* bus);

	};
}