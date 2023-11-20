#if !defined PG_GODS_VIEW_COMMAND_MANAGER_HEADER_INCLUDED
#define PG_GODS_VIEW_COMMAND_MANAGER_HEADER_INCLUDED
#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>

namespace pg::gods_view {

class vulkan_engine;

class command_manager {
private:
	gods_view::vulkan_engine* engine_;
	VkCommandPool command_pool_;
	VkCommandBuffer command_buffer_;

public:
	command_manager(gods_view::vulkan_engine* init_engine);

	~command_manager();

	[[nodiscard]] const VkCommandPool command_pool() const noexcept { return command_pool_; }
	
	[[nodiscard]] const VkCommandBuffer command_buffer() const noexcept { return command_buffer_; }

	void create_command_pool();
	
	void create_command_buffer();
	
	void record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index);
};

} // end namespace pg::gods_view

#endif