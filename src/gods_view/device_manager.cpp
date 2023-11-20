#include "gods_view/device_manager.h"	
#include "gods_view/vulkan_engine.h"

namespace pg::gods_view {

void device_manager::grab_physical_device() {
	vkEnumeratePhysicalDevices(engine_->vulkan_instance()->vk_instance(), &device_count_, nullptr);
	if (device_count_ == 0) {
		throw std::runtime_error{"Failed to find GPU's with Vulkan support"};
	}
	std::vector<VkPhysicalDevice> devices{device_count_};
	vkEnumeratePhysicalDevices(engine_->vulkan_instance()->vk_instance(), &device_count_, devices.data());
	for (const auto& device : devices) {
		if (is_device_suitable(device)) {
			physical_device_ = device;
			break;
		}
	}
	if (physical_device_ == nullptr) {
		throw std::runtime_error{"Failed to find a suitable GPU"};
	}
}

void device_manager::create_logical_device() {
	queue_family_indices indices{details::find_queue_families(physical_device_, engine_->surface_manager()->surface())};

	std::vector<VkDeviceQueueCreateInfo> queue_create_infos{};
	std::set<uint32_t> unique_queue_families = {
		indices.graphics_family.value(),
		indices.present_family.value()
	};

	float queue_priority{1.0f};
	for (auto queue_family : unique_queue_families) {
		VkDeviceQueueCreateInfo queue_create_info{};
		queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info.queueFamilyIndex = queue_family;
		queue_create_info.queueCount = 1;
		queue_create_info.pQueuePriorities = &queue_priority;
		queue_create_infos.push_back(queue_create_info);
	}

	VkPhysicalDeviceFeatures device_features{};
	VkDeviceCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
	create_info.pQueueCreateInfos = queue_create_infos.data();
	create_info.pEnabledFeatures = &device_features;

	create_info.enabledExtensionCount = static_cast<uint32_t>(details::device_extensions.size());
	create_info.ppEnabledExtensionNames = details::device_extensions.data();
	if (details::enable_validation_layers) {
		auto validation_manager = engine_->validation_layer_manager();
		create_info.enabledLayerCount = validation_manager->validation_layer_size();
		create_info.ppEnabledLayerNames = validation_manager->validation_layer_names();
	} else {
		create_info.enabledLayerCount = 0;
	}
	if (vkCreateDevice(physical_device_, &create_info, nullptr, &device_) != VK_SUCCESS) {
		throw std::runtime_error{"Failed to create logical device."};
	}
	vkGetDeviceQueue(device_, indices.graphics_family.value(), 0, &graphics_queue_);
	vkGetDeviceQueue(device_, indices.present_family.value(), 0, &present_queue_);
}


bool device_manager::is_device_suitable(VkPhysicalDevice device) {
	queue_family_indices indices = details::find_queue_families(device, engine_->surface_manager()->surface());

	bool extensions_supported = check_device_extensions(device);
	bool swap_chain_adequate{false};
	if (extensions_supported) {
		swap_chain_support_details swap_chain_support = details::query_swap_chain_support(device, engine_->surface_manager()->surface());
		swap_chain_adequate = !swap_chain_support.formats.empty() && !swap_chain_support.present_modes.empty();
	}
	return indices.is_complete();
}

bool device_manager::check_device_extensions(VkPhysicalDevice device) {
	uint32_t extension_count;
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
	std::vector<VkExtensionProperties> available_extensions{extension_count};
	vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, available_extensions.data());

	std::set<std::string> required_extensions{details::device_extensions.begin(), details::device_extensions.end()};
	for (const auto& extension : available_extensions) {
		required_extensions.erase(extension.extensionName);
	}
	return required_extensions.empty();
}

void device_manager::destroy_devices() {
	if (device_ != nullptr) {
		vkDestroyDevice(device_, nullptr);
	}
}

} // end namespace pg::gods_view