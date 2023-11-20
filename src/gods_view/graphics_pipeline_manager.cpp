#include "gods_view/graphics_pipeline_manager.h"
#include "gods_view/vulkan_engine.h"

#include <fstream>

namespace pg::gods_view {

graphics_pipeline_manager::graphics_pipeline_manager(gods_view::vulkan_engine* init_engine) :
	engine_{init_engine}
{ }

graphics_pipeline_manager::~graphics_pipeline_manager() {
	destroy_pipeline();
}

void graphics_pipeline_manager::create_graphics_pipeline() {
	auto vertex_shader_module = shader_module(read_shader("shaders/vert.spv"));
	auto fragment_shader_module = shader_module(read_shader("shaders/frag.spv"));

	VkPipelineShaderStageCreateInfo vertex_shader_stage_info{};
	vertex_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertex_shader_stage_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
	vertex_shader_stage_info.module = vertex_shader_module;
	vertex_shader_stage_info.pName = "main";

	VkPipelineShaderStageCreateInfo fragment_shader_stage_info{};
	fragment_shader_stage_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragment_shader_stage_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
	fragment_shader_stage_info.module = fragment_shader_module;
	fragment_shader_stage_info.pName = "main";

	VkPipelineShaderStageCreateInfo shader_stages[] = {vertex_shader_stage_info, fragment_shader_stage_info};

	VkPipelineVertexInputStateCreateInfo vertex_input_info{};
	vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_info.vertexBindingDescriptionCount = 0;
	vertex_input_info.vertexAttributeDescriptionCount = 0;

	VkPipelineInputAssemblyStateCreateInfo input_assembly{};
	input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	input_assembly.primitiveRestartEnable = VK_FALSE;

	VkPipelineViewportStateCreateInfo viewport_state{};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.scissorCount = 1;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE;
	rasterizer.rasterizerDiscardEnable = VK_FALSE;
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth = 1.0f;
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
	rasterizer.depthBiasEnable = VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE;
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState color_blend_attachment{};
	color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	color_blend_attachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo color_blending{};
	color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blending.logicOpEnable = VK_FALSE;
	color_blending.logicOp = VK_LOGIC_OP_COPY;
	color_blending.attachmentCount = 1;
	color_blending.pAttachments = &color_blend_attachment;
	color_blending.blendConstants[0] = 0.0f;
	color_blending.blendConstants[1] = 0.0f;
	color_blending.blendConstants[2] = 0.0f;
	color_blending.blendConstants[3] = 0.0f;

	std::vector<VkDynamicState> dynamic_states {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};
	VkPipelineDynamicStateCreateInfo dynamic_state{};
	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
	dynamic_state.pDynamicStates = dynamic_states.data();

	VkPipelineLayoutCreateInfo pipeline_layout_info{};
	pipeline_layout_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_info.setLayoutCount = 0;
	pipeline_layout_info.pushConstantRangeCount = 0;

	auto result = vkCreatePipelineLayout(engine_->device_manager()->logical_device(), &pipeline_layout_info, nullptr, &pipeline_layout_);
	if (result != VK_SUCCESS) {
		throw std::runtime_error{"Failed to crate pipeline layout"};
	}

	VkGraphicsPipelineCreateInfo pipeline_info{};
	pipeline_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipeline_info.stageCount = 2;
	pipeline_info.pStages = shader_stages;
	pipeline_info.pVertexInputState = &vertex_input_info;
	pipeline_info.pInputAssemblyState = &input_assembly;
	pipeline_info.pViewportState = &viewport_state;
	pipeline_info.pRasterizationState = &rasterizer;
	pipeline_info.pMultisampleState = &multisampling;
	pipeline_info.pColorBlendState = &color_blending;
	pipeline_info.pDynamicState = &dynamic_state;
	pipeline_info.layout = pipeline_layout_;
	pipeline_info.renderPass = render_pass_;
	pipeline_info.subpass = 0;
	pipeline_info.basePipelineHandle = VK_NULL_HANDLE;
	if (vkCreateGraphicsPipelines(engine_->device_manager()->logical_device(), VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &graphics_pipeline_) != VK_SUCCESS) {
		throw std::runtime_error{"Failed to create graphics pipeline"};
	}

	vkDestroyShaderModule(engine_->device_manager()->logical_device(), fragment_shader_module, nullptr);
	vkDestroyShaderModule(engine_->device_manager()->logical_device(), vertex_shader_module, nullptr);
}

void graphics_pipeline_manager::create_render_pass() {
	VkAttachmentDescription color_attachment{};
	color_attachment.format = engine_->surface_manager()->swap_chain_image_format();
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentReference color_attachment_ref{};
	color_attachment_ref.attachment = 0;
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	VkRenderPassCreateInfo renderpass_info{};
	renderpass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderpass_info.attachmentCount = 1;
	renderpass_info.pAttachments = &color_attachment;
	renderpass_info.subpassCount = 1;
	renderpass_info.pSubpasses = &subpass;

	if (vkCreateRenderPass(engine_->device_manager()->logical_device(), &renderpass_info, nullptr, &render_pass_) != VK_SUCCESS) {
		throw std::runtime_error{"Failed to create render pass"};
	}
}

std::vector<char> graphics_pipeline_manager::read_shader(const std::string& filename) {
	std::ifstream file{filename, std::ios::ate | std::ios::binary};
	if (!file.is_open()) {
		throw std::runtime_error{"Failed to open file"};
	}
	size_t file_size = static_cast<size_t>(file.tellg());
	std::vector<char> buffer{};
	buffer.resize(file_size);
	file.seekg(0);
	file.read(buffer.data(), file_size);
	return buffer;
}

VkShaderModule graphics_pipeline_manager::shader_module(const std::vector<char>& shader_bytecode) {
	VkShaderModuleCreateInfo create_info{};
	create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	create_info.codeSize = shader_bytecode.size();
	create_info.pCode = reinterpret_cast<const uint32_t*>(shader_bytecode.data());

	VkShaderModule shader_module;
	if (vkCreateShaderModule(engine_->device_manager()->logical_device(), &create_info, nullptr, &shader_module) != VK_SUCCESS) {
		throw std::runtime_error{"Failed to create shader module"};
	}
	return shader_module;
}

void graphics_pipeline_manager::destroy_pipeline() {
	vkDestroyPipeline(engine_->device_manager()->logical_device(), graphics_pipeline_, nullptr);
	vkDestroyPipelineLayout(engine_->device_manager()->logical_device(), pipeline_layout_, nullptr);
	vkDestroyRenderPass(engine_->device_manager()->logical_device(), render_pass_, nullptr);
}

} // end namespace pg::gods_view