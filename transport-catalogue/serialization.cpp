#include "serialization.h"

namespace serializer {
	using namespace std::literals;
	
	Serialization::Serialization(catalogue::TransportCatalogue& tc, renderer::RouteMapSettings& ms, router::RouterSettings& rs)
		: tc_(tc)
		, map_settings_(ms)
		, router_settings_(rs) {
	};
	
	void Serialization::SetSerializerSettings(const SerializationSettings& serializatiion_settings) {
		serializatiion_settings_ = serializatiion_settings;
	}

	void Serialization::Serialize() {
		SerializeStops();
		SerializeBuses();
		SerializeDistances();
		SerializeMapRender();
		SerializeRouter();
		std::ofstream out_file(serializatiion_settings_.path, std::ios::binary);
		serialized_catalogue_.SerializeToOstream(&out_file);
	}

	void Serialization::Deserialize() {
		std::ifstream in_file(serializatiion_settings_.path, std::ios::binary);
		serialized_catalogue_.ParseFromIstream(&in_file);
		DeserializeStops();
		DeserializeBuses();
		DeserializeDistances();
		DeserializeMapRender();
		DeserializeRouter();
	}


	void Serialization::SerializeStops() {
		const std::vector<const data::Stop*> stops = tc_.GetStopsAsPtrs();
		std::vector<trasport_catalogue_serialize::Stop> serialized_stops_list(stops.size());
		*serialized_catalogue_.mutable_stops() = { serialized_stops_list.begin(), serialized_stops_list.end() };
		for (const auto& stop_ : stops) {
			trasport_catalogue_serialize::Stop* stop_serialized = serialized_catalogue_.mutable_stops(stop_->id);
			stop_serialized->set_name(stop_->stopname_); 
			stop_serialized->mutable_coordinates()->set_lat(stop_->coordinates_.lat);
			stop_serialized->mutable_coordinates()->set_lng(stop_->coordinates_.lng);
			stop_serialized->set_id(stop_->id);
		}
	}

	void Serialization::SerializeBuses() {
		const std::vector<const data::Bus*> buses = tc_.GetBusesAsPtrs();
		for (const auto& bus_ : buses) {
			trasport_catalogue_serialize::Bus bus_serialized;
			bus_serialized.set_name(bus_->busname_);
			for (const auto& stop : bus_->stops_) {
				bus_serialized.add_stops_index(stop->id);
			}
			bus_serialized.set_is_circle(bus_->is_circle);
			*serialized_catalogue_.add_buses() = std::move(bus_serialized);
		}
	}

	void Serialization::SerializeDistances() {
		const catalogue::DistancesBetweenStops distances = tc_.GetDistances();
		for (const auto& [stops, distance] : distances) {
			trasport_catalogue_serialize::Distances distance_serialized;
			distance_serialized.set_stop_from_index(stops.first->id);
			distance_serialized.set_stop_to_index(stops.second->id);
			distance_serialized.set_distance(distance);
			*serialized_catalogue_.add_distances() = std::move(distance_serialized);
		}
	}

	svg_serialize::Color Serialization::SerializeColor(const svg::Color& color) {
		svg_serialize::Color serialized_color;
		if (std::holds_alternative<std::string>(color)) {
			serialized_color.set_color_string(std::get<std::string>(color));
		}
		else if (std::holds_alternative<svg::Rgb>(color)) {
			svg_serialize::Rgb* rgb = serialized_color.mutable_color_rgb();
			rgb->set_red(std::get<svg::Rgb>(color).red);
			rgb->set_green(std::get<svg::Rgb>(color).green);
			rgb->set_blue(std::get<svg::Rgb>(color).blue);
		}
		else if (std::holds_alternative<svg::Rgba>(color)) {
			svg_serialize::Rgba* rgba = serialized_color.mutable_color_rgba();
			rgba->set_red(std::get<svg::Rgba>(color).red);
			rgba->set_green(std::get<svg::Rgba>(color).green);
			rgba->set_blue(std::get<svg::Rgba>(color).blue);
			rgba->set_opacity(std::get<svg::Rgba>(color).opacity);
		}
		else {
			serialized_color.set_color_string("monostate"s);
		}
		return serialized_color;
	}

	void Serialization::SerializeMapRender() {
		map_renderer_serialize::MapSettings* render_settings_serialize = serialized_catalogue_.mutable_render_settings();
		render_settings_serialize->set_width(map_settings_.width);
		render_settings_serialize->set_height(map_settings_.height);
		render_settings_serialize->set_padding(map_settings_.padding);
		render_settings_serialize->set_line_width(map_settings_.line_width);
		render_settings_serialize->set_stop_radius(map_settings_.stop_radius);
		render_settings_serialize->set_bus_label_font_size(map_settings_.bus_label_font_size);
		render_settings_serialize->set_stop_label_font_size(map_settings_.stop_label_font_size);
		render_settings_serialize->set_underlayer_width(map_settings_.underlayer_width);
		
		render_settings_serialize->mutable_bus_label_offset()->set_x(map_settings_.bus_label_offset.x);
		render_settings_serialize->mutable_bus_label_offset()->set_y(map_settings_.bus_label_offset.y);
		render_settings_serialize->mutable_stop_label_offset()->set_x(map_settings_.stop_label_offset.x);
		render_settings_serialize->mutable_stop_label_offset()->set_y(map_settings_.stop_label_offset.y);
		*render_settings_serialize->mutable_underlayer_color() = SerializeColor(map_settings_.underlayer_color);
		for (const svg::Color& color_ : map_settings_.color_palette) {
			*render_settings_serialize->add_color_palette() = SerializeColor(color_);
		}

	}

