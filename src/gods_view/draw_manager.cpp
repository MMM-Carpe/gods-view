#include "gods_view/draw_manager.h"
#include "gods_view/vulkan_engine.h"

namespace pg::gods_view {

draw_manager::draw_manager(gods_view::vulkan_engine* init_engine) :
	engine_{init_engine}
{ }

draw_manager::~draw_manager() {
	vkDestroySemaphore(engine_->device_manager()->logical_device(), render_finished_semaphore_, nullptr);
	vkDestroySemaphore(engine_->device_manager()->logical_device(), image_available_semaphore_, nullptr);
	vkDestroyFence(engine_->device_manager()->logical_device(), inflight_fence_, nullptr);
	for (auto framebuffer : swap_chain_framebuffers_) {
		vkDestroyFramebuffer(engine_->device_manager()->logical_device(), framebuffer, nullptr);
	}
}

void draw_manager::create_framebuffers() {
	swap_chain_framebuffers_.resize(engine_->surface_manager()->swap_chain_image_views().size());
	for (size_t i = 0; i < engine_->surface_manager()->swap_chain_image_views().size(); ++i) {
		VkImageView attachments[] = {
			engine_->surface_manager()->swap_chain_image_views()[i]
		};
		VkFramebufferCreateInfo framebuffer_info{};
		framebuffer_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebuffer_info.renderPass = engine_->graphics_pipeline_manager()->render_pass();
		framebuffer_info.attachmentCount = 1;
		framebuffer_info.pAttachments = attachments;
		framebuffer_info.width = engine_->surface_manager()->swap_chain_extent().width;
		framebuffer_info.height = engine_->surface_manager()->swap_chain_extent().height;
		framebuffer_info.layers = 1;

		if (vkCreateFramebuffer(engine_->device_manager()->logical_device(), &framebuffer_info, nullptr, &swap_chain_framebuffers_[i]) != VK_SUCCESS) {
			throw std::runtime_error{"Failed to create framebuffer"};
		}
	}
}

void draw_manager::create_sync_objects() {
	VkSemaphoreCreateInfo semaphore_info{};
	semaphore_info.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
	
	VkFenceCreateInfo fence_info{};
	fence_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fence_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	if (vkCreateSemaphore(engine_->device_manager()->logical_device(), &semaphore_info, nullptr, &image_available_semaphore_) != VK_SUCCESS ||
		vkCreateSemaphore(engine_->device_manager()->logical_device(), &semaphore_info, nullptr, &render_finished_semaphore_) != VK_SUCCESS ||
		vkCreateFence(engine_->device_manager()->logical_device(), &fence_info, nullptr, &inflight_fence_) != VK_SUCCESS)
	{
		throw std::runtime_error{"Failed to create synchronization objects for a frame"};
	}
}

void draw_manager::draw_frame() {
	vkWaitForFences(engine_->device_manager()->logical_device(), 1, &inflight_fence_, VK_TRUE, UINT64_MAX);
	vkResetFences(engine_->device_manager()->logical_device(), 1, &inflight_fence_);

	uint32_t image_index;
	vkAcquireNextImageKHR(
		engine_->device_manager()->logical_device(),
		engine_->surface_manager()->swapchain(),
		UINT64_MAX,
		image_available_semaphore_,
		VK_NULL_HANDLE,
		&image_index
	);
	auto command_buffer = engine_->command_manager()->command_buffer();
	vkResetCommandBuffer(command_buffer, 0);
	engine_->command_manager()->record_command_buffer(command_buffer, image_index);

	VkSubmitInfo submit_info{};
	submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	VkSemaphore wait_semaphores[] = {image_available_semaphore_};
	VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submit_info.waitSemaphoreCount = 1;
	submit_info.pWaitSemaphores = wait_semaphores;
	submit_info.pWaitDstStageMask = wait_stages;
	submit_info.commandBufferCount = 1;
	submit_info.pCommandBuffers = &command_buffer;

	VkSemaphore signal_semaphores[] = {render_finished_semaphore_};
	submit_info.signalSemaphoreCount = 1;
	submit_info.pSignalSemaphores = signal_semaphores;

	if (vkQueueSubmit(engine_->device_manager()->graphics_queue(), 1, &submit_info, inflight_fence_) != VK_SUCCESS) {
		throw std::runtime_error{"Failed to submit draw command buffer"};
	}

	VkPresentInfoKHR present_info{};
	present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	present_info.waitSemaphoreCount = 1;
	present_info.pWaitSemaphores = signal_semaphores;
	VkSwapchainKHR swapchains[] = {engine_->surface_manager()->swapchain()};
	present_info.swapchainCount = 1;
	present_info.pSwapchains = swapchains;
	present_info.pImageIndices = &image_index;
	vkQueuePresentKHR(engine_->device_manager()->present_queue(), &present_info);
}

} // end namespace pg::gods_view