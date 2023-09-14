# Транспортный справочник
Программа поддерживает графический вывод, поиск маршрутов и вычисление времени поездки.
Программа разделена на 2 части - одна отвечает за наполнение транспортного справочника, вторая за обработку запросов к нему:
  - Команда make_base: создание базы транспортного справочника по запросам `base_requests` и её сериализация в файл.
  - Программа process_requests: десериализация базы из файла и использование её для ответов на запросы `stat_requests`.
Таким образом в программе была реализована двухстадийность.
Для сериализации используется protobuf.
## Формат ввода базы данных
### make_base
В качестве аргумента командной строки задается команда make_base.
На вход программе make_base через стандартный поток ввода подаётся JSON со следующими ключами: 
  1. `base_requests`: запросы Bus и Stop на создание базы.
  2. `routing_settings`: настройки маршрутизации. 
  3. `render_settings`: настройки отрисовки.
  4. `serialization_settings`: настройки сериализации. 
### process_requests
В качестве аргумента командной строки задается команда process_requests.
На вход программе process_requests подаётся файл с сериализованной базой (результат работы make_base), а также — через стандартный поток ввода — JSON со следующими ключами:
  1. `stat_requests`: запросы Bus, Stop, Map, Route к готовой базе.
  2. `serialization_settings`: настройки сериализации в формате, аналогичном этой же секции на входе make_base.
Программа process_requests должна вывести JSON с ответами на запросы
### Содержимое ключей
#### Описание базы маршрутов
Массив `base_requests` содержит элементы двух типов: маршруты и остановки. Они перечисляются в произвольном порядке.
Пример описания остановки:
```json
{
  "type": "Stop",
  "name": "Электросети",
  "latitude": 43.598701,
  "longitude": 39.730623,
  "road_distances": {
    "Улица Докучаева": 3000,
    "Улица Лизы Чайкиной": 4300
  }
}
```
Описание остановки — словарь с ключами:
  - `type` — строка, равная `"Stop"`. Означает, что словарь описывает остановку;
  - `name` — название остановки;
  - `latitude` и `longitude` — широта и долгота остановки — числа с плавающей запятой;
  - `road_distances` — словарь, задающий дорожное расстояние от этой остановки до соседних. Каждый ключ в этом словаре — название соседней остановки, значение — целочисленное расстояние в метрах.

Пример описания автобусного маршрута:
```json
{
  "type": "Bus",
  "name": "14",
  "stops": [
    "Улица Лизы Чайкиной",
    "Электросети",
    "Улица Докучаева",
    "Улица Лизы Чайкиной"
  ],
  "is_roundtrip": true
}
```
Описание автобусного маршрута — словарь с ключами:
  - `type` — строка `"Bus"`. Означает, что словарь описывает автобусный маршрут;
  - `name` — название маршрута;
  - `stops` — массив с названиями остановок, через которые проходит маршрут. У кольцевого маршрута название последней остановки дублирует название первой. Например: `["stop1", "stop2", "stop3", "stop1"]`;
  - `is_roundtrip` — значение типа `bool`. `true`, если маршрут кольцевой.

#### Настройки отрисовки
Чтобы управлять визуализацией карты, во входном JSON-документе используется словарь `render_settings`.
Структура словаря `render_settings`:
```json
{
  "width": 1200.0,
  "height": 1200.0,

  "padding": 50.0,

  "line_width": 14.0,
  "stop_radius": 5.0,

  "bus_label_font_size": 20,
  "bus_label_offset": [7.0, 15.0],

  "stop_label_font_size": 20,
  "stop_label_offset": [7.0, -3.0],

  "underlayer_color": [255, 255, 255, 0.85],
  "underlayer_width": 3.0,

  "color_palette": [
    "green",
    [255, 160, 0],
    "red"
  ]
}
```
  - `width` и `height` — ширина и высота изображения в пикселях. Вещественное число в диапазоне от 0 до 100000.
  - `padding` — отступ краёв карты от границ SVG-документа. Вещественное число не меньше 0 и меньше `min(width, height)/2`.
  - `line_width` — толщина линий, которыми рисуются автобусные маршруты. Вещественное число в диапазоне от 0 до 100000.
  - `stop_radius` — радиус окружностей, которыми обозначаются остановки. Вещественное число в диапазоне от 0 до 100000.
  - `bus_label_font_size` — размер текста, которым написаны названия автобусных маршрутов. Целое число в диапазоне от 0 до 100000.
  - `bus_label_offset` — смещение надписи с названием маршрута относительно координат конечной остановки на карте. Массив из двух элементов типа `double`. Задаёт значения свойств `dx` и `dy` SVG-элемента `<text>`. Элементы массива — числа в диапазоне от –100000 до 100000.
  - `stop_label_font_size` — размер текста, которым отображаются названия остановок. Целое число в диапазоне от 0 до 100000.
  - `stop_label_offset` — смещение названия остановки относительно её координат на карте. Массив из двух элементов типа `double`. Задаёт значения свойств `dx` и `dy` SVG-элемента `<text>`. Числа в диапазоне от –100000 до 100000.
  - `underlayer_color` — цвет подложки под названиями остановок и маршрутов. Формат хранения цвета будет ниже.
  - `underlayer_width` — толщина подложки под названиями остановок и маршрутов. Задаёт значение атрибута `stroke-width` элемента `<text>`. Вещественное число в диапазоне от 0 до 100000.
  - `color_palette` — цветовая палитра. Непустой массив.

Цвет можно указать в одном из следующих форматов:
  - в виде строки, например, `"red"` или `"black"`;
  - в массиве из трёх целых чисел диапазона [0, 255]. Они определяют `r`, `g` и `b` компоненты цвета в формате `svg::Rgb`. Цвет `[255, 16, 12]` нужно вывести в SVG как `rgb(255,16,12)`;
  - в массиве из четырёх элементов: три целых числа в диапазоне от [0, 255] и одно вещественное число в диапазоне от [0.0, 1.0]. Они задают составляющие `red`, `green`, `blue` и `opacity` цвета формата `svg::Rgba`. Цвет, заданный как `[255, 200, 23, 0.85]`, должен быть выведен в SVG как `rgba(255,200,23,0.85)`.
Гарантируется, что каждый цвет задан в одном из этих трёх форматов.

#### Настройки маршрутизации
Ключ routing_settings, значение которого — словарь с двумя ключами:
  - `bus_wait_time` — время ожидания автобуса на остановке, в минутах. Считайте, что когда бы человек ни пришёл на остановку и какой бы ни была эта остановка, он будет ждать любой автобус в точности указанное количество минут. Значение — целое число от 1 до 1000.
  - `bus_velocity` — скорость автобуса, в км/ч. Считайте, что скорость любого автобуса постоянна и в точности равна указанному числу. Время стоянки на остановках не учитывается, время разгона и торможения тоже. Значение — вещественное число от 1 до 1000.
Пример:
```json
"routing_settings": {
      "bus_wait_time": 6,
      "bus_velocity": 40
}
```
#### Настройки сериализации
Словарь с единственным ключом `file`, которому соответствует строка — название файла. Именно в этот файл нужно сохранить сериализованную базу или считать сериализованную базу.
Задача программы make_base — построить базу и сериализовать её в файл с указанным именем.
Пример:
```json
"serialization_settings": {
      "file": "transport_catalogue.db"
}
```
### Запросы к базе и ответы на них
Запросы хранятся в массиве `stat_requests`. Ответ программа выводит в виде JSON массива:
```json
[
  { ответ на первый запрос },
  { ответ на второй запрос },
  ...
  { ответ на последний запрос }
]
```
Каждый запрос — словарь с обязательными ключами `id` и `type`. Они задают уникальный числовой идентификатор запроса и его тип. В словаре могут быть и другие ключи, специфичные для конкретного типа запроса.

#### Запрос информации о маршруте
Формат запроса:
```json
{
  "id": 12345678,
  "type": "Bus",
  "name": "14"
}
```
Ключ `type` имеет значение “Bus”. По нему можно определить, что это запрос на получение информации о маршруте.
Ключ `name` задаёт название маршрута, для которого приложение должно вывести статистическую информацию.
Ответ на этот запрос в виде словаря:
```json
{
  "curvature": 2.18604,
  "request_id": 12345678,
  "route_length": 9300,
  "stop_count": 4,
  "unique_stop_count": 3
}
```
Ключи словаря:
  - `curvature` — извилистость маршрута. Она равна отношению длины дорожного расстояния маршрута к длине географического расстояния. Число типа double;
  - `request_id` — должен быть равен id соответствующего запроса Bus. Целое число;
  - `route_length` — длина дорожного расстояния маршрута в метрах, целое число;
  - `stop_count` — количество остановок на маршруте;
  - `unique_stop_count` — количество уникальных остановок на маршруте.

Если в справочнике нет маршрута с указанным названием, ответ выглядит так:
```json
{
  "request_id": 12345678,
  "error_message": "not found"
}
```

#### Запрос информации об остановке
Формат запроса:
```json
{
  "id": 12345,
  "type": "Stop",
  "name": "Улица Докучаева"
}
```
Ключ `name` задаёт название остановки.
Ответ на запрос:
```json
{
  "buses": [
      "14", "22к"
  ],
  "request_id": 12345
}
```
Значение ключей ответа:
  - `buses` — массив названий маршрутов, которые проходят через эту остановку. Названия отсортированы в лексикографическом порядке.
  - `request_id` — целое число, равное id соответствующего запроса Stop.
Если в справочнике нет остановки с переданным названием, ответ на запрос выглядит так:
```json
{
  "request_id": 12345,
  "error_message": "not found"
}
```

#### Запрос на получение изображения
Формат запроса:
```json
{
  "type": "Map",
  "id": 11111
}
```
Ответ на этот запрос отдаётся в виде словаря с ключами `request_id` и `map`:
```json
{
  "map": "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n  <polyline points=\"100.817,170 30,30 100.817,170\" fill=\"none\" stroke=\"green\" stroke-width=\"14\" stroke-linecap=\"round\" stroke-linejoin=\"round\"/>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"100.817\" y=\"170\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"green\" x=\"100.817\" y=\"170\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"30\" y=\"30\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <text fill=\"green\" x=\"30\" y=\"30\" dx=\"7\" dy=\"15\" font-size=\"20\" font-family=\"Verdana\" font-weight=\"bold\">114</text>\n  <circle cx=\"100.817\" cy=\"170\" r=\"5\" fill=\"white\"/>\n  <circle cx=\"30\" cy=\"30\" r=\"5\" fill=\"white\"/>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"100.817\" y=\"170\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Морской вокзал</text>\n  <text fill=\"black\" x=\"100.817\" y=\"170\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Морской вокзал</text>\n  <text fill=\"rgba(255,255,255,0.85)\" stroke=\"rgba(255,255,255,0.85)\" stroke-width=\"3\" stroke-linecap=\"round\" stroke-linejoin=\"round\" x=\"30\" y=\"30\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Ривьерский мост</text>\n  <text fill=\"black\" x=\"30\" y=\"30\" dx=\"7\" dy=\"-3\" font-size=\"20\" font-family=\"Verdana\">Ривьерский мост</text>\n</svg>",
  "request_id": 11111
}
```
Ключ `map` — строка с изображением карты в формате SVG.
#### Запрос на построение маршрута
Элементы с `"type": "Route"` — это запросы на построение маршрута между двумя остановками. Помимо стандартных свойств `id` и `type`, они содержат ещё два:
  - `from` — остановка, где нужно начать маршрут.
  - `to` — остановка, где нужно закончить маршрут.
Оба значения — названия существующих в базе остановок. Однако они, возможно, не принадлежат ни одному автобусному маршруту.
Пример:
```json
{
      "type": "Route",
      "from": "Biryulyovo Zapadnoye",
      "to": "Universam",
      "id": 4
}
```

## Сборка
Для проекта используется система сборки CMake. Рекомендуется осуществлять сборку проекта в отдельной директории, чтобы не испортить директорию с исходным кодом, так в процессе сборки создаются вспомогательные файлы CMake, промежуточные файлы компиляции.
Возможная структура каталога для сборки:
```
cpp-transport-catalogue/
├── transport-catalogue/ # папка с исходным кодом
│   ├── CMakeLists.txt
│   ├── domain.cpp
│   ├── domain.h
│   ├── geo.cpp
│   ├── geo.h
│   ├── graph.h
│   ├── graph.proto
│   ├── input_reader.cpp
│   ├── input_reader.h
│   ├── json_builder.cpp
│   ├── json_builder.h
│   ├── json.cpp
│   ├── json.h
│   ├── json_reader.cpp
│   ├── json_reader.h
│   ├── main.cpp
│   ├── map_renderer.cpp
│   ├── map_renderer.h
│   ├── map_renderer.proto
│   ├── ranges.h
│   ├── request_handler.cpp
│   ├── request_handler.h
│   ├── router.h
│   ├── serialization.cpp
│   ├── serialization.h
│   ├── stat_reader.cpp
│   ├── stat_reader.h
│   ├── svg.cpp
│   ├── svg.h
│   ├── svg.proto
│   ├── transport_catalogue.cpp
│   ├── transport_catalogue.h
│   ├── transport_catalogue.proto
│   ├── transport_router.cpp
│   ├── transport_router.h
│   └── transport_router.proto
└── transport-catalogue-build/ # папка для сборки
    └── # тут будут файлы сборки
```
При сборке в консоли необходимо также указать путь к собранному пакету Protobuf. Для этого нужно в переменной `CMAKE_PREFIX_PATH` указать путь к пакету. Таким образом, команды для сборки будут выглядеть следующим образом:
```
cd transport-catalogue-build/
cmake ../transport-catalogue -DCMAKE_PREFIX_PATH=/path/to/protobuf/package -G "MinGW Makefiles"
cmake --build .
```
Флаг `-G "MinGW Makefiles"` используется под Windows, так как используется генератор `"MinGW Makefiles"`
## Запуск
Будет приведен пример запуска программы под Windows.
 - make_base:
```
./transport_catalogue.exe make_base <in1m.json
```
 - process_requests:
```
./transport_catalogue.exe process_requests <in1p.json >out1.json
```
