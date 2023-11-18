#ifndef render_class
#define render_class 

#include "vulkan.h"
#include "glfw3.h"
#include "vulkan_video_codec_h264std.h"
#include <stdexcept>
#include <vector>
#include <array>
#include "uniforms.h"
#include <iostream>
#include <string>
#include <optional>
#include "../include/window.h"
#include <fstream>
#include "vulkan_loader.h"

class render {
public:
	render();
	render(window* w);
	~render();
	
	void create_pipeline();
	void create_raytracing_buffers();
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code);
	void create_SBT_buffer();
	std::vector<char> readFile(const std::string& filename);

private:
	// Command pool and command buffers
	VkCommandPool command_pool_;
	std::vector<VkCommandBuffer> command_buffers;
	
	// Pipeline
	VkPipeline pipeline_;
	VkPipelineLayout pipelineLayout;
	window* window_;
	VkShaderModule raygen;
	VkShaderModule miss;
	VkShaderModule closestHit;

	// Raytracing
	VkBuffer rays_buffer_;
	VkBuffer results_buffer_;
	VkBuffer SBT_buffer_;
	VkDeviceMemory SBT_buffer_memory_;
	uint32_t SBT_size_;
	uint32_t stride_size_;
	VulkanLoader* vulkanLoader;

	// Pointers to Vulkan acceleration functions
	PFN_vkCreateRayTracingPipelinesKHR pfnVkCreateRayTracingPipelinesKHR;
	PFN_vkGetRayTracingShaderGroupHandlesKHR pfnVkGetRayTracingShaderGroupHandlesKHR;
	PFN_vkCmdTraceRaysKHR pfnVkCmdTraceRaysKHR;
	PFN_vkCmdBuildAccelerationStructuresKHR pfnVkCmdBuildAccelerationStructuresKHR;
	// Command buffer
	void create_command_pool();
	void create_command_buffers();
	void record_command_buffers();
};


#endif 

