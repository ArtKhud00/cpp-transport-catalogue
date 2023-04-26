#include <iostream>

#include "json_reader.h"
#include "request_handler.h"

int main() {
	catalogue::TransportCatalogue tc;
	json_reader::Reader reader(tc);
	reader.LoadFromJson(std::cin);
	RequestHandler req_handle(tc, reader);
	req_handle.JsonOut(std::cout);
	//svg::Document res = req_handle.RenderMap();
	//res.Render(std::cout);
}