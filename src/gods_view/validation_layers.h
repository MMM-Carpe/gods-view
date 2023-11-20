#if !defined PG_GODS_VIEW_VALIDATION_LAYERS_HEADER_INCLUDED
#define PG_GODS_VIEW_VALIDATION_LAYERS_HEADER_INCLUDED
#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>

namespace pg::gods_view {

namespace details {

#if defined NDEBUG
	constexpr bool enable_validation_layers = false;
#else
	constexpr bool enable_validation_layers = true;
#endif

const std::vector<const char*> validation_layers = {
	"VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> device_extensions = {
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

static VkResult create_debug_utils_messenger_ext(
	VkInstance instance,
	const VkDebugUtilsMessengerCreateInfoEXT* create_info,
	const VkAllocationCallbacks* allocator,
	VkDebugUtilsMessengerEXT* debug_messenger
)
{
	auto func = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
		vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT")
	);
	if (func == nullptr) {
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
	return func(instance, create_info, allocator, debug_messenger);
}

static void destory_debug_utils_messenger_ext(
	VkInstance instance,
	VkDebugUtilsMessengerEXT debug_messenger,
	const VkAllocationCallbacks* allocator
)
{
	auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
		vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT")
	);
	if (func != nullptr) {
		func(instance, debug_messenger, allocator);
	}
}

} // end namespace pg::gods_view::details

class validation_layer_manager {
public:
	validation_layer_manager() = default;

	bool check_validation_layer_support() const noexcept {
		if (details::enable_validation_layers && !do_check_validation_layer_support()) {
			return false;
		}
		return true;
	}

	[[nodiscard]] uint32_t validation_layer_size() const noexcept { return static_cast<uint32_t>(details::validation_layers.size()); }

	[[nodiscard]] const char* const* validation_layer_names() const noexcept { return details::validation_layers.data(); }

private:
	bool do_check_validation_layer_support() const noexcept {
		uint32_t layer_count{};
		vkEnumerateInstanceLayerProperties(&layer_count, nullptr);
		
		std::vector<VkLayerProperties> available_layers{layer_count};
		vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

		for (const auto layer_name : details::validation_layers) {
			bool layer_found{false};
			for (const auto& layer_properties : available_layers) {
				if (std::strcmp(layer_name, layer_properties.layerName) == 0) {
					layer_found = true;
					break;
				}
			}
			if (!layer_found) { return false; }
		}
		return true;
	}

};

class debug_messenger {
private:
	VkDebugUtilsMessengerEXT debug_messenger_handle_;
	VkInstance vk_instance_;

public:
	debug_messenger(VkInstance vk_instance) :
		debug_messenger_handle_{nullptr},
		vk_instance_{vk_instance}
	{ 
		initiate_debug_messenger();
	}

	~debug_messenger() {
		destroy_debug_messenger();
	}

	static void populate_debug_messenger_info(VkDebugUtilsMessengerCreateInfoEXT& info) {
		info = {};
		info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
							   VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
							   VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
						   VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
						   VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		info.pfnUserCallback = debug_cb;
	}

private:
	void initiate_debug_messenger() {
		if (!details::enable_validation_layers) { return; }
		VkDebugUtilsMessengerCreateInfoEXT info{};
		populate_debug_messenger_info(info);
		if (details::create_debug_utils_messenger_ext(vk_instance_, &info, nullptr, &debug_messenger_handle_) != VK_SUCCESS) {
			throw std::runtime_error{"Failed to setup debug messegner"};
		}
	}

	void destroy_debug_messenger() {
		if (!details::enable_validation_layers || vk_instance_ == nullptr) { return; }
		details::destory_debug_utils_messenger_ext(vk_instance_, debug_messenger_handle_, nullptr);
	}

	static VKAPI_ATTR VkBool32 VKAPI_CALL debug_cb(
		VkDebugUtilsMessageSeverityFlagBitsEXT message_sevirity,
		VkDebugUtilsMessageTypeFlagsEXT message_type,
		const VkDebugUtilsMessengerCallbackDataEXT* cb_data,
		void* user_data
	)
	{
		std::cerr << "validation layer: " << cb_data->pMessage << std::endl;
		return VK_FALSE;
	}

};

} // end namespace pg::gods_view

#endif