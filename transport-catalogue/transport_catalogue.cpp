#include "transport_catalogue.h"
#include <unordered_set>
#include <stdexcept>

namespace catalogue {
	void TransportCatalogue::AddStop(const Stop& stop) {
		all_stops_.push_back(stop);
		stopname_to_stop_[all_stops_.back().stopname_] = &all_stops_.back();
	}

	const Stop* TransportCatalogue::FindStop(std::string_view stop_name) {
		if (stopname_to_stop_.count(stop_name) > 0) {
			return stopname_to_stop_.at(stop_name);
		}
		return nullptr;
	}

	void TransportCatalogue::AddBus(const Bus& bus) {
		all_buses_.push_back(bus);
		busname_to_bus_[all_buses_.back().busname_] = &all_buses_.back();
		InsertBusToStop(&all_buses_.back());
	}

	const Bus* TransportCatalogue::FindBus(std::string_view bus_name) {
		if (busname_to_bus_.count(bus_name) > 0) {
			return busname_to_bus_.at(bus_name);
		}
		return nullptr;
	}

	const BusInfo TransportCatalogue::GetBusInfo(std::string_view bus_name) {
		const Bus* bus = FindBus(bus_name);
		BusInfo res;
		if (bus != nullptr) {
			res.busname_ = bus->busname_;
			auto& stops = bus->stops_;
			res.stops_count = bus->is_circle ? static_cast<int>(stops.size())
				: static_cast<int>(stops.size() * 2 - 1);
			std::unordered_set<const Stop*> unique_stops = { stops.begin(),stops.end() };
			res.unique_stops = unique_stops.size();
			res.route_length = CalculateRouteLength(bus_name);
			double route_geo_length = CalculateRouteLengthGeo(bus_name);
			res.curvature = res.route_length / route_geo_length;
		}
		return res;
	}

	const StopInfo TransportCatalogue::GetStopInfo(std::string_view stop_name) {
		const Stop* stop = FindStop(stop_name);
		StopInfo res;
		res.isExist = false;
		if (stop != nullptr) {
			res.isExist = true;
			res.stopname_ = stop->stopname_;
			if (stopname_to_buses_.count(stop) > 0) {
				const auto& buses = stopname_to_buses_.at(stop);
				res.buses_ = { buses.begin(),buses.end() };
			}
		}
		return res;
	}

	int TransportCatalogue::DistanceBetweenStops(const Stop* stop1, const Stop* stop2) {
		auto two_stops1 = std::make_pair(stop1, stop2);
		auto two_stops2 = std::make_pair(stop2, stop1);
		int result = 0;
		if (stops_distances_.count(two_stops1) > 0) {
			result = stops_distances_.at(two_stops1);
			return result;
		}
		if (stops_distances_.count(two_stops2) > 0) {
			result = stops_distances_.at(two_stops2);
			return result;
		}
		return result;
	}

	double TransportCatalogue::CalculateRouteLengthGeo(std::string_view bus_name) {
		if (busname_to_bus_.count(bus_name) > 0) {
			const Bus* bus = FindBus(bus_name);
			bool isCircular = bus->is_circle;
			double distance = 0;
			auto& stops = bus->stops_;
			for (int i = 0; i < stops.size() - 1; ++i) {
				auto stop1 = stops[i];
				auto stop2 = stops[i + 1];
				distance += isCircular ? geo::ComputeDistance({ stop1->latitude, stop1->longitude }, { stop2->latitude, stop2->longitude })
					: (geo::ComputeDistance({ stop1->latitude, stop1->longitude }, { stop2->latitude, stop2->longitude }) + geo::ComputeDistance({ stop2->latitude, stop2->longitude }, { stop1->latitude, stop1->longitude }));
			}
			return distance;
		}
		return 0;
	}

	double TransportCatalogue::CalculateRouteLength(std::string_view bus_name) {
		if (busname_to_bus_.count(bus_name) > 0) {
			const Bus* bus = FindBus(bus_name);
			bool isCircular = bus->is_circle;
			int distance = 0;
			auto& stops = bus->stops_;
			for (int i = 0; i < stops.size() - 1; ++i) {
				auto stop1 = stops[i];
				auto stop2 = stops[i + 1];

				distance += isCircular ? DistanceBetweenStops(stop1, stop2) :
					(DistanceBetweenStops(stop1, stop2) + DistanceBetweenStops(stop2, stop1));
			}

			return distance;
		}
		return 0;
	}

	void TransportCatalogue::InsertBusToStop(Bus* bus) {
		for (const auto& stopp : bus->stops_) {
			stopname_to_buses_[stopp].insert(bus);
		}
	}

	void TransportCatalogue::AddStopsDistances(std::string_view stop_name, std::vector<std::pair<std::string, int>>& distances) {
		if (stopname_to_stop_.count(stop_name) == 0) {
			throw std::invalid_argument("Invalid stop in distances");
		}
		else {
			const Stop* from_stop = FindStop(stop_name);
			for (const auto& [stop, distance] : distances) {
				if (stopname_to_stop_.count(stop) == 0) {
					throw std::invalid_argument("Invalid stop in distances");
				}
				const Stop* to_stop = FindStop(stop);
				stops_distances_[{from_stop, to_stop}] = distance;
			}
		}
	}
}