#include "include.h"

int main() {
	try {
		ft::Application firstApp{};
		firstApp.run();
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	}

	return 0;
}