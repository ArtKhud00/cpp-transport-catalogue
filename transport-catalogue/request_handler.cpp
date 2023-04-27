#include "request_handler.h"
void RequestHandler::JsonOut(std::ostream& out) {
	json::Array responce;
	auto requests = reader_.GetStatRequests();
	for (auto& req : requests) {
		auto req_dict = req.AsMap();
		json::Dict content;
		content["request_id"s] = std::move(json::Node(req_dict.at("id").AsInt()));
		if (req_dict.at("type"s).AsString() == "Stop"s) {
			std::string stop_name = req_dict.at("name"s).AsString();
			const auto& stop_info = GetStopStat(stop_name);
			if (stop_info->is_exist) {
				json::Array items;
				if (stop_info->buses_.size() > 0) {
					for (auto& bus_ : stop_info->buses_) {
						items.push_back(std::move(json::Node(bus_->busname_)));
					}
				}
				content["buses"s] = std::move(json::Node(std::move(items)));
			}
			else {
				content["error_message"s] = std::move(json::Node("not found"s));
			}

		}
		else if (req_dict.at("type"s).AsString() == "Bus"s) {
			std::string bus_name = req_dict.at("name"s).AsString();
			const auto& bus_info = GetBusStat(bus_name);
			if (bus_info->stops_count != 0) {
				content["curvature"s] = std::move(json::Node(bus_info->curvature));
				content["route_length"s] = std::move(json::Node(bus_info->route_length));
				content["unique_stop_count"s] = std::move(json::Node(bus_info->unique_stops));
				content["stop_count"s] = std::move(json::Node(bus_info->stops_count));
			}
			else {
				content["error_message"s] = std::move(json::Node("not found"s));
			}
		}
		else if (req_dict.at("type"s).AsString() == "Map"s) {
			std::ostringstream svgMap;
			RenderMap().Render(svgMap);
			content.insert({ "map"s, svgMap.str() });
		}
		responce.push_back(std::move(json::Node(std::move(content))));
	}
	json::Document doc{ std::move(json::Node(responce)) };
	json::Print(doc, out);
}

std::optional<const data::BusInfo> RequestHandler::GetBusStat(const std::string_view& bus_name) const {
	return db_.GetBusInfo(bus_name);
}

std::optional<const data::StopInfo> RequestHandler::GetStopStat(const std::string_view& stop_name) const {
	return db_.GetStopInfo(stop_name);
}

svg::Document RequestHandler::RenderMap() const {
	const auto route_map_settings = reader_.GetRenderSettings();
	const auto& busname_to_buses = db_.GetBusnameToBuses();
	renderer::MapRenderer map_renderer(route_map_settings);
	return map_renderer.RenderMap(busname_to_buses);
}
