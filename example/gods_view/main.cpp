#include "application.h"

#include <chrono>
#include <iostream>
#include <thread>

using namespace pg;

int main(void) {
	try {
		example::application app{"gods_view", 1280, 720};
		app.run();

		uint32_t extension_count{0};
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
		std::cout << "Extensions: " << extension_count << std::endl;
	} catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
	} catch (...) {
		std::cerr << "Uknown Exception: Terminating" << std::endl;
	}
	
	return 0;
}