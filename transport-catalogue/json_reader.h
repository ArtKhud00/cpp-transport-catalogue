#pragma once
#include "json.h"
#include "transport_catalogue.h"
#include "map_renderer.h"
#include <string>
#include <sstream>


using namespace std::literals;



namespace json_reader {
    class Reader 
    {
    public:
        //Reader () = default;
        using StopToOtherStopsDistances = std::unordered_map<std::string, std::vector<std::pair<std::string, int>>>;
        Reader(catalogue::TransportCatalogue& tc): tc_(tc) {}
        
        void LoadFromJson(std::istream& in);

        json::Array GetStatRequests();

        renderer::RouteMapSettings& GetRenderSettings();

    private:
        json::Dict queries_;
        renderer::RouteMapSettings route_map_settings_;
        std::vector<json::Dict> stops_data_;
        std::vector<json::Dict> buses_data_;
        catalogue::TransportCatalogue& tc_;

        void ProcessBaseRequests();
        void ProcessRenderSettings();

        void ParseStopsAndAdd();
        void ParseDistancesAndSet(StopToOtherStopsDistances& stop_to_other_stops);
        void ParseBusesAndAdd();


    };



    
}
