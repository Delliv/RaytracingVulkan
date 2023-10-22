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
	vkDestroyRenderPass(vk_device_, render_pass_,nullptr);
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

	render_pass();
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

void window::render_pass()
{
	color_attachment = {};
	color_attachment.format = swapchain_info.imageFormat;
	// I dont want multisampling
	color_attachment.samples = VK_SAMPLE_COUNT_1_BIT;
	
	// Here i configure the actions to do at the begining and end of rendering
	color_attachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
	color_attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
	color_attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	color_attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;

	// First layout
	color_attachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	// Layout to present using swap chain
	color_attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	// Here i configure the subpasses
	color_attachment_ref = {};
	// Attachment index
	color_attachment_ref.attachment = 0;
	// Layour while subpass
	color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	subpass = {};
	// Type of subpass
	subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount = 1;
	subpass.pColorAttachments = &color_attachment_ref;

	subpass_dependency = {};
	subpass_dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
	subpass_dependency.dstSubpass = 0;
	subpass_dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass_dependency.srcAccessMask = 0;
	subpass_dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
	subpass_dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

	attachments = { color_attachment };

	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments = attachments.data();
	renderPassInfo.subpassCount = 1;
	renderPassInfo.pSubpasses = &subpass;
	renderPassInfo.dependencyCount = 1;
	renderPassInfo.pDependencies = &subpass_dependency;

	if (vkCreateRenderPass(vk_device_, &renderPassInfo, nullptr, &render_pass_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create render pass!");
	}
}

void window::define_descriptors()
{
	descriptor_layout = {};
	descriptor_layout.binding = 0;
	descriptor_layout.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptor_layout.descriptorCount = 1;
	descriptor_layout.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;

	layout_info = {};
	layout_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layout_info.bindingCount = 1;
	layout_info.pBindings = &descriptor_layout;
	
	if (vkCreateDescriptorSetLayout(vk_device_, &layout_info, nullptr, &descriptor_set_layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}

	pool_size = {};
	pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	pool_size.descriptorCount = 1;
	
	pool_info = {};
	pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	pool_info.poolSizeCount = 1;
	pool_info.pPoolSizes = &pool_size;
	pool_info.maxSets = 1;
	if (vkCreateDescriptorPool(vk_device_, &pool_info, nullptr, &descriptor_pool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}

	layouts.push_back(descriptor_set_layout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptor_pool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
	allocInfo.pSetLayouts = layouts.data();
	if (vkAllocateDescriptorSets(vk_device_, &allocInfo, &descriptor_set) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}

	/*
	* TO DO
	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = yourUniformBuffer;
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof(YourUniformBufferObject);
	VkWriteDescriptorSet descriptorWrite{};
	descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrite.dstSet = descriptorSet;
	descriptorWrite.dstBinding = 0;
	descriptorWrite.dstArrayElement = 0;
	descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrite.descriptorCount = 1;
	descriptorWrite.pBufferInfo = &bufferInfo;
	vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);*/
}

void window::create_buffers()
{
	buffer_Info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_Info.size = sizeof(UniformBufferObject);
	buffer_Info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	buffer_Info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(vk_device_, &buffer_Info, nullptr, &buffer_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create Uniform Buffer!");
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

void window::allocate_memory_for_buffers(VkBuffer* buffer, VkDeviceMemory* buffer_memory, const void* data, VkDeviceSize data_size)
{
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(vk_device_, *buffer, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);  // Implementa la función 'findMemoryType' para buscar un tipo de memoria compatible.

	if (vkAllocateMemory(vk_device_, &allocInfo, nullptr, buffer_memory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate Uniform Buffer memory!");
	}

	vkBindBufferMemory(vk_device_, *buffer, *buffer_memory, 0);

	if (data != nullptr && data_size > 0) {
		void* mapped_data;
		vkMapMemory(vk_device_, *buffer_memory, 0, data_size, 0, &mapped_data);
		memcpy(mapped_data, data, data_size);
		vkUnmapMemory(vk_device_, *buffer_memory);
	}
}

uint32_t window::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(vk_physical_device_, &memProperties);  

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
}