	void Serialization::SerializeRouter() {
		router_serialize::RouterSettings* router_settings_serialize = serialized_catalogue_.mutable_router_settings();
		router_settings_serialize->set_bus_wait_time(router_settings_.bus_wait_time);
		router_settings_serialize->set_bus_velocity(router_settings_.bus_velocity);
	}


	void Serialization::DeserializeStops() {
		for (const auto& stop_ : serialized_catalogue_.stops()) {
			data::Stop temp_stop = { stop_.name(), {stop_.coordinates().lat(), stop_.coordinates().lng()}, static_cast<int>(stop_.id()) };
			tc_.AddStop(temp_stop);
		}
	}

	void Serialization::DeserializeBuses() {
		int stop_size = serialized_catalogue_.stops_size();
		for (const auto& bus_ : serialized_catalogue_.buses()) {
			std::string busname = bus_.name();
			bool is_circle = bus_.is_circle();
			std::vector<const data::Stop*> stops_;
			
			stops_.reserve(bus_.stops_index_size());
			for (const auto& stop_index : bus_.stops_index()) {
				std::string stop_name = serialized_catalogue_.stops(stop_index).name();
				stops_.emplace_back(tc_.FindStop(stop_name));
			}
			tc_.AddBus({ busname, std::move(stops_), is_circle });
		}
	}

	void Serialization::DeserializeDistances() {
		int stop_size = serialized_catalogue_.stops_size();
		for (const auto& dist_ : serialized_catalogue_.distances()) {
			std::string stop_from_name = serialized_catalogue_.mutable_stops(dist_.stop_from_index())->name();
			std::string stop_to_name = serialized_catalogue_.mutable_stops(dist_.stop_to_index())->name();
			const data::Stop* stop_from = tc_.FindStop(stop_from_name);
			const data::Stop* stop_to = tc_.FindStop(stop_to_name);
			tc_.SetStopsDistances(stop_from, stop_to, dist_.distance());
		}
	}

	svg::Color Serialization::DeserializeColor(const svg_serialize::Color& serialized_color) {
		if (serialized_color.has_color_rgb()) {
			svg::Rgb rgb_color;
			rgb_color.red = serialized_color.color_rgb().red();
			rgb_color.green = serialized_color.color_rgb().green();
			rgb_color.blue = serialized_color.color_rgb().blue();
			return rgb_color;
		}
		else if (serialized_color.has_color_rgba()) {
			svg::Rgba rgba_color;
			rgba_color.red = serialized_color.color_rgba().red();
			rgba_color.green = serialized_color.color_rgba().green();
			rgba_color.blue = serialized_color.color_rgba().blue();
			rgba_color.opacity = serialized_color.color_rgba().opacity();
			return rgba_color;
		}
		else  {
			std::string color = serialized_color.color_string();
			if (color != "monostate"s) {
				return color;
			}
		}
		return std::monostate();
	}

	void Serialization::DeserializeMapRender() {
		map_renderer_serialize::MapSettings* render_settings_serialized = serialized_catalogue_.mutable_render_settings();
		map_settings_.width = render_settings_serialized->width();
		map_settings_.height = render_settings_serialized->height();

		map_settings_.padding = render_settings_serialized->padding();
		map_settings_.line_width = render_settings_serialized->line_width();
		map_settings_.stop_radius = render_settings_serialized->stop_radius();
		map_settings_.bus_label_font_size = render_settings_serialized->bus_label_font_size();
		map_settings_.stop_label_font_size = render_settings_serialized->stop_label_font_size();
		map_settings_.underlayer_width = render_settings_serialized->underlayer_width();
		map_settings_.stop_label_offset.x = render_settings_serialized->stop_label_offset().x();
		map_settings_.stop_label_offset.y = render_settings_serialized->stop_label_offset().y();
		map_settings_.bus_label_offset.x = render_settings_serialized->bus_label_offset().x();
		map_settings_.bus_label_offset.y = render_settings_serialized->bus_label_offset().y();
		map_settings_.underlayer_color = DeserializeColor(render_settings_serialized->underlayer_color());
		map_settings_.color_palette.reserve(render_settings_serialized->color_palette_size());
		for (const svg_serialize::Color& color_serialized : render_settings_serialized->color_palette()) {
			map_settings_.color_palette.emplace_back(DeserializeColor(color_serialized));
		}
	}

	void Serialization::DeserializeRouter() {
		router_serialize::RouterSettings* router_settings_serialize = serialized_catalogue_.mutable_router_settings();
		router_settings_.bus_velocity = router_settings_serialize->bus_velocity();
		router_settings_.bus_wait_time = router_settings_serialize->bus_wait_time();
	}
}