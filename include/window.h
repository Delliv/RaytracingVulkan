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
#include "vulkan_loader.h"
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
	void create_views();
	VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pMessenger);
	void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks* pAllocator);
	void render_pass();
	void define_descriptors();
	void create_buffers();
	void create_framebuffers();
	void create_command_pool();
	void create_command_buffers();
	void record_command_buffers();

	// Change these names
	void createDescriptorSetLayout();
	void createDescriptorPool();
	void createDescriptorSets();
	void updateDescriptorSets();

	QueueFamilyIndices indices;
	// Getters
	GLFWwindow* get_window();
	VkSurfaceKHR get_vulkan_surface();
	// Setters
	void set_clear_color(float r, float g, float b, float a);

	void allocate_memory_for_buffers(VkBuffer* buffer, VkDeviceMemory* buffer_memory, const void* data, VkDeviceSize data_size);
	uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

	std::vector<object> scene_objects_;

	friend class render;
	friend class object;
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
	// Swap chain
	VkSwapchainKHR swapChain;
	std::vector<VkImage> swap_chain_images;
	VkSwapchainCreateInfoKHR swapchain_info;
	VkDebugUtilsMessengerEXT debugMessenger;
	std::vector<const char*> validation_layers_;
	// Render pass
	VkAttachmentDescription color_attachment;
	VkAttachmentReference color_attachment_ref;
	VkSubpassDescription subpass;
	VkSubpassDependency subpass_dependency;
	std::array<VkAttachmentDescription, 1> attachments;
	VkRenderPass render_pass_;

	VkPhysicalDeviceProperties deviceProperties;

	// Descriptors
	VkDescriptorSetLayoutBinding descriptor_layout;
	VkDescriptorSetLayoutCreateInfo layout_info;
	VkDescriptorPoolSize pool_size;
	VkDescriptorPoolCreateInfo pool_info;
	VkDescriptorSetLayout descriptor_set_layout;
	VkDescriptorPool descriptor_pool;
	std::vector<VkDescriptorSetLayout> layouts;
	std::vector<VkDescriptorSet> descriptors_set;
	std::vector<VkDescriptorSet> descriptorSets;

	// Change these names
	VkDescriptorSetLayout descriptorSetLayoutModelMatrix; // Para la matriz modelo
	VkDescriptorSetLayout descriptorSetLayoutVertex;      // Para los vértices
	VkDescriptorSetLayout descriptorSetLayoutBLAS;      // Para los vértices

	std::vector<VkDescriptorSet> descriptorSetsModelMatrix;
	std::vector<VkDescriptorSet> descriptorSetsVertex;
	std::vector<VkDescriptorSet> descriptorSetsBLAS;

	// Change these names
	std::vector<VkBuffer> modelMatrixBuffers;
	std::vector<VkDeviceMemory> modelMatrixBuffersMemory;

	std::vector<VkBuffer> vertexBuffers;
	std::vector<VkDeviceMemory> vertexBuffersMemory;


	// Buffers
	std::vector<VkBuffer> buffers_;
	VkBufferCreateInfo buffer_Info;
	std::vector<VkDeviceMemory> uniforms_buffers_memory_;
	UniformBufferObject ubo_;

	// Framebuffers
	std::vector<VkFramebuffer> framebuffers_;

	// Command pool and command buffers
	VkCommandPool command_pool_;
	std::vector<VkCommandBuffer> command_buffers;
	// Views
	std::vector<VkImageView> vk_image_views;
	uint32_t minImgCount;
	uint32_t maxImgCount;
	VkExtent2D currentExtent;
	VkExtent2D minExtent;
	VkExtent2D maxExtent;
	bool supportsAsync;
	bool presentWithCompositeAlphaSupported;


	VulkanLoader vk_loader_;
	
};


#endif 

