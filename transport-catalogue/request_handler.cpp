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

svg::Document RequestHandler::RenderMap() const {
	const auto& busname_to_buses = db_.GetBusnameToBuses();
	return map_renderer_.RenderMap(busname_to_buses);
}
