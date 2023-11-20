#include "gods_view/surface_manager.h"
#include "gods_view/vulkan_engine.h"

#include <algorithm>
#include <limits>

namespace pg::gods_view {

surface_manager::surface_manager(vulkan_engine* init_engine) :
	engine_{init_engine},
	surface_{nullptr}
{ }

surface_manager::~surface_manager() {
	destroy_image_views();
	destroy_swap_chain();
	destroy_surface();
}

void surface_manager::create_vulkan_surface(GLFWwindow* window) {
	if (glfwCreateWindowSurface(engine_->vulkan_instance()->vk_instance(), window, nullptr, &surface_) != VK_SUCCESS) {
		throw std::runtime_error{"Failed to create window surface"};
	}
}

void surface_manager::create_swap_chain() {
	swap_chain_support_details swap_chain_support = details::query_swap_chain_support(engine_->device_manager()->physical_device(), surface_);

	VkSurfaceFormatKHR surface_format = choose_swap_surface_format(swap_chain_support.formats);
	VkPresentModeKHR present_mode = choose_swap_present_mode(swap_chain_support.present_modes);
	VkExtent2D extent = choose_swap_extent(swap_chain_support.capabilities);

	uint32_t image_count = swap_chain_support.capabilities.minImageCount + 1;
	if (swap_chain_support.capabilities.maxImageCount > 0 && image_count > swap_chain_support.capabilities.maxImageCount) {
		image_count = swap_chain_support.capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	create_info.surface = surface_;
	create_info.minImageCount = image_count;
	create_info.imageFormat = surface_format.format;
	create_info.imageColorSpace = surface_format.colorSpace;
	create_info.imageExtent = extent;
	create_info.imageArrayLayers = 1;
	create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	queue_family_indices indices = details::find_queue_families(engine_->device_manager()->physical_device(), surface_);
	uint32_t queue_family_indices_val[] = {indices.graphics_family.value(), indices.present_family.value()};
	if (indices.graphics_family != indices.present_family) {
		create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		create_info.queueFamilyIndexCount = 2;
		create_info.pQueueFamilyIndices = queue_family_indices_val;
	} else {
		create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
	}
	create_info.preTransform = swap_chain_support.capabilities.currentTransform;
	create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	create_info.presentMode = present_mode;
	create_info.clipped= VK_TRUE;
	create_info.oldSwapchain = VK_NULL_HANDLE;

	if (vkCreateSwapchainKHR(engine_->device_manager()->logical_device(), &create_info, nullptr, &swap_chain_) != VK_SUCCESS) {
		throw std::runtime_error{"Failed to create swap chain"};
	}
	vkGetSwapchainImagesKHR(engine_->device_manager()->logical_device(), swap_chain_, &image_count, nullptr);
	swap_chain_images_.resize(image_count);
	vkGetSwapchainImagesKHR(engine_->device_manager()->logical_device(), swap_chain_, &image_count, swap_chain_images_.data());
	swap_chain_image_format_ = surface_format.format;
	swap_chain_extent_ = extent;
}

void surface_manager::create_image_views() {
	swap_chain_image_views_.resize(swap_chain_images_.size());

	for (size_t i = 0; i < swap_chain_images_.size(); ++i) {
		VkImageViewCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		create_info.image = swap_chain_images_[i];
		create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
		create_info.format = swap_chain_image_format_;
		create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		create_info.subresourceRange.baseMipLevel = 0;
		create_info.subresourceRange.levelCount = 1;
		create_info.subresourceRange.baseArrayLayer = 0;
		create_info.subresourceRange.layerCount = 1;
		if (vkCreateImageView(engine_->device_manager()->logical_device(), &create_info, nullptr, &swap_chain_image_views_[i]) != VK_SUCCESS) {
			throw std::runtime_error{"Failed to create image views"};
		}
	}
}

VkExtent2D surface_manager::choose_swap_extent(const VkSurfaceCapabilitiesKHR& capabilities) {
	if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
		return capabilities.currentExtent;
	} else {
		int width, height;
		glfwGetFramebufferSize(engine_->current_window(), &width, &height);
		VkExtent2D actual_extent = {
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};
		actual_extent.width = std::clamp(
			actual_extent.width,
			capabilities.minImageExtent.width,
			capabilities.maxImageExtent.width
		);
		actual_extent.height = std::clamp(
			actual_extent.height,
			capabilities.minImageExtent.height,
			capabilities.maxImageExtent.height
		);
		return actual_extent;
	}
}

void surface_manager::destroy_surface() {
	vkDestroySurfaceKHR(engine_->vulkan_instance()->vk_instance(), surface_, nullptr);
}

void surface_manager::destroy_swap_chain() {
	vkDestroySwapchainKHR(engine_->device_manager()->logical_device(), swap_chain_, nullptr);
}

void surface_manager::destroy_image_views() {
	for (auto image_view : swap_chain_image_views_) {
		vkDestroyImageView(engine_->device_manager()->logical_device(), image_view, nullptr);
	}
}

} // end namespace pg::gods_view