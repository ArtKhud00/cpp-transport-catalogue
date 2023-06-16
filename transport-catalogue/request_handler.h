#pragma once
#include "transport_catalogue.h"
#include "map_renderer.h"
#include "transport_router.h"
#include "svg.h"

#include <optional>
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
     RequestHandler(catalogue::TransportCatalogue& db, 
                    renderer::MapRenderer& map_renderer) 
         : db_(db)
         , map_renderer_(map_renderer) {
     }

     // Возвращает информацию о маршруте (запрос Bus)
     std::optional<const data::BusInfo> GetBusStat(const std::string_view& bus_name) const;

     // Возвращает информацию об остановке (запрос Stop)
     std::optional<const data::StopInfo> GetStopStat(const std::string_view& stop_name) const;

     //void JsonOut(std::ostream& out);

     void SetRenderSettings(const renderer::RouteMapSettings& map_settings);

     void SetRouterSettings(router::TransportRouter& tr,router::RouterSettings&);
     
     svg::Document RenderMap() const;

     std::optional<const router::RouteData> CalculateRoute(router::TransportRouter& tr, const std::string_view, const std::string_view);

 private:
     // RequestHandler использует агрегацию объектов "Транспортный Справочник" и "Визуализатор Карты"
     catalogue::TransportCatalogue& db_;
     renderer::MapRenderer& map_renderer_;
 };
 