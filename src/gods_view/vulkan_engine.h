#if !defined PG_GODS_VIEW_VULKAN_ENGINE_HEADER_INCLUDED
#define PG_GODS_VIEW_VULKAN_ENGINE_HEADER_INCLUDED
#pragma once

#include "gods_view/validation_layers.h"
#include "gods_view/device_manager.h"
#include "gods_view/surface_manager.h"
#include "gods_view/vulkan_instance.h"
#include "gods_view/graphics_pipeline_manager.h"
#include "gods_view/draw_manager.h"
#include "gods_view/command_manager.h"
#include "gods_view/window.h"

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"

#include <cstdint>
#include <cstring>
#include <string>
#include <system_error>

namespace pg::gods_view {

class vulkan_engine {
private:
	gods_view::validation_layer_manager validation_layer_manager_;
	gods_view::vulkan_instance vulkan_instance_;
	gods_view::debug_messenger debug_messenger_;
	gods_view::device_manager device_manager_;
	gods_view::surface_manager surface_manager_;
	gods_view::graphics_pipeline_manager graphics_pipeline_manager_;
	gods_view::draw_manager draw_manager_;
	gods_view::command_manager command_manager_;
	GLFWwindow* current_window_;

public:
	vulkan_engine(
		const std::string& init_engine_name,
		const std::string& init_app_name = "PG"
	);

	[[nodiscard]] std::string engine_name() const noexcept { return std::string{vulkan_instance_.engine_name()}; }

	[[nodiscard]] std::string application_name() const noexcept { return std::string{vulkan_instance_.app_name()}; }

	[[nodiscard]] gods_view::vulkan_instance* vulkan_instance() noexcept { return &vulkan_instance_; }

	[[nodiscard]] gods_view::validation_layer_manager* validation_layer_manager() noexcept { return &validation_layer_manager_; }

	[[nodiscard]] gods_view::surface_manager* surface_manager() noexcept { return &surface_manager_; } 

	[[nodiscard]] gods_view::device_manager* device_manager() noexcept { return &device_manager_; }

	[[nodiscard]] gods_view::graphics_pipeline_manager* graphics_pipeline_manager() noexcept { return &graphics_pipeline_manager_; }

	[[nodiscard]] gods_view::draw_manager* draw_manager() noexcept { return &draw_manager_; }

	[[nodiscard]] gods_view::command_manager* command_manager() noexcept { return &command_manager_; }

	[[nodiscard]] GLFWwindow* current_window() const noexcept { return current_window_; }

	void current_window(GLFWwindow* window) noexcept { current_window_ = window; }

	void create_vulkan_surface(GLFWwindow* window) {
		surface_manager_.create_vulkan_surface(window);
	}

	void initialize_device_manager() {
		device_manager_.grab_physical_device();
		device_manager_.create_logical_device();
	}

	void create_swap_chain() {
		surface_manager_.create_swap_chain();
	}

	void create_image_views() {
		surface_manager_.create_image_views();
	}

	void create_graphics_pipeline() {
		graphics_pipeline_manager_.create_graphics_pipeline();
	}

	void create_render_pass() {
		graphics_pipeline_manager_.create_render_pass();
	}

	void create_framebuffers() {
		draw_manager_.create_framebuffers();
	}

	void create_command_pool() {
		command_manager_.create_command_pool();
	}

	void create_command_buffer() {
		command_manager_.create_command_buffer();
	}

	void create_synchronization_objects() {
		draw_manager_.create_sync_objects();
	}
};

} // end namespace pg::gods_view

#endif