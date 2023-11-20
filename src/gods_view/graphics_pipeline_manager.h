#if !defined PG_GODS_VIEW_GRAPHICS_PIPELINE_HEADER_INCLUDED
#define PG_GODS_VIEW_GRAPHICS_PIPELINE_HEADER_INCLUDED
#pragma once

#include <vulkan/vulkan.h>

#include <string>
#include <vector>

namespace pg::gods_view {

class vulkan_engine;

class graphics_pipeline_manager {
private:	
	gods_view::vulkan_engine* engine_;
	VkRenderPass render_pass_;
	VkPipelineLayout pipeline_layout_;
	VkPipeline graphics_pipeline_;

public:
	graphics_pipeline_manager(gods_view::vulkan_engine* engine);

	~graphics_pipeline_manager();

	[[nodiscard]] VkRenderPass render_pass() const noexcept { return render_pass_; }

	[[nodiscard]] VkPipeline graphics_pipeline() const noexcept { return graphics_pipeline_; }

	void create_graphics_pipeline();

	void create_render_pass();

	std::vector<char> read_shader(const std::string& filename);

	VkShaderModule shader_module(const std::vector<char>& shader_bytecode);

private:
	void destroy_pipeline();
};

} // end namespace pg::gods_view

#endif