#if !defined PG_GODS_VIEW_WINDOW_HEADER_INCLUDED
#define PG_GODS_VIEW_WINDOW_HEADER_INCLUDED
#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <cstdint>
#include <string>

namespace pg::gods_view {

class vulkan_window {
private:
	using window_handle_type = GLFWwindow*;

	window_handle_type window_;
	std::string window_name_;
	uint32_t width_;
	uint32_t height_;

public:
	vulkan_window(
		const std::string& init_window_name,
		uint32_t init_width,
		uint32_t init_height
	) :
		window_{nullptr},
		window_name_{init_window_name},
		width_{init_width},
		height_{init_height}
	{ }

	~vulkan_window() {
		glfwDestroyWindow(window_);
		glfwTerminate();
	}

	void initiate_window() {
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
		window_ = glfwCreateWindow(width_, height_, window_name_.c_str(), nullptr, nullptr);
	}

	bool should_window_close() {
		return glfwWindowShouldClose(window_);
	}

	[[nodiscard]] window_handle_type handle() const noexcept { return window_; }
};

} // end namespace pg::gods_view


#endif