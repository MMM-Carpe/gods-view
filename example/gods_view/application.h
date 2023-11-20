#include "gods_view/vulkan_engine.h"
#include "gods_view/window.h"

#include <cstdint>

namespace pg::example {

class application {
private:
	gods_view::vulkan_window window_;
	gods_view::vulkan_engine engine_;

public:
	application(
		const std::string& app_name,
		uint32_t width,
		uint32_t height
	) :
		window_{app_name, width, height},
		engine_{app_name}
	{ }

	void run() {
		window_.initiate_window();
		engine_.current_window(window_.handle());
		engine_.create_vulkan_surface(window_.handle());
		engine_.initialize_device_manager();
		engine_.create_swap_chain();
		engine_.create_image_views();
		engine_.create_render_pass();
		engine_.create_graphics_pipeline();
		engine_.create_framebuffers();
		engine_.create_command_pool();
		engine_.create_command_buffer();
		engine_.create_synchronization_objects();
		while (!window_.should_window_close()) {
			glfwPollEvents();
			engine_.draw_manager()->draw_frame();
		}
		vkDeviceWaitIdle(engine_.device_manager()->logical_device());
	}
};

} // end namespace pg::example
