#include <iostream>
#include "input_reader.h"
#include "stat_reader.h"


int main() {
	catalogue::TransportCatalogue tc;
	inputtxt::read_input(std::cin,tc);
	outtxt::readRequest(std::cin, tc);
}