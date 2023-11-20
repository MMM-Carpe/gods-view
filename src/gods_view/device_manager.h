#if !defined PG_GODS_VIEW_DEVICE_MANAGER_HEADER_INCLUDED
#define PG_GODS_VIEW_DEVICE_MANAGER_HEADER_INCLUDED
#pragma once

#include "gods_view/validation_layers.h"

#include <vulkan/vulkan.h>

#include <system_error>
#include <set>
#include <vector>
#include <optional>

namespace pg::gods_view {

struct queue_family_indices {
	std::optional<uint32_t> graphics_family;
	std::optional<uint32_t> present_family;

	bool is_complete() {
		return graphics_family.has_value() && present_family.has_value();
	}
};

namespace details {

static queue_family_indices find_queue_families(VkPhysicalDevice physical_device, VkSurfaceKHR surface) {
	queue_family_indices indices{};
	uint32_t queue_family_count{0};
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, nullptr);

	std::vector<VkQueueFamilyProperties> queue_families{queue_family_count};
	vkGetPhysicalDeviceQueueFamilyProperties(physical_device, &queue_family_count, queue_families.data());
	int i{0};
	for (const auto& queue_family : queue_families) {
		if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
			indices.graphics_family = i;
		}
		VkBool32 present_support = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(physical_device, i, surface, &present_support);
		if (present_support) {
			indices.present_family = i;
		}
		if (indices.is_complete()) { break; }
		++i;
	}
	return indices;
}

} // end namespace pg::gods_view::details

class vulkan_engine;

class device_manager {
private:
	gods_view::vulkan_engine* engine_;
	uint32_t device_count_;
	VkPhysicalDevice physical_device_;
	VkDevice device_;
	VkQueue graphics_queue_;
	VkQueue present_queue_;

public:
	device_manager(gods_view::vulkan_engine* init_engine) :
		engine_{init_engine},
		device_count_{0},
		physical_device_{nullptr},
		device_{nullptr},
		graphics_queue_{nullptr}
	{ }

	~device_manager() {
		destroy_devices();
	}

	[[nodiscard]] VkPhysicalDevice physical_device() const noexcept { return physical_device_; }

	[[nodiscard]] VkDevice logical_device() const noexcept { return device_; }

	[[nodiscard]] const VkQueue graphics_queue() const noexcept { return graphics_queue_; }

	[[nodiscard]] const VkQueue present_queue() const noexcept { return present_queue_; }

	void grab_physical_device();

	void create_logical_device();

private:
	bool is_device_suitable(VkPhysicalDevice device);

	bool check_device_extensions(VkPhysicalDevice device);

	void destroy_devices();
};
	
} // end namespace pg::gods_view

#endif