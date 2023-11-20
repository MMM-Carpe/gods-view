#if !defined PG_GODS_VIEW_SURFACE_MANAGER_HEADER_INCLUDED
#define PG_GODS_VIEW_SURFACE_MANAGER_HEADER_INCLUDED
#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <vector>

namespace pg::gods_view {

struct swap_chain_support_details {
	VkSurfaceCapabilitiesKHR capabilities;
	std::vector<VkSurfaceFormatKHR> formats;
	std::vector<VkPresentModeKHR> present_modes;
};

namespace details {

static swap_chain_support_details query_swap_chain_support(VkPhysicalDevice device, VkSurfaceKHR surface) {
	swap_chain_support_details details{};
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
	uint32_t format_count;
	vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, nullptr);
	if (format_count != 0) {
		details.formats.resize(format_count);
		vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &format_count, details.formats.data());
	}
	uint32_t preset_mode_count;
	vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &preset_mode_count, nullptr);
	if (preset_mode_count != 0) {
		details.present_modes.resize(preset_mode_count);
		vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &preset_mode_count, details.present_modes.data());
	}
	return details;
}

} // end namespace pg::gods_view::details

class vulkan_engine;

class surface_manager {
private:
	gods_view::vulkan_engine* engine_;
	VkSurfaceKHR surface_;
	VkSwapchainKHR swap_chain_;
	std::vector<VkImage> swap_chain_images_;
	VkFormat swap_chain_image_format_;
	VkExtent2D swap_chain_extent_;
	std::vector<VkImageView> swap_chain_image_views_;
	
public:
	surface_manager(gods_view::vulkan_engine* init_engine);

	~surface_manager();

	[[nodiscard]] VkSurfaceKHR surface() const noexcept { return surface_; }

	[[nodiscard]] VkFormat swap_chain_image_format() const noexcept { return swap_chain_image_format_; }

	[[nodiscard]] const std::vector<VkImageView>& swap_chain_image_views() const noexcept { return swap_chain_image_views_; }

	[[nodiscard]] const VkExtent2D swap_chain_extent() const noexcept { return swap_chain_extent_; }

	[[nodiscard]] const VkSwapchainKHR swapchain() const noexcept { return swap_chain_; }

	void create_vulkan_surface(GLFWwindow* window);

	void create_swap_chain();

	void create_image_views();

private:
	VkSurfaceFormatKHR choose_swap_surface_format(const std::vector<VkSurfaceFormatKHR>& available_formats) {
		for (const auto& available_format : available_formats) {
			if (available_format.format == VK_FORMAT_B8G8R8A8_SRGB && available_format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
				return available_format;
			}
		}
		return available_formats[0];
	}

	VkPresentModeKHR choose_swap_present_mode(const std::vector<VkPresentModeKHR>& available_present_modes) {
		for (const auto& available_present_mode : available_present_modes) {
			if (available_present_mode == VK_PRESENT_MODE_MAILBOX_KHR) {
				return available_present_mode;
			}
		}
		return VK_PRESENT_MODE_FIFO_KHR;
	}

	VkExtent2D choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities);

	void destroy_surface();

	void destroy_swap_chain();

	void destroy_image_views();
};

} // end namespace pg::gods_view

#endif