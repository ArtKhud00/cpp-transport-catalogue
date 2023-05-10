#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "request_handler.h"
#include "map_renderer.h"
#include <string>
#include <sstream>

using namespace std::literals;

namespace json_reader {
    class Reader 
    {
    public:
        using StopToOtherStopsDistances = std::unordered_map<std::string, std::vector<std::pair<std::string, int>>>;
        Reader(catalogue::TransportCatalogue& tc, RequestHandler& handler): tc_(tc), handler_(handler) {}
        
        void LoadFromJson(std::istream& in);

        json::Array GetStatRequests();

        renderer::RouteMapSettings& GetRenderSettings();

        void ProcessStatRequests(std::ostream& out);

    private:
        catalogue::TransportCatalogue& tc_;
        RequestHandler& handler_;
        json::Dict queries_;
        renderer::RouteMapSettings route_map_settings_;
        std::vector<json::Dict> stops_data_;
        std::vector<json::Dict> buses_data_;
        
        void ProcessBaseRequests();
        void ProcessRenderSettings();

        void ParseStopsAndAdd();
        void ParseDistancesAndSet(StopToOtherStopsDistances& stop_to_other_stops);
        void ParseBusesAndAdd();

        void ProcessStatStop(json::Node, json::Array&);
        void ProcessStatBus(json::Node, json::Array&);
        void ProcessStatMap(json::Node, json::Array&);
    };
}
