#include "json_reader.h"
#include "json_builder.h"

namespace json_reader {
    
    void Reader::LoadFromJson(std::istream& input) {
        json::Document doc = json::Load(input);
        json::Node root = doc.GetRoot();
        queries_ = root.AsDict();
        ProcessBaseRequests();
        ProcessRenderSettings();
        ProcessRoutingSettings();
        ProcessSerializingSettings();
    }

    void Reader::ProcessBaseRequests() {
        if (queries_.count("base_requests"s) > 0) {
            for (auto& rec : queries_.at("base_requests"s).AsArray()) {
                auto info = rec.AsDict();
                if (info.at("type"s).AsString() == "Bus"s) {
                    buses_data_.push_back(info);
                }
                if (info.at("type"s).AsString() == "Stop"s) {
                    stops_data_.push_back(info);
                }
            }
            ParseStopsAndAdd();
            ParseBusesAndAdd();
        }
    }

    void Reader::ParseStopsAndAdd() {
        StopToOtherStopsDistances stop_to_other_stops;
        int id = 0;
        for (auto& one_stop : stops_data_) {
            std::string stop_name = one_stop.at("name"s).AsString();
            double longit = one_stop.at("longitude"s).AsDouble();
            double latit = one_stop.at("latitude"s).AsDouble();
            if (one_stop.count("road_distances") > 0) {
                auto stop_dist = one_stop.at("road_distances").AsDict();
                for (auto& [stop1, dist] : stop_dist) {
                    stop_to_other_stops[stop_name].push_back({ stop1, dist.AsInt() });
                }
            }
            data::Stop stop = { stop_name, {latit, longit}, id++ };
            tc_.AddStop(std::move(stop));
        }
        ParseDistancesAndSet(stop_to_other_stops);
    }

    void Reader::ParseDistancesAndSet(StopToOtherStopsDistances& stop_to_other_stops) {
        for (auto& [stop_name, data] : stop_to_other_stops) {
            const data::Stop* from_stop = tc_.FindStop(stop_name);
            for (const auto& [stop, distance] : data) {
                const data::Stop* to_stop = tc_.FindStop(stop);
                tc_.SetStopsDistances(from_stop, to_stop, distance);
            }
        }
    }

    void Reader::ParseBusesAndAdd() {
        for (auto& one_bus : buses_data_) {
            std::string bus_name = one_bus.at("name"s).AsString();
            bool round_trip = one_bus.at("is_roundtrip"s).AsBool();
            auto bus_stops = one_bus.at("stops").AsArray();
            std::vector<const data::Stop*> stops;
            for (const auto& bus_stop : bus_stops) {
                stops.push_back(tc_.FindStop(bus_stop.AsString()));
            }
            data::Bus bus = { bus_name, stops, round_trip };
            tc_.AddBus(std::move(bus));
        }
    }

    json::Array Reader::GetStatRequests() {
        return queries_.at("stat_requests").AsArray();
    }

    renderer::RouteMapSettings& Reader::GetRenderSettings() {
        return route_map_settings_;
    }


    void Reader::MakeBaseCommand(std::istream& in) {
        LoadFromJson(in);
        serializer::Serialization serializer(tc_,route_map_settings_, router_settings_);
        handler_.SetSerializationSettings(serializer, serialization_settings_);
        handler_.Serialize(serializer);
    }

    void Reader::ProcessRequestsCommand(std::istream& in, std::ostream& out) {
        LoadFromJson(in);
        serializer::Serialization deserializer(tc_,route_map_settings_, router_settings_);
        handler_.SetSerializationSettings(deserializer, serialization_settings_);
        handler_.Deserialize(deserializer);
        handler_.SetRenderSettings(route_map_settings_);
        ProcessStatRequests(out);
    }

    void Reader::ProcessRenderSettings() {
        if (queries_.count("render_settings") > 0) {
            for (const auto& [key, value] : queries_.at("render_settings"s).AsDict()) {
                if (key == "width"s) {
                    route_map_settings_.width = value.AsDouble();
                    continue;
                }
                if (key == "height"s) {
                    route_map_settings_.height = value.AsDouble();
                    continue;
                }
                if (key == "padding"s) {
                    route_map_settings_.padding = value.AsDouble();
                    continue;
                }
                if (key == "line_width"s) {
                    route_map_settings_.line_width = value.AsDouble();
                    continue;
                }
                if (key == "stop_radius"s) {
                    route_map_settings_.stop_radius = value.AsDouble();
                    continue;
                }
                if (key == "bus_label_font_size"s) {
                    route_map_settings_.bus_label_font_size = value.AsInt();
                    continue;
                }
                if (key == "bus_label_offset"s) {
                    if (value.IsArray()) {
                        const auto arr = value.AsArray();
                        if (arr.size() > 1) {
                            route_map_settings_.bus_label_offset.x = arr[0].AsDouble();
                            route_map_settings_.bus_label_offset.y = arr[1].AsDouble();
                        }
                    }
                    continue;
                }
                if (key == "stop_label_font_size"s) {
                    route_map_settings_.stop_label_font_size = value.AsInt();
                    continue;
                }
                if (key == "stop_label_offset"s) {
                    if (value.IsArray()) {
                        const auto arr = value.AsArray();
                        if (arr.size() > 1) {
                            route_map_settings_.stop_label_offset.x = arr[0].AsDouble();
                            route_map_settings_.stop_label_offset.y = arr[1].AsDouble();
                        }
                    }
                    continue;
                }
                if (key == "underlayer_color"s) {
                    if (value.IsString()) {
                        route_map_settings_.underlayer_color = value.AsString();
                        continue;
                    }
                    if (value.IsArray()) {
                        const auto arr = value.AsArray();
                        if (arr.size() == 3) {
                            svg::Rgb color(arr[0].AsInt(), arr[1].AsInt(), arr[2].AsInt());
                            route_map_settings_.underlayer_color = std::move(color);
                        }
                        if (arr.size() == 4) {
                            svg::Rgba color(arr[0].AsInt(), arr[1].AsInt(), arr[2].AsInt(), arr[3].AsDouble());
                            route_map_settings_.underlayer_color = std::move(color);
                        }
                    }
                }
                if (key == "underlayer_width"s) {
                    route_map_settings_.underlayer_width = value.AsDouble();
                }
                if (key == "color_palette"s) {
                    if (value.IsArray()) {
                        auto colors = value.AsArray();
                        for (auto& col : colors) {

                            if (col.IsString()) {
                                route_map_settings_.color_palette.push_back(col.AsString());
                                continue;
                            }
                            if (col.IsArray()) {
                                const auto arr = col.AsArray();
                                if (arr.size() == 3) {
                                    svg::Rgb color(arr[0].AsInt(), arr[1].AsInt(), arr[2].AsInt());
                                    route_map_settings_.color_palette.push_back(std::move(color));
                                }
                                if (arr.size() == 4) {
                                    svg::Rgba color(arr[0].AsInt(), arr[1].AsInt(), arr[2].AsInt(), arr[3].AsDouble());
                                    route_map_settings_.color_palette.push_back(std::move(color));
                                }
                                continue;
                            }
                        }
                    }
                    continue;
                }

            }
            handler_.SetRenderSettings(route_map_settings_);
        }
    }

    void Reader::ProcessRoutingSettings() {
        if (queries_.count("routing_settings") > 0) {
            for (const auto& [key, value] : queries_.at("routing_settings"s).AsDict()) {
                if (key == "bus_velocity"s) {
                    router_settings_.bus_velocity = value.AsInt();
                    continue;
                }
                if (key == "bus_wait_time"s) {
                    router_settings_.bus_wait_time = value.AsInt();
                    continue;
                }
            }
        }
    }

    void Reader::ProcessSerializingSettings() {
        if (queries_.count("serialization_settings"s) > 0) {
            for (const auto& [key, value] : queries_.at("serialization_settings"s).AsDict()) {
                if (key == "file"s) {
                    serialization_settings_.path = value.AsString();
                }
            }
        }
    }
    
    void Reader::ProcessStatStop(json::Node elem, json::Array& responce) {
        auto& elem_dict = elem.AsDict();
        int id = elem_dict.at("id").AsInt();
        std::string stop_name = elem_dict.at("name"s).AsString();
        const auto& stop_info = handler_.GetStopStat(stop_name);
        if (stop_info->is_exist) {
            std::vector<std::string> bus_names;
            if (stop_info->buses_.size() > 0) {
                for (auto& bus_ : stop_info->buses_) {
                    bus_names.push_back(bus_->busname_);
                }
            }
            responce.push_back(json::Builder{}
                .StartDict()
                .Key("request_id"s)
                .Value(id)
                .Key("buses"s)
                .Value(json::Array{ bus_names.begin(), bus_names.end() })
                .EndDict()
                .Build());
        }
        else {
            responce.push_back(json::Builder{}
                .StartDict()
                .Key("request_id"s)
                .Value(id)
                .Key("error_message"s)
                .Value("not found"s)
                .EndDict()
                .Build());
        }
    }

    void Reader::ProcessStatBus(json::Node elem, json::Array& responce) {
        auto& elem_dict = elem.AsDict();
        int id = elem_dict.at("id").AsInt();
        std::string bus_name = elem_dict.at("name"s).AsString();
        const auto& bus_info = handler_.GetBusStat(bus_name);

        if (bus_info->stops_count != 0) {
            responce.push_back(json::Builder{}
                .StartDict()
                .Key("request_id"s)
                .Value(id)
                .Key("curvature"s)
                .Value(bus_info->curvature)
                .Key("route_length"s)
                .Value(bus_info->route_length)
                .Key("unique_stop_count"s)
                .Value(bus_info->unique_stops)
                .Key("stop_count"s)
                .Value(bus_info->stops_count)
                .EndDict()
                .Build());
        }
        else {
            responce.push_back(json::Builder{}
                .StartDict()
                .Key("request_id"s)
                .Value(id)
                .Key("error_message"s)
                .Value("not found"s)
                .EndDict()
                .Build());
        }

    }

    void Reader::ProcessStatMap(json::Node elem, json::Array& responce) {
        int id = elem.AsDict().at("id").AsInt();
        std::ostringstream svgMap;
        handler_.RenderMap().Render(svgMap);
        responce.push_back(json::Builder{}
            .StartDict()
            .Key("request_id"s)
            .Value(id)
            .Key("map"s)
            .Value(svgMap.str())
            .EndDict()
            .Build());
    }


    json::Array Reader::CreateArrayOfItems(const std::vector<router::EdgeExtraInfo>& items) {
        json::Array array_of_items;
        for (const auto& item : items) {
            if (item.span_count == 0) {
                array_of_items.push_back(json::Builder{}
                    .StartDict()
                    .Key("stop_name"s)
                    .Value(item.edge_name)
                    .Key("time"s)
                    .Value(item.weight_info)
                    .Key("type"s)
                    .Value("Wait"s)
                    .EndDict()
                    .Build());
            }
            else {
                array_of_items.push_back(json::Builder{}
                    .StartDict()
                    .Key("bus"s)
                    .Value(item.edge_name)
                    .Key("span_count"s)
                    .Value(item.span_count)
                    .Key("time")
                    .Value(item.weight_info)
                    .Key("type")
                    .Value("Bus"s)
                    .EndDict()
                    .Build());
            }
        }
        return array_of_items;
    }

    void Reader::ProcessStatRoute(router::TransportRouter& tr, json::Node elem, json::Array& responce) {
        int id = elem.AsDict().at("id").AsInt();
        std::string from_stop = elem.AsDict().at("from").AsString();
        std::string to_stop = elem.AsDict().at("to").AsString();
        auto calculated_route = handler_.CalculateRoute(tr, from_stop, to_stop);

		if (!calculated_route.has_value()){
			responce.push_back(json::Builder{}
			    .StartDict()
				.Key("request_id"s)
				.Value(id)
				.Key("error_message"s)
				.Value("not found"s)
				.EndDict()
				.Build());
        }
        else {
            json::Array array_of_items = CreateArrayOfItems(calculated_route.value().items);
            responce.push_back(json::Builder{}
            .StartDict()
                .Key("total_time"s)
                .Value(calculated_route.value().total_time)
                .Key("request_id"s)
                .Value(id)
                .Key("items"s)
                .Value(array_of_items)
                .EndDict()
                .Build());
        }
    }
    
    void Reader::ProcessStatRequests(std::ostream& out) {
        json::Array responce;
        auto requests = GetStatRequests();
        router::TransportRouter tr(tc_);
        handler_.SetRouterSettings(tr, router_settings_);
        for (auto& req : requests) {
            auto& req_dict = req.AsDict();
            std::string_view type = req_dict.at("type"s).AsString();
            if (type == "Stop"sv) {
                ProcessStatStop(req, responce);
            }
            else if (type == "Bus"sv) {
                ProcessStatBus(req, responce);
            }
            else if (type == "Map"sv) {
                ProcessStatMap(req, responce);
            }
            else if (type == "Route"sv) {
                ProcessStatRoute(tr, req, responce);
            }
        }
        json::Document doc{ std::move(json::Node(responce)) };
        json::Print(doc, out);
        
    }
}
