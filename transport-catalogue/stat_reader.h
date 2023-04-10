#pragma once
#include "transport_catalogue.h"

namespace outtxt {
	void readRequest(std::istream& input, catalogue::TransportCatalogue& tc);
}