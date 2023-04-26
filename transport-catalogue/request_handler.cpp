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
			const auto& stop_info = db_.GetStopInfo(stop_name);
			if (stop_info.is_exist) {
				json::Array items;
				if (stop_info.buses_.size() > 0) {
					for (auto& bus_ : stop_info.buses_) {
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
			const auto& bus_info = db_.GetBusInfo(bus_name);
			if (bus_info.stops_count != 0) {
				content["curvature"s] = std::move(json::Node(bus_info.curvature));
				content["route_length"s] = std::move(json::Node(bus_info.route_length));
				content["unique_stop_count"s] = std::move(json::Node(bus_info.unique_stops));
				content["stop_count"s] = std::move(json::Node(bus_info.stops_count));
			}
			else {
				content["error_message"s] = std::move(json::Node("not found"s));
			}
		}
		else if (req_dict.at("type"s).AsString() == "Map"s) {
			std::ostringstream svgMap;
			RenderMap().Render(svgMap);
			content.insert({ "map"s, svgMap.str() });
			//RenderMap()
		}
		responce.push_back(std::move(json::Node(std::move(content))));
	}
	json::Document doc{ std::move(json::Node(responce)) };
	json::Print(doc, out);
}

svg::Document RequestHandler::RenderMap() const {
	const auto route_map_settings = reader_.GetRenderSettings();
	std::set<std::string_view> bus_names;
	std::set<std::string_view> stop_names;
	std::vector<geo::Coordinates> stop_geo_coords;
	const auto& busname_to_buses = db_.GetBusnameToBuses();
	for (const auto& [bus_name, bus] : busname_to_buses) {
		bus_names.insert(bus_name);
		for (const auto& stop : bus->stops_) {
			stop_names.insert(stop->stopname_);
			stop_geo_coords.push_back({ stop->coordinates_.lat,
										stop->coordinates_.lng });
		}
	}

	const renderer::SphereProjector proj{
		stop_geo_coords.begin(), stop_geo_coords.end(),
		route_map_settings.width,route_map_settings.height, route_map_settings.padding
	};
	//std::unordered_map<std::string_view, std::vector<svg::Point>> bus_to_stop_coordinates;
	std::unordered_map<std::string_view, std::pair<bool,std::vector<svg::Point>>> bus_to_stop_coordinates;

	std::unordered_map<std::string_view, svg::Point> stop_to_modified_coordinates;

	for (const auto& bus_sorted : bus_names) {
		auto buss = busname_to_buses.at(bus_sorted);
		bus_to_stop_coordinates[bus_sorted].first = buss->is_circle;
		for (const auto& stop : buss->stops_) {
			bus_to_stop_coordinates[bus_sorted].second.push_back(proj(stop->coordinates_));
			if (stop_to_modified_coordinates.count(stop->stopname_) == 0) {
				stop_to_modified_coordinates[stop->stopname_] = proj(stop->coordinates_);
			}
		}
		/*if (!buss->is_circle) {
			for (int i = static_cast<int>(buss->stops_.size()) - 2; i != -1; i--) {
				bus_to_stop_coordinates[bus_sorted].second.push_back(proj(buss->stops_[i]->coordinates_));
			}
		}*/
	}
	renderer::MapRenderer map_renderer(route_map_settings);
	map_renderer.SetBuses(bus_names);
	map_renderer.SetStops(stop_names);
	map_renderer.SetBusesCoordinatesInfo(bus_to_stop_coordinates);
	map_renderer.SetStopsCoordinatesInfo(stop_to_modified_coordinates);
	return map_renderer.RenderMap();
}
