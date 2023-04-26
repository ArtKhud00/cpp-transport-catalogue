#include "json_reader.h"

namespace json_reader {
    
    
    void Reader::LoadFromJson(std::istream& input) {
        json::Document doc = json::Load(input);
        json::Node root = doc.GetRoot();
        queries_ = root.AsMap();
        ProcessBaseRequests();
        ProcessRenderSettings();
    }

    void Reader::ProcessBaseRequests() {
        for (auto& rec : queries_.at("base_requests"s).AsArray()) {
            auto info = rec.AsMap();
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
    void Reader::ParseStopsAndAdd() {
        StopToOtherStopsDistances stop_to_other_stops;
        for (auto& one_stop : stops_data_) {
            std::string stop_name = one_stop.at("name"s).AsString();
            double longit = one_stop.at("longitude"s).AsDouble();
            double latit = one_stop.at("latitude"s).AsDouble();
            if (one_stop.count("road_distances") > 0) {
                auto stop_dist = one_stop.at("road_distances").AsMap();
                for (auto& [stop1, dist] : stop_dist) {
                    stop_to_other_stops[stop_name].push_back({ stop1, dist.AsInt() });
                }
            }
            data::Stop stop = { stop_name, {latit, longit} };
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
                //auto test = bus_stop.AsString();
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

    void Reader::ProcessRenderSettings() {

        for (const auto& [key, value] : queries_.at("render_settings"s).AsMap()) {
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
    }
}