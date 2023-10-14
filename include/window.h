#include "vulkan.h"
#include "glfw3.h"
#include "vulkan_video_codec_h264std.h"
#include <stdexcept>
#include <vector>
#include <iostream>
#include <string>
#include <optional>

#ifndef window_class
#define window_class 

class window {
public:
	window();
	~window();
	struct QueueFamilyIndices {
		std::optional<uint32_t> graphicsFamily;
		std::optional<uint32_t> presentFamily;
		uint32_t queueFamilyCount;

		bool isComplete() {
			return graphicsFamily.has_value() && presentFamily.has_value();
		}
	};

	QueueFamilyIndices findQueueFamilies(VkPhysicalDevice physicalDevice) {
		QueueFamilyIndices indices;

		// Obtener la cantidad de familias de colas disponibles en la GPU
		uint32_t queueFamilyCount = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, nullptr);

		// Obtener información de las familias de colas disponibles
		std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
		vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, &queueFamilyCount, queueFamilies.data());

		// Buscar una familia de colas que admita gráficos y/o presentación
		for (uint32_t i = 0; i < queueFamilyCount; ++i) {
			if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
				indices.graphicsFamily = i;
			}

			VkBool32 presentSupport = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice, i, surface_, &presentSupport);

			if (presentSupport) {
				indices.presentFamily = i;
			}

			if (indices.isComplete()) {
				break;
			}
		}

		return indices;
	}

	void end();
	void clean();

	// Vulkan stuff
	void initialize_vulkan();
	void validation_layers();
	void physical_devices();
	void logical_devices();
	void create_presentation_queue_and_swapchain();
	void obtain_swap_images();
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger);
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator);


	QueueFamilyIndices indices;
	// Getters
	GLFWwindow* get_window();
	VkSurfaceKHR get_vulkan_surface();
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
	VkDevice vk_device_;
	VkPhysicalDevice vk_physical_device_;
	VkSurfaceKHR surface_;
	VkQueue graphicsQueue;
	VkSurfaceFormatKHR selectedFormat;
	VkSwapchainKHR swapChain;
	VkSwapchainCreateInfoKHR swapchainInfo;
	std::vector<VkImage> swapChainImages;
	VkDebugUtilsMessengerEXT debugMessenger;
	std::vector<const char*> validation_layers_;

	uint32_t minImgCount;
	uint32_t maxImgCount;
	VkExtent2D currentExtent;
	VkExtent2D minExtent;
	VkExtent2D maxExtent;
	bool supportsAsync;
	bool presentWithCompositeAlphaSupported;

};


#endif 

