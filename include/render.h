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

class camera;
class render {
public:
	render();
	render(window* w, camera* c);
	~render();

	void create_pipeline();
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
	VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code);
	void create_SBT_buffer();
	std::vector<char> readFile(const std::string& filename);
	// Command buffer
	void create_command_pool();
	void create_command_buffers();
	void record_command_buffers();

	void render_scene();
	void init_fences();
	void init_semaphore();

	uint8_t current_frame_;


	// Descriptors
	void createSpecificDescriptorSetLayouts();
	void createDescriptorPool();
	void createDescriptorSets();
	void updateDescriptorSets();
	friend window;
private:
	// Command pool and command buffers
	VkCommandPool command_pool_;
	std::vector<VkCommandBuffer> command_buffers;
	VkCommandBuffer temp_command_buffer;

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


	// Render the scene
	std::vector<VkFence> in_flight_fences_;
	std::vector<VkFence> in_flight_images_;
	VkFence temp_com_buffer_fence_;
	std::vector<VkSemaphore> imageAvailableSemaphores;
	std::vector<VkSemaphore> renderFinishedSemaphores;

	const int MAX_FRAMES_IN_FLIGHT = 2; // o cualquier número que necesites
	VkSemaphore copyFinishedSemaphore;
	VkSemaphore signalSemaphores[2];


	// Descriptors
	VkDescriptorSetLayoutBinding descriptor_layout;
	VkDescriptorSetLayoutCreateInfo layout_info;
	VkDescriptorPoolSize pool_size;
	VkDescriptorPoolCreateInfo pool_info;
	VkDescriptorSetLayout descriptor_set_layout;	// Currently i am using this one
	VkDescriptorPool descriptor_pool;
	std::vector<VkDescriptorSetLayout> layouts;
	VkDescriptorSet descriptorSets_;

	camera* camera_;

	std::array<VkPipelineShaderStageCreateInfo, 3> shaderStages;
	std::array<VkRayTracingShaderGroupCreateInfoKHR, 3> shaderGroups;
};

#endif 

