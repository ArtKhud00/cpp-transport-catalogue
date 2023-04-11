#pragma once
#include "transport_catalogue.h"

namespace outtxt {
	using RequestsByTypeAndContent = std::vector<std::pair<std::string, std::string>>;

	void GetInformationAboutBus(std::string& request_content, catalogue::TransportCatalogue& tc);
	void GetInformationAboutStop(std::string& request_content, catalogue::TransportCatalogue& tc);
	void ProcessRequest(RequestsByTypeAndContent& requests, catalogue::TransportCatalogue& tc);
	void ReadRequest(std::istream& input, catalogue::TransportCatalogue& tc);
}