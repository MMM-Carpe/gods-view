#if !defined PG_GODS_VIEW_VULKAN_INSTANCE_HEADER_INCLUDED
#define PG_GODS_VIEW_VULKAN_INSTANCE_HEADER_INCLUDED
#pragma once

#include "gods_view/validation_layers.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <string>
#include <string_view>

namespace pg::gods_view {

// raii'd up vulkan instance...
class vulkan_instance {
private:
	VkInstance vk_instance_;
	const validation_layer_manager& validation_layer_manager_;
	std::string app_name_;
	std::string engine_name_;

public:	
	vulkan_instance(
		const std::string& init_app_name,
		const std::string& init_engine_name,
		const validation_layer_manager& init_validation_layer_manager
	) :
		vk_instance_{nullptr},
		validation_layer_manager_{init_validation_layer_manager},
		app_name_{init_app_name},
		engine_name_{init_engine_name}
	{
		create_vulkan_instance();
	}

	~vulkan_instance() {
		destroy_resources();
	}

	[[nodiscard]] VkInstance vk_instance() const noexcept { return vk_instance_; }

	[[nodiscard]] std::string_view app_name() const noexcept { return app_name_; }

	[[nodiscard]] std::string_view engine_name() const noexcept { return engine_name_; }

private:
	void create_vulkan_instance() {
		if (!validation_layer_manager_.check_validation_layer_support()) {
			throw std::runtime_error{"Validation layers requested, but not available."};
		}

		VkApplicationInfo app_info{};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pApplicationName = app_name_.c_str();
		app_info.applicationVersion = VK_MAKE_VERSION(1, 2, 0);
		app_info.pEngineName = engine_name_.c_str();
		app_info.engineVersion = VK_MAKE_VERSION(1, 2, 0);
		app_info.apiVersion = VK_API_VERSION_1_2;

		VkInstanceCreateInfo creation_info{};
		creation_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		creation_info.pApplicationInfo = &app_info;

		auto extensions = required_extensions();
		creation_info.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
		creation_info.ppEnabledExtensionNames = extensions.data();

		VkDebugUtilsMessengerCreateInfoEXT debug_creation_info{};
		if (details::enable_validation_layers) {
			creation_info.enabledLayerCount = validation_layer_manager_.validation_layer_size();
			creation_info.ppEnabledLayerNames = validation_layer_manager_.validation_layer_names();
			debug_messenger::populate_debug_messenger_info(debug_creation_info);
			creation_info.pNext = reinterpret_cast<VkDebugUtilsMessengerCreateInfoEXT*>(&debug_creation_info);
		} else {
			creation_info.enabledLayerCount = 0;
			creation_info.pNext = nullptr;
		}
		if (vkCreateInstance(&creation_info, nullptr, &vk_instance_) != VK_SUCCESS) {
			throw std::runtime_error{"Failed to create intance"};
		}
	}

	// might need to move this into window....
	std::vector<const char*> required_extensions() {
		glfwInit();
		uint32_t glfw_extension_count{0};
		const char** glfw_extensions{nullptr};
		glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);
		
		std::vector<const char*> extensions{glfw_extensions, glfw_extensions + glfw_extension_count};
		if (details::enable_validation_layers) {
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
		}
		return extensions;
	}

	void destroy_resources() {
		vkDestroyInstance(vk_instance_, nullptr);
	}
};

} // end namespace pg::gods_view

#endif