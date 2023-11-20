#if !defined PG_GODS_VIEW_DRAW_MANAGER_HEADER_INCLUDED 
#define PG_GODS_VIEW_DRAW_MANAGER_HEADER_INCLUDED
#pragma once

#include <vulkan/vulkan.h>

#include <vector>

namespace pg::gods_view {

class vulkan_engine;

class draw_manager {
private:
	gods_view::vulkan_engine* engine_;
	std::vector<VkFramebuffer> swap_chain_framebuffers_;
	VkSemaphore image_available_semaphore_;
	VkSemaphore render_finished_semaphore_;
	VkFence inflight_fence_;

public:	
	draw_manager(gods_view::vulkan_engine* init_engine);

	~draw_manager();

	[[nodiscard]] const std::vector<VkFramebuffer>& swap_chain_framebuffers() const noexcept { return swap_chain_framebuffers_; }

	void create_framebuffers();

	void create_sync_objects();

	void draw_frame();
};

} // end namespace pg::gods_view

#endif