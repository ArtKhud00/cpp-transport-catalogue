#pragma once
#include <memory>
#include <string>

#include "transport_catalogue.h"
#include "graph.h"
#include "router.h"

namespace router {

	struct RouterSettings {
		int bus_wait_time = 0; // время ожидания автобуса в минутах
		int bus_velocity = 0; // скорость автобуса в км/ч
	};

	struct EdgeExtraInfo {
		std::string edge_name;
		double weight_info = 0.0;
		// по полю span_count можем смотреть тип ребра
		// если span_count=0, то тип "Wait"
		// иначе - "Bus"
		int span_count = 0;
	};

	struct RouteData {
		double total_time = 0.0;
		//std::vector<EdgeExtraInfo> items;
		std::vector<EdgeExtraInfo> items;
	};

	class TransportRouter {
	public:
		TransportRouter() = default;

		TransportRouter(catalogue::TransportCatalogue&);

		void SetRouterSettings(RouterSettings&);

		const RouterSettings& GetRouterSettings() const;

		std::optional<const RouteData> CalculateRoute(const std::string_view, const std::string_view);

	private:
		RouterSettings settings_;
		catalogue::TransportCatalogue& tc_;
		graph::DirectedWeightedGraph<double> dw_graph_;
		std::unique_ptr<graph::Router<double>> router_ = nullptr;
		std::unordered_map<std::string_view, size_t> start_wait_;
		std::unordered_map<std::string_view, size_t> stop_wait_;
		//int edge_count = 0;
		std::vector<EdgeExtraInfo> edge_info_;

		void BuildGraph();

		void AddStopsToGraph();

		void AddBusesToGraph();

		const RouteData BuildOptimalRoute(const graph::Router<double>::RouteInfo&) const;
	};
}