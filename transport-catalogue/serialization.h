#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"

#include <filesystem>
#include <vector>
#include <fstream>

#include <transport_catalogue.pb.h>
#include <map_renderer.pb.h>
#include <transport_router.pb.h>
#include <svg.pb.h>


namespace serializer {
	using Path = std::filesystem::path;

	struct SerializationSettings {
		Path path;
	};

	class Serialization {
	public:
		Serialization(catalogue::TransportCatalogue& tc, renderer::RouteMapSettings& ms, router::RouterSettings& rs);

		void SetSerializerSettings(const SerializationSettings& serializatiion_settings);

		void Serialize();

		void Deserialize();

	private:
		SerializationSettings serializatiion_settings_;
		trasport_catalogue_serialize::TransportCatalogue serialized_catalogue_;
		renderer::RouteMapSettings& map_settings_;
		router::RouterSettings& router_settings_;
		catalogue::TransportCatalogue& tc_;


		void SerializeStops();
		void SerializeBuses();
		void SerializeDistances();
		svg_serialize::Color SerializeColor(const svg::Color& color);
		void SerializeMapRender();
		void SerializeRouter();

		void DeserializeStops();
		void DeserializeBuses();
		void DeserializeDistances();
		svg::Color DeserializeColor(const svg_serialize::Color& serialized_color);
		void DeserializeMapRender();
		void DeserializeRouter();

	};

} //namespace serializer