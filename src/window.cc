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

	window_ = glfwCreateWindow(width_, 600, "Vulkan Window", nullptr, nullptr);

	if (!window_) {
		throw std::runtime_error("Error creating GLFW window");
	}

	initialize_vulkan();

}

window::~window()
{
	vkDestroySurfaceKHR(vk_instance_, surface_, nullptr);
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

	vkDestroyDevice(vk_device_, nullptr);
	//vkCmdClearAttachments()
}

void window::initialize_vulkan()
{
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
	validation_layers();
	vk_create_info_.enabledLayerCount = static_cast<uint32_t>(validation_layers_.size());
	vk_create_info_.ppEnabledLayerNames = validation_layers_.data();
	// Here we create the vulkan instance and also select the GPU that we want
	if (vkCreateInstance(&vk_create_info_, nullptr, &vk_instance_) != VK_SUCCESS) {
		throw std::runtime_error("Error while creating Vulkan instance.");
	}


	physical_devices();

	//validation_layers();

	logical_devices();

	create_presentation_queue_and_swapchain();

	create_views();
}

void window::validation_layers()
{
	uint32_t layerCount = 0;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	const char* desiredValidationLayers[] = {
		"VK_LAYER_KHRONOS_validation",
		"VK_LAYER_LUNARG_api_dump"
	};

	for (const char* layerName : desiredValidationLayers) {
		bool layerFound = false;
		for (const auto& layerProperties : availableLayers) {
			if (strcmp(layerName, layerProperties.layerName) == 0) {
				layerFound = true;
				break;
			}
		}
		if (!layerFound) {
			throw std::runtime_error("Validation layer not available.");
		}
	}

	// Mueve las capas de validación a la lista de miembros en lugar de 
	// asignar una matriz local.
	validation_layers_ = std::vector<const char*>(std::begin(desiredValidationLayers), std::end(desiredValidationLayers));
}


void window::physical_devices()
{
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


}

void window::logical_devices()
{
	if (glfwCreateWindowSurface(vk_instance_, window_, nullptr, &surface_) != VK_SUCCESS) {
		throw std::runtime_error("Error while creating surface screen");
	}

	indices = findQueueFamilies(vk_physical_device_);
	
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	float queue_priority = 1.0f;

	if (indices.graphicsFamily.has_value()) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queue_priority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	if (indices.presentFamily.has_value() && indices.presentFamily != indices.graphicsFamily) {
		VkDeviceQueueCreateInfo queueCreateInfo = {};
		queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex = indices.presentFamily.value();
		queueCreateInfo.queueCount = 1;
		queueCreateInfo.pQueuePriorities = &queue_priority;
		queueCreateInfos.push_back(queueCreateInfo);
	}


	VkPhysicalDeviceFeatures deviceFeatures{};
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.enabledExtensionCount = 1;
	const char* deviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
	createInfo.ppEnabledExtensionNames = deviceExtensions;

	createInfo.pEnabledFeatures = &deviceFeatures;


	if (vkCreateDevice(vk_physical_device_, &createInfo, nullptr, &vk_device_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}



	VkBool32 surfaceSupport = VK_FALSE;
	vkGetPhysicalDeviceSurfaceSupportKHR(vk_physical_device_, indices.graphicsFamily.value(), surface_, &surfaceSupport);

	if (surfaceSupport != VK_TRUE) {
		throw std::runtime_error("El dispositivo físico no es compatible con la superficie de ventana.");
	}
}

void window::create_presentation_queue_and_swapchain()
{
	vkGetPhysicalDeviceQueueFamilyProperties(vk_physical_device_, &indices.queueFamilyCount, nullptr);
	

	VkBool32 surfaceSupport = VK_FALSE;
	for (uint32_t i = 0; i < indices.queueFamilyCount; ++i) {
		if (vkGetPhysicalDeviceSurfaceSupportKHR(vk_physical_device_, i, surface_, &surfaceSupport) == VK_SUCCESS && surfaceSupport) {
			vkGetDeviceQueue(vk_device_, i, 0, &graphicsQueue);
			break;
		}
	}

	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device_, surface_, &formatCount, nullptr);
	
	if (formatCount == 0) {
		throw std::runtime_error("We could not find any surface formats compatible");
	}

	std::vector<VkSurfaceFormatKHR> surfaceFormats(formatCount);
	vkGetPhysicalDeviceSurfaceFormatsKHR(vk_physical_device_, surface_, &formatCount, surfaceFormats.data());
	
	selectedFormat = surfaceFormats[0];

	for (const VkSurfaceFormatKHR& availableFormat : surfaceFormats) {
		if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
			availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
			selectedFormat = availableFormat;
			break;
		}
	}


	VkSurfaceCapabilitiesKHR surfaceCapabilities;
	if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk_physical_device_, surface_, &surfaceCapabilities) != VK_SUCCESS) {
		throw std::runtime_error("Error awhile obtaining the surface capabilities.");
	}

	minImgCount = 2;//surfaceCapabilities.minImageCount;
	maxImgCount = surfaceCapabilities.maxImageCount;
	currentExtent = surfaceCapabilities.currentExtent;
	minExtent = surfaceCapabilities.minImageExtent;
	maxExtent = surfaceCapabilities.maxImageExtent;

	supportsAsync = (surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR) != 0;
	presentWithCompositeAlphaSupported = (surfaceCapabilities.supportedCompositeAlpha & VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR) != 0;

	swapchain_info = {};
	swapchain_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	swapchain_info.surface = surface_;
	swapchain_info.minImageCount = minImgCount;
	/*if (maxImgCount > 0 && (minImgCount + 1) <= maxImgCount) {
		swapchainInfo.minImageCount = minImgCount + 1;
	}*/


	swapchain_info.imageFormat = selectedFormat.format;
	swapchain_info.imageColorSpace = selectedFormat.colorSpace;
	swapchain_info.imageExtent = currentExtent;
	swapchain_info.imageArrayLayers = 1;


	swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	uint32_t queueFamilyIndices[] = {
		indices.graphicsFamily.value(),
		indices.presentFamily.value()
	};

	if (indices.graphicsFamily != indices.presentFamily) {
		swapchain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
		swapchain_info.queueFamilyIndexCount = 2;
		swapchain_info.pQueueFamilyIndices = queueFamilyIndices;
	}
	else {
		swapchain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		swapchain_info.pQueueFamilyIndices = nullptr;
	}

	swapchain_info.preTransform = surfaceCapabilities.currentTransform;

	VkCompositeAlphaFlagBitsKHR compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	if (supportsAsync) {
		compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	}
	else if (presentWithCompositeAlphaSupported) {
		compositeAlpha = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
	}
	swapchain_info.compositeAlpha = compositeAlpha;
	swapchain_info.presentMode = VK_PRESENT_MODE_FIFO_KHR; // Ajusta según tus necesidades
	swapchain_info.clipped = VK_TRUE;
	swapchain_info.oldSwapchain = VK_NULL_HANDLE; // Solo relevante al recrear la cadena

	if (vkCreateSwapchainKHR(vk_device_, &swapchain_info, nullptr, &swapChain) != VK_SUCCESS) {
		throw std::runtime_error("Error while creating the swap chain.");
	}

}

