#include <iostream>

#include "json_reader.h"
#include "request_handler.h"


using namespace std::literals;

void PrintUsage(std::ostream& stream = std::cerr) {
    stream << "Usage: transport_catalogue [make_base|process_requests]\n"sv;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        PrintUsage();
        return 1;
    }

    const std::string_view mode(argv[1]);

    catalogue::TransportCatalogue tc;
	renderer::MapRenderer map_renderer;
	RequestHandler req_handle(tc, map_renderer);
    json_reader::Reader reader(tc, req_handle);

    if (mode == "make_base"sv) {
        // make base here
        reader.MakeBaseCommand(std::cin);
    }
    else if (mode == "process_requests"sv) {
        // process requests here
        reader.ProcessRequestsCommand(std::cin, std::cout);
    }
    else {
        PrintUsage();
        return 1;
    }
}