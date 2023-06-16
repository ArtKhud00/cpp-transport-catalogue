#include "request_handler.h"

std::optional<const data::BusInfo> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
	return db_.GetBusInfo(bus_name);
}

std::optional<const data::StopInfo> RequestHandler::GetStopStat(const std::string_view& stop_name) const {
	return db_.GetStopInfo(stop_name);
}

void RequestHandler::SetRenderSettings(const renderer::RouteMapSettings& map_settings) {
	map_renderer_.SetRenderSettings(map_settings);
}

void RequestHandler::SetRouterSettings(router::TransportRouter& tr , router::RouterSettings& router_settings) {
	tr.SetRouterSettings(router_settings);
}

svg::Document RequestHandler::RenderMap() const {
	//const auto route_map_settings = reader_.GetRenderSettings();
	const auto& busname_to_buses = db_.GetBusnameToBuses();
	//renderer::MapRenderer map_renderer(route_map_settings);
	return map_renderer_.RenderMap(busname_to_buses);
}

std::optional<const router::RouteData> RequestHandler::CalculateRoute(router::TransportRouter& tr,const std::string_view from, const std::string_view to) {
	return tr.CalculateRoute(from, to);
}