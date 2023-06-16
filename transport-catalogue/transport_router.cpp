#include "transport_router.h"
#include <iostream>

namespace router {
	TransportRouter::TransportRouter(catalogue::TransportCatalogue& tc) : tc_(tc) {
		size_t tmp_size = tc_.GetStopnameToStops().size();
		dw_graph_ = graph::DirectedWeightedGraph<double>(tmp_size * 2);
	}

	void TransportRouter::SetRouterSettings(RouterSettings& router_settings) {
		settings_ = std::move(router_settings);
	}

	const RouterSettings& TransportRouter::GetRouterSettings() const {
		return settings_;
	}

	std::optional<const RouteData> TransportRouter::CalculateRoute(const std::string_view from, const std::string_view to) {
		
		if (!router_) {
			BuildGraph();
		}
		const data::Stop* stop_departure = tc_.FindStop(from);
		const data::Stop* stop_destination = tc_.FindStop(to);
		
		if (stop_departure == nullptr || stop_destination == nullptr) {
			return std::nullopt;
		}

		const size_t stop_from = start_wait_.at(from);
		const size_t stop_to = start_wait_.at(to);
		auto calculated_route = router_->BuildRoute(stop_from, stop_to);
		if (!calculated_route) {
			return std::nullopt;
		}
		return BuildOptimalRoute(calculated_route.value());
	}

	void TransportRouter::BuildGraph() {
		AddStopsToGraph();
		AddBusesToGraph();
		router_ = std::make_unique<graph::Router<double>>(dw_graph_);
	}

	void TransportRouter::AddStopsToGraph() {
		const std::vector<const data::Stop*> all_stops = tc_.GetStopsAsPtrs();
		int vertex_id = -1;
		for (const auto& stop : all_stops) {
			start_wait_.insert({ stop->stopname_,++vertex_id });
			stop_wait_.insert({ stop->stopname_,++vertex_id });
			dw_graph_.AddEdge({
					start_wait_.at(stop->stopname_),
					stop_wait_.at(stop->stopname_),
					settings_.bus_wait_time * 1.0
					});
			edge_info_.push_back({
					stop->stopname_,
					settings_.bus_wait_time * 1.0,
					0
					});
		}
	}

	void TransportRouter::AddBusesToGraph() {
		const std::vector<const data::Bus*> all_buses = tc_.GetBusesAsPtrs();
		for (const auto& bus : all_buses) {
			size_t stops_count =  bus->stops_.size();
			// прямое направление
			for (size_t ind_from = 0; ind_from < stops_count; ++ind_from) {
				int span_count = 0;
				for (size_t ind_to = ind_from + 1; ind_to < stops_count; ++ind_to) {
					double road_distance = 0.0;
					if (bus->stops_[ind_to]->stopname_ != bus->stops_[ind_from]->stopname_)
					{
						for (size_t it = ind_from; it < ind_to; ++it) {
							road_distance += 1.0 * (tc_.GetStopsDistance(bus->stops_[it], bus->stops_[it + 1]));
						}
						dw_graph_.AddEdge({
								stop_wait_.at(bus->stops_[ind_from]->stopname_),
								start_wait_.at(bus->stops_[ind_to]->stopname_),
								road_distance / (settings_.bus_velocity * 1000.0 / 60.0)
							});
						edge_info_.push_back({
								bus->busname_,
								road_distance / (settings_.bus_velocity * 1000.0 / 60.0),
								++span_count
							});
					}
				}
			}
			//обратное направление
			if (!bus->is_circle) {
				for (size_t ind_from = stops_count - 1; ind_from != -1; ind_from--) {
					int span_count = 0;
					for (size_t ind_to = ind_from - 1; ind_to != -1; ind_to--) {
						double road_distance = 0.0;
						for (size_t it = ind_from; it > ind_to; it--) {
							road_distance += 1.0 * (tc_.GetStopsDistance(bus->stops_[it], bus->stops_[it - 1]));
						}
						dw_graph_.AddEdge({
								stop_wait_.at(bus->stops_[ind_from]->stopname_),
								start_wait_.at(bus->stops_[ind_to]->stopname_),
								road_distance / (settings_.bus_velocity * 1000.0 / 60.0)
							});
						edge_info_.push_back({
								bus->busname_,
								road_distance / (settings_.bus_velocity * 1000.0 / 60.0),
								++span_count
							});
					}
				}
			}
		}
	}

	const RouteData TransportRouter::BuildOptimalRoute(const graph::Router<double>::RouteInfo& route_info) const {
		RouteData result;
		result.total_time = route_info.weight;
		for (auto& edge : route_info.edges) {
			result.items.emplace_back(edge_info_[edge]);
		}
		return result;
	}
}