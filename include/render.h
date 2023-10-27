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

#ifndef render_class
#define render_class 


class render {
public:
	render();
	render(window* w);
	~render();
	
	void create_command_pool();
	void create_command_buffers();
	void record_command_buffers();
	void create_pipeline();
	VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code);
	std::vector<char> readFile(const std::string& filename);

private:
	// Command pool and command buffers
	VkCommandPool command_pool_;
	std::vector<VkCommandBuffer> command_buffers;
	
	// Pipeline
	VkPipeline pipeline_;
	VkShaderModule fragment_shader_;
	VkShaderModule vertex_shader_;
	window* window_;
};


#endif 