void window::obtain_swap_images()
{

	if (vkGetSwapchainImagesKHR(vk_device_, swapChain, &swapchain_info.minImageCount, nullptr) != VK_SUCCESS) {
		throw std::runtime_error("Error obtaining the number of images from the swap chain.");
	}
	swap_chain_images.resize(swapchain_info.minImageCount);
	if (vkGetSwapchainImagesKHR(vk_device_, swapChain, &swapchain_info.minImageCount, swap_chain_images.data()) != VK_SUCCESS) {
		throw std::runtime_error("Error obtaining the number of images from the swap chain.");
	}
}

void window::create_views()
{
	// First we have to get the images to access them
	uint32_t imageCount;
	vkGetSwapchainImagesKHR(vk_device_, swapChain, &imageCount, nullptr);

	// Then we save those images in a vector
	swap_chain_images.resize(imageCount);
	vk_image_views.resize(imageCount);

	vkGetSwapchainImagesKHR(vk_device_, swapChain, &imageCount, swap_chain_images.data());
	// Now that swapChainImages has all the images we need we have to create the views, since we will have 2 i will do it inside a for
	for (size_t i = 0; i < imageCount; i++){
		VkImageViewCreateInfo viewInfo{};
		// I put the exact same format as our swap chain
		viewInfo.format = swapchain_info.imageFormat;
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		// Here we are passing the image to our view info
		viewInfo.image = swap_chain_images.at(i);

		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(vk_device_, &viewInfo, nullptr, &vk_image_views.at(i)) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create image view!");
		}
	}

}

static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
	VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
	VkDebugUtilsMessageTypeFlagsEXT messageType,
	const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
	void* pUserData) {

	// Imprime información de depuración o registra eventos de depuración aquí
	if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
		// Mensaje de advertencia o error, registra o muestra información de depuración
		std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
	}

	return VK_FALSE;
}

GLFWwindow* window::get_window()
{
	return window_;
}

VkSurfaceKHR window::get_vulkan_surface()
{
	return surface_;
}

void window::set_clear_color(float r, float g, float b, float a)
{
	clearColor = { r,g,b,a };
}


