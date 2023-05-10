#include <iostream>

#include "json_reader.h"
#include "request_handler.h"

int main() {
	catalogue::TransportCatalogue tc;
	renderer::MapRenderer map_renderer;
	RequestHandler req_handle(tc, map_renderer);
	json_reader::Reader reader(tc, req_handle);
	reader.LoadFromJson(std::cin);
	reader.ProcessStatRequests(std::cout);
}