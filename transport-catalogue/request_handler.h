#pragma once
#include "transport_catalogue.h"
#include <optional>
#include "json.h"
#include "json_reader.h"
#include "svg.h"

/*
 * Здесь можно было бы разместить код обработчика запросов к базе, содержащего логику, которую не
 * хотелось бы помещать ни в transport_catalogue, ни в json reader.
 *
 * В качестве источника для идей предлагаем взглянуть на нашу версию обработчика запросов.
 * Вы можете реализовать обработку запросов способом, который удобнее вам.
 *
 * Если вы затрудняетесь выбрать, что можно было бы поместить в этот файл,
 * можете оставить его пустым.
 */

 // Класс RequestHandler играет роль Фасада, упрощающего взаимодействие JSON reader-а
 // с другими подсистемами приложения.
 // См. паттерн проектирования Фасад: https://ru.wikipedia.org/wiki/Фасад_(шаблон_проектирования)
 
 class RequestHandler {
 public:
     // MapRenderer понадобится в следующей части итогового проекта
     //RequestHandler(const TransportCatalogue& db, const renderer::MapRenderer& renderer);
     RequestHandler(catalogue::TransportCatalogue& db, json_reader::Reader& reader) 
         : db_(db)
         , reader_(reader) {
     }

     // Возвращает информацию о маршруте (запрос Bus)
     //std::optional<data::BusInfo> GetBusStat(const std::string_view& bus_name) const;

     // Возвращает маршруты, проходящие через
     //const std::unordered_set<BusPtr>* GetBusesByStop(const std::string_view& stop_name) const;
     void JsonOut(std::ostream& out);

     // Этот метод будет нужен в следующей части итогового проекта
     svg::Document RenderMap() const;

 private:
     // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
     catalogue::TransportCatalogue& db_;
     json_reader::Reader& reader_;
     //const renderer::MapRenderer& renderer_;
 };
 