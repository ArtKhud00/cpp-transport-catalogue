#include "transport_catalogue.h"
#include <unordered_set>
#include <stdexcept>

namespace catalogue {
	void TransportCatalogue::AddStop(const data::Stop& stop) {
		all_stops_.push_back(stop);
		stopname_to_stop_[all_stops_.back().stopname_] = &all_stops_.back();
	}

	const data::Stop* TransportCatalogue::FindStop(std::string_view stop_name) {
		if (stopname_to_stop_.count(stop_name) > 0) {
			return stopname_to_stop_.at(stop_name);
		}
		return nullptr;
	}

	void TransportCatalogue::AddBus(const data::Bus& bus) {
		all_buses_.push_back(bus);
		busname_to_bus_[all_buses_.back().busname_] = &all_buses_.back();
		InsertBusToStop(&all_buses_.back());
	}

	const data::Bus* TransportCatalogue::FindBus(std::string_view bus_name) {
		if (busname_to_bus_.count(bus_name) > 0) {
			return busname_to_bus_.at(bus_name);
		}
		return nullptr;
	}

	const data::BusInfo TransportCatalogue::GetBusInfo(std::string_view bus_name) {
		const data::Bus* bus = FindBus(bus_name);
		data::BusInfo res;
		if (bus != nullptr) {
			res.busname_ = bus->busname_;
			auto& stops = bus->stops_;
			res.stops_count = bus->is_circle ? static_cast<int>(stops.size())
				: static_cast<int>(stops.size() * 2 - 1);
			std::unordered_set<const data::Stop*> unique_stops = { stops.begin(),stops.end() };
			res.unique_stops = unique_stops.size();
			res.route_length = CalculateRouteLength(bus_name);
			double route_geo_length = CalculateRouteLengthGeo(bus_name);
			res.curvature = res.route_length / route_geo_length;
		}
		return res;
	}

	const data::StopInfo TransportCatalogue::GetStopInfo(std::string_view stop_name) {
		const data::Stop* stop = FindStop(stop_name);
		data::StopInfo res;
		res.is_exist = false;
		if (stop != nullptr) {
			res.is_exist = true;
			res.stopname_ = stop->stopname_;
			if (stopname_to_buses_.count(stop) > 0) {
				const auto& buses = stopname_to_buses_.at(stop);
				res.buses_ = { buses.begin(),buses.end() };
			}
		}
		return res;
	}

	int TransportCatalogue::GetStopsDistance(const data::Stop* from_stop, const data::Stop* to_stop) {
		auto direct_direction = std::make_pair(from_stop, to_stop);
		auto reverse_direction = std::make_pair(to_stop, from_stop);
		int result = 0;
		if (stops_distances_.count(direct_direction) > 0) {
			result = stops_distances_.at(direct_direction);
			return result;
		}
		if (stops_distances_.count(reverse_direction) > 0) {
			result = stops_distances_.at(reverse_direction);
			return result;
		}
		return result;
	}

	double TransportCatalogue::CalculateRouteLengthGeo(std::string_view bus_name) {
		if (busname_to_bus_.count(bus_name) > 0) {
			const data::Bus* bus = FindBus(bus_name);
			bool is_circular = bus->is_circle;
			double distance = 0;
			auto& stops = bus->stops_;
			for (size_t i = 0; i < stops.size() - 1; ++i) {
				auto stop = stops[i];
				auto next_stop = stops[i + 1];
				distance += is_circular ? geo::ComputeDistance(stop->coordinates_, next_stop->coordinates_)
					: (geo::ComputeDistance(stop->coordinates_, next_stop->coordinates_)
						+ geo::ComputeDistance(next_stop->coordinates_, stop->coordinates_));
			}
			return distance;
		}
		return 0;
	}

	double TransportCatalogue::CalculateRouteLength(std::string_view bus_name) {
		if (busname_to_bus_.count(bus_name) > 0) {
			const data::Bus* bus = FindBus(bus_name);
			bool isCircular = bus->is_circle;
			int distance = 0;
			auto& stops = bus->stops_;
			for (size_t i = 0; i < stops.size() - 1; ++i) {
				auto stop = stops[i];
				auto next_stop = stops[i + 1];
				distance += isCircular ? GetStopsDistance(stop, next_stop)
					: (GetStopsDistance(stop, next_stop)
						+ GetStopsDistance(next_stop, stop));
			}

			return distance;
		}
		return 0;
	}

	void TransportCatalogue::InsertBusToStop(data::Bus* bus) {
		for (const auto& stopp : bus->stops_) {
			stopname_to_buses_[stopp].insert(bus);
		}
	}

	void TransportCatalogue::SetStopsDistances(const data::Stop* from_stop, const data::Stop* to_stop, int distance) {
		if (from_stop == nullptr || to_stop == nullptr) {
			throw std::invalid_argument("Invalid stop in distances");
		}
		stops_distances_[{from_stop, to_stop}] = distance;
	}

	std::unordered_map<std::string_view, data::Bus*> TransportCatalogue::GetBusnameToBuses() const {
		return busname_to_bus_;
	}

	std::unordered_map<std::string_view, data::Stop*> TransportCatalogue::GetStopnameToStops() const {
		return stopname_to_stop_;
	}

	const std::vector<const data::Stop*> TransportCatalogue::GetStopsAsPtrs() {
		std::vector<const data::Stop*> stops_pointers;
		for (const auto& [_, stop_ptr] : stopname_to_stop_) {
			stops_pointers.emplace_back(stop_ptr);
		}
		return stops_pointers;
	}

	const std::vector<const data::Bus*> TransportCatalogue::GetBusesAsPtrs() {
		std::vector<const data::Bus*> buses_pointers;
		for (const auto& [_, bus_ptr] : busname_to_bus_) {
			buses_pointers.emplace_back(bus_ptr);
		}
		return buses_pointers;
	}

}