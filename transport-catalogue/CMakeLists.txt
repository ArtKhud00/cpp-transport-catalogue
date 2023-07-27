cmake_minimum_required(VERSION 3.10)

project(TransportCatalogue CXX)
set(CMAKE_CXX_STANDARD 17)

# Эта команда найдёт собранный нами пакет Protobuf.
# REQUIRED означает, что библиотека обязательна.
# Путь для поиска укажем в параметрах команды cmake.
find_package(Protobuf REQUIRED)
# Помимо Protobuf, понадобится библиотека Threads
find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto
						map_renderer.proto svg.proto
						graph.proto transport_router.proto)

if (CMAKE_SYSTEM_NAME MATCHES "^MINGW")
    set(SYSTEM_LIBS -lstdc++)
else()
    set(SYSTEM_LIBS)
endif()

set(JSON_FILES json.cpp json.h 
	json_builder.cpp json_builder.h 
	json_reader.cpp json_reader.h)
set(TRANSPORT_CATALOGUE_FILES domain.cpp domain.h 
							  geo.cpp geo.h 
							  transport_catalogue.cpp transport_catalogue.h
							  request_handler.cpp request_handler.h
							  main.cpp)
set(MAP_RENDER_FILES map_renderer.cpp map_renderer.h
					 svg.cpp svg.h)
set(TRANSPORT_ROUTER_FILES graph.h ranges.h router.h 
						   transport_router.h transport_router.cpp)
set(SERIALIZATION_FILES serialization.cpp serialization.h)

add_executable(transport_catalogue ${PROTO_SRCS} ${PROTO_HDRS} 
								   ${JSON_FILES} ${TRANSPORT_CATALOGUE_FILES} 
								   ${MAP_RENDER_FILES} ${TRANSPORT_ROUTER_FILES} 
								   ${SERIALIZATION_FILES} )
target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})


string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads ${SYSTEM_LIBS})					 