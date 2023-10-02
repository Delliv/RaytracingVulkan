#include "../include/window.h"

window::window()
{
	// Create window with GLFW
	width_ = 800;
	height_ = 600;
	if (!glfwInit()) {
		throw std::runtime_error("Error initializating GLFW");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window_ = glfwCreateWindow(width_, 600, "Ventana Vulkan", nullptr, nullptr);

	if (!window_) {
		throw std::runtime_error("Error al crear la ventana GLFW");
	}

	// Here we set the info of our application 
	vk_create_info_ = {};

	vk_create_info_.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	VkApplicationInfo vk_info = {};
	vk_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
	vk_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	vk_info.pEngineName = "No engine";
	vk_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
	vk_info.pApplicationName = "RaytracingPrototype";
	vk_create_info_.pApplicationInfo = &vk_info;


	// ---------------------------------------

	// Vulkan extensions
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;

	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	vk_create_info_.enabledExtensionCount = glfwExtensionCount;
	vk_create_info_.ppEnabledExtensionNames = glfwExtensions;
	// --------------------------------------

	// .......................................

	// Here we create the vulkan instance and also select the GPU that we want
	if (vkCreateInstance(&vk_create_info_, nullptr, &vk_instance_) != VK_SUCCESS) {
		throw std::runtime_error("Error while creating Vulkan instance.");
	}

	unsigned int n_devices = 0;
	vkEnumeratePhysicalDevices(vk_instance_, &n_devices, nullptr);

	if (n_devices == 0) {
		throw std::runtime_error("Devices could not be found");
	}

	std::vector<VkPhysicalDevice> devices(n_devices);
	vkEnumeratePhysicalDevices(vk_instance_, &n_devices, devices.data());

	for (const auto& device : devices) {
		VkPhysicalDeviceProperties deviceProperties;
		vkGetPhysicalDeviceProperties(device, &deviceProperties);

		if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
			vk_physical_device_ = device;
			break;
		}
	}
	VkPhysicalDeviceProperties deviceProperties;
	vkGetPhysicalDeviceProperties(vk_physical_device_, &deviceProperties);
	std::cout << "Selected GPU: " << deviceProperties.deviceName << std::endl;
	if (vk_physical_device_ == VK_NULL_HANDLE) {
		throw std::runtime_error("We could not find a GPU compatible with Vulkan.");
	}

	// --------------------------------------

	// ......................................

	// Here we set the validation layers

	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	vk_create_info_.enabledLayerCount = 2;
	const char* validationLayers[] = {
		"VK_LAYER_KHRONOS_validation",
		"VK_LAYER_LUNARG_api_dump"
	};
	vk_create_info_.ppEnabledLayerNames = validationLayers;

	for (const char* layerName : validationLayers) {
		bool layerFound = false;
		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}

		if (!layerFound) {
			throw std::runtime_error("Capa de validación solicitada no disponible: " + std::string(layerName));
		}
	}


}

window::~window()
{
	vkDestroyInstance(vk_instance_, nullptr);

	glfwDestroyWindow(window_);

	glfwTerminate();
}


void window::end()
{
	glfwDestroyWindow(window_);
	glfwTerminate();
}

void window::clean()
{
	VkClearAttachment clearAttachment = {};
	clearAttachment.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	clearAttachment.clearValue = clearColor;

	VkClearRect clearRect = {};
	clearRect.layerCount = 1;
	clearRect.rect.extent.width = width_;
	clearRect.rect.extent.height = height_;

	//vkCmdClearAttachments()
}

GLFWwindow* window::get_window()
{
	return window_;
}

void window::set_clear_color(float r, float g, float b, float a)
{
	clearColor = { r,g,b,a };
}


