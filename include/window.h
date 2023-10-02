#include "vulkan.h"
#include "glfw3.h"
#include "vulkan_video_codec_h264std.h"
#include <stdexcept>
#include <vector>
#include <iostream>
#include <string>
#ifndef window_class
#define window_class 

class window {
public:
	window();
	~window();
	void end();
	void clean();

	// Getters
	GLFWwindow* get_window();

	// Setters
	void set_clear_color(float r, float g, float b, float a);

private:
	// Window stuff
	GLFWwindow* window_;
	VkClearValue clearColor;
	int width_;
	int height_;


	// Vulkan stuff
	VkInstanceCreateInfo vk_create_info_;
	VkInstance vk_instance_;
	VkPhysicalDevice vk_physical_device_;
};


#endif 
