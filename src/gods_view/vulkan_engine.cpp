#include "gods_view/vulkan_engine.h"

namespace pg::gods_view {

vulkan_engine::vulkan_engine(
	const std::string& init_engine_name,
	const std::string& init_app_name
) :
	validation_layer_manager_{},
	vulkan_instance_{init_app_name, init_engine_name, validation_layer_manager_},
	debug_messenger_{vulkan_instance_.vk_instance()},
	device_manager_{this},
	surface_manager_{this},
	graphics_pipeline_manager_{this},
	draw_manager_{this},
	command_manager_{this}
{ }

} // end namespace pg::gods_view