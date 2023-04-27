#include "map_renderer.h"

namespace renderer {

    bool IsZero(double value) {
        return std::abs(value) < EPSILON;
    }

    svg::Point SphereProjector::operator()(geo::Coordinates coords) const {
        return {
            (coords.lng - min_lon_) * zoom_coeff_ + padding_,
            (max_lat_ - coords.lat) * zoom_coeff_ + padding_
        };
    }

    void MapRenderer::SetBuses(std::set<std::string_view>& buses_sorted) {
        buses_sorted_ = buses_sorted;
    }
    void MapRenderer::SetStops(std::set<std::string_view>& stops_sorted) {
        stops_sorted_ = stops_sorted;
    }
    void MapRenderer::SetBusesCoordinatesInfo(std::unordered_map<std::string_view, 
                                              std::pair<bool, std::vector<svg::Point>>>& buses_coordinate) {
        bus_to_stop_coordinates_ = buses_coordinate;
    }
    void MapRenderer::SetStopsCoordinatesInfo(std::unordered_map<std::string_view, svg::Point>& stop_to_coordinate) {
        stop_to_modified_coordinates_ = stop_to_coordinate;
    }

    svg::Document MapRenderer::RenderMap(const std::unordered_map<std::string_view, data::Bus*>& busname_to_bus) {
        svg::Document document;
        int colors_count = static_cast<int>(map_settings_.color_palette.size());
        PrepareData(busname_to_bus);
        DrawRouteLine(colors_count, document);
        DrawRouteName(colors_count, document);
        DrawStopCircle(document);
        DrawStopNames(document);
        
        return document;
    }

    void MapRenderer::PrepareData(const std::unordered_map<std::string_view, data::Bus*>& busname_to_bus) {
        std::set<std::string_view> bus_names;
        std::set<std::string_view> stop_names;
        std::vector<geo::Coordinates> stop_geo_coords;
        for (const auto& [bus_name, bus] : busname_to_bus) {
            bus_names.insert(bus_name);
            for (const auto& stop : bus->stops_) {
                stop_names.insert(stop->stopname_);
                stop_geo_coords.push_back({ stop->coordinates_.lat,
                                            stop->coordinates_.lng });
            }
        }
        const renderer::SphereProjector proj{
            stop_geo_coords.begin(), stop_geo_coords.end(),
            map_settings_.width,map_settings_.height, map_settings_.padding
        };
        std::unordered_map<std::string_view, std::pair<bool, std::vector<svg::Point>>> bus_to_stop_coordinates;
        std::unordered_map<std::string_view, svg::Point> stop_to_modified_coordinates;
        for (const auto& bus_sorted : bus_names) {
            auto buss = busname_to_bus.at(bus_sorted);
            bus_to_stop_coordinates[bus_sorted].first = buss->is_circle;
            for (const auto& stop : buss->stops_) {
                bus_to_stop_coordinates[bus_sorted].second.push_back(proj(stop->coordinates_));
                if (stop_to_modified_coordinates.count(stop->stopname_) == 0) {
                    stop_to_modified_coordinates[stop->stopname_] = proj(stop->coordinates_);
                }
            }
        }
        SetBuses(bus_names);
        SetStops(stop_names);
        SetBusesCoordinatesInfo(bus_to_stop_coordinates);
        SetStopsCoordinatesInfo(stop_to_modified_coordinates);
    }

    void MapRenderer::DrawRouteLine(int colors_count, svg::Document& document) {
        int color_index = 0;
        for (const auto& bus : buses_sorted_) {
            svg::Polyline polyline;
            color_index = color_index >= colors_count ? 0 : color_index;
            auto stop_coordinates = bus_to_stop_coordinates_.at(bus).second;
            bool is_round_trip = bus_to_stop_coordinates_.at(bus).first;
            for (auto& coordinates : stop_coordinates) {
                polyline.AddPoint(coordinates);
            }
            if (!is_round_trip) {
                for (int i = static_cast<int>(stop_coordinates.size()) - 2; i != -1; i--) {
                    //bus_to_stop_coordinates[bus_sorted].second.push_back(proj(buss->stops_[i]->coordinates_));
                    polyline.AddPoint(stop_coordinates[i]);
                }
            }
            polyline.SetStrokeColor(map_settings_.color_palette[color_index]);
            polyline.SetStrokeWidth(map_settings_.line_width);
            polyline.SetStrokeLineCap(svg::StrokeLineCap::ROUND);
            polyline.SetStrokeLineJoin(svg::StrokeLineJoin::ROUND);
            polyline.SetFillColor(svg::NoneColor);
            document.Add(std::move(polyline));
            ++color_index;
        }
    }

    void MapRenderer::DrawRouteName(int colors_count, svg::Document& document) {
        int color_index = 0;
        for (const auto& bus : buses_sorted_) {
            svg::Text text_data;
            color_index = color_index >= colors_count ? 0 : color_index;
            auto stop_coordinates = bus_to_stop_coordinates_.at(bus).second;
            bool is_round_trip = bus_to_stop_coordinates_.at(bus).first;

            int stops_amount = static_cast<int>(stop_coordinates.size());
            auto first_stop_coord = stop_coordinates[0];

            const Text base_text =  //
                Text()
                .SetFontFamily("Verdana"s)
                .SetFontSize(map_settings_.bus_label_font_size)
                .SetFontWeight("bold")
                .SetPosition(first_stop_coord)
                .SetOffset(map_settings_.bus_label_offset)
                .SetData(std::string(bus));
            document.Add(Text{base_text}
                .SetStrokeColor(map_settings_.underlayer_color)
                .SetFillColor(map_settings_.underlayer_color)
                .SetStrokeWidth(map_settings_.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
            document.Add(Text{ base_text }.SetFillColor(map_settings_.color_palette[color_index]));
            if (!is_round_trip) {
                auto last_stop_coord = stop_coordinates[stops_amount - 1];
                if (!(last_stop_coord.x == first_stop_coord.x && last_stop_coord.y == first_stop_coord.y)) {
                    const Text base_text =  //
                        Text()
                        .SetFontFamily("Verdana"s)
                        .SetFontSize(map_settings_.bus_label_font_size)
                        .SetFontWeight("bold")
                        .SetPosition(last_stop_coord)
                        .SetOffset(map_settings_.bus_label_offset)
                        .SetData(std::string(bus));
                    document.Add(Text{ base_text }
                        .SetStrokeColor(map_settings_.underlayer_color)
                        .SetFillColor(map_settings_.underlayer_color)
                        .SetStrokeWidth(map_settings_.underlayer_width)
                        .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                        .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
                    document.Add(Text{ base_text }.SetFillColor(map_settings_.color_palette[color_index]));
                }
            }
            ++color_index;
        }
    }

    void MapRenderer::DrawStopCircle(svg::Document& document) {
        for (auto& stop_ : stops_sorted_) {
            auto stop_coord = stop_to_modified_coordinates_.at(stop_);
            svg::Circle stop_circle = Circle()
                .SetCenter(stop_coord)
                .SetRadius(map_settings_.stop_radius)
                .SetFillColor("white");
            document.Add(stop_circle);
        }
    }

    void MapRenderer::DrawStopNames(svg::Document& document) {
        for (auto& stop_ : stops_sorted_) {
            auto stop_coord = stop_to_modified_coordinates_.at(stop_);
            const Text base_text =  //
                Text()
                .SetFontFamily("Verdana"s)
                .SetFontSize(map_settings_.stop_label_font_size)
                .SetPosition(stop_coord)
                .SetOffset(map_settings_.stop_label_offset)
                .SetData(std::string(stop_));
            document.Add(Text{ base_text }
                .SetStrokeColor(map_settings_.underlayer_color)
                .SetFillColor(map_settings_.underlayer_color)
                .SetStrokeWidth(map_settings_.underlayer_width)
                .SetStrokeLineCap(svg::StrokeLineCap::ROUND)
                .SetStrokeLineJoin(svg::StrokeLineJoin::ROUND));
            document.Add(Text{ base_text }.SetFillColor("black"));
        }
    }
}