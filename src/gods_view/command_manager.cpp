#include "gods_view/command_manager.h"
#include "gods_view/vulkan_engine.h"

namespace pg::gods_view {

command_manager::command_manager(gods_view::vulkan_engine* init_engine) :
	engine_{init_engine}
{ }	

command_manager::~command_manager() {
	vkDestroyCommandPool(engine_->device_manager()->logical_device(), command_pool_, nullptr);
}

void command_manager::create_command_pool() {
	queue_family_indices indices = details::find_queue_families(
		engine_->device_manager()->physical_device(), 
		engine_->surface_manager()->surface()
	);
	VkCommandPoolCreateInfo pool_info{};
	pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	pool_info.queueFamilyIndex = indices.graphics_family.value();
	if (vkCreateCommandPool(engine_->device_manager()->logical_device(), &pool_info, nullptr, &command_pool_) != VK_SUCCESS) {
		throw std::runtime_error{"Failed to create command pool"};
	}
}

void command_manager::create_command_buffer() {
	VkCommandBufferAllocateInfo allocate_info{};
	allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocate_info.commandPool = command_pool_;
	allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocate_info.commandBufferCount = 1;
	if (vkAllocateCommandBuffers(engine_->device_manager()->logical_device(), &allocate_info, &command_buffer_) != VK_SUCCESS) {
		throw std::runtime_error{"Failed to allocate command buffers"};
	}
}

void command_manager::record_command_buffer(VkCommandBuffer command_buffer, uint32_t image_index) {
	VkCommandBufferBeginInfo begin_info{};
	begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
		throw std::runtime_error{"Failed to begin recording command buffer"};
	}
	
	VkRenderPassBeginInfo renderpass_info{};
	renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderpass_info.renderPass = engine_->graphics_pipeline_manager()->render_pass();
	renderpass_info.framebuffer = engine_->draw_manager()->swap_chain_framebuffers()[image_index];
	renderpass_info.renderArea.offset = {0, 0};
	renderpass_info.renderArea.extent = engine_->surface_manager()->swap_chain_extent();

	VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
	renderpass_info.clearValueCount = 1;
	renderpass_info.pClearValues = &clear_color;

	vkCmdBeginRenderPass(command_buffer_, &renderpass_info, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(command_buffer_, VK_PIPELINE_BIND_POINT_GRAPHICS, engine_->graphics_pipeline_manager()->graphics_pipeline());
	
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = static_cast<float>(engine_->surface_manager()->swap_chain_extent().width);
	viewport.height = static_cast<float>(engine_->surface_manager()->swap_chain_extent().height);
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;
	vkCmdSetViewport(command_buffer_, 0, 1, &viewport);
	
	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = engine_->surface_manager()->swap_chain_extent();
	vkCmdSetScissor(command_buffer_, 0, 1, &scissor);
	vkCmdDraw(command_buffer_, 3, 1, 0, 0);
	vkCmdEndRenderPass(command_buffer_);
	
	if (vkEndCommandBuffer(command_buffer_) != VK_SUCCESS) {
		throw std::runtime_error{"Failed to record command buffer"};
	}
}

} // end namespace pg::gods_view