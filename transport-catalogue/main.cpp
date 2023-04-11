#include <iostream>
#include "input_reader.h"
#include "stat_reader.h"


int main() {
	catalogue::TransportCatalogue tc;
	inputtxt::ReadInput(std::cin,tc);
	outtxt::ReadRequest(std::cin, tc);
}