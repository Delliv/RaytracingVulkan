#include "../include/window.h"
#include "../include/object.h"
#include "../include/render.h"
#include "../include/camera.h"

window::window()
{
	// Create window with GLFW
	width_ = 800;
	height_ = 600;
	blas_id_ = 0;
	if (!glfwInit()) {
		throw std::runtime_error("Error initializating GLFW");
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window_ = glfwCreateWindow(width_, 600, "Vulkan Window", nullptr, nullptr);

	if (!window_) {
		throw std::runtime_error("Error creating GLFW window");
	}
	
	initialize_vulkan();
	pfnVkCreateAccelerationStructureKHR = (PFN_vkCreateAccelerationStructureKHR)vkGetDeviceProcAddr(vk_device_, "vkCreateAccelerationStructureKHR");
	pfnVkCmdBuildAccelerationStructuresKHR = (PFN_vkCmdBuildAccelerationStructuresKHR)vkGetDeviceProcAddr(vk_device_, "vkCmdBuildAccelerationStructuresKHR");
	pfnVkGetAccelerationStructureBuildSizesKHR = (PFN_vkGetAccelerationStructureBuildSizesKHR)vkGetDeviceProcAddr(vk_device_, "vkGetAccelerationStructureBuildSizesKHR");
	PpfnVkGetBufferDeviceAddressKHR = (PFN_vkGetBufferDeviceAddressKHR)vkGetDeviceProcAddr(vk_device_, "vkGetBufferDeviceAddressKHR");

	//createDescriptorSetLayout();
	//createDescriptorPool();
	//createDescriptorSets();
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
	vk_info.apiVersion = VK_API_VERSION_1_3;
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

	// Imprimir las extensiones soportadas por el VkPhysicalDevice seleccionado
	uint32_t extensionCount = 0;
	vkEnumerateDeviceExtensionProperties(vk_physical_device_, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> extensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(vk_physical_device_, nullptr, &extensionCount, extensions.data());

	std::cout << "Extensiones soportadas:" << std::endl;
	for (const auto& extension : extensions) {
		std::cout << '\t' << extension.extensionName << std::endl;
	}

	//validation_layers();

	logical_devices();

	create_presentation_queue_and_swapchain();

	create_views();

	create_draw_buffer();

	render_pass();

	create_framebuffers();
	
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
	VkResult res = vkEnumeratePhysicalDevices(vk_instance_, &n_devices, nullptr);
	if (res != VK_SUCCESS) {
		printf("%d", res);
	}
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
void window::logical_devices() {
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

	VkPhysicalDeviceAccelerationStructureFeaturesKHR accelFeature{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR };
	accelFeature.accelerationStructure = VK_TRUE;
	VkPhysicalDeviceRayTracingPipelineFeaturesKHR rtPipelineFeature{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR };
	rtPipelineFeature.rayTracingPipeline = VK_TRUE;

	VkPhysicalDeviceBufferDeviceAddressFeatures bufferDeviceAddressFeatures{};
	bufferDeviceAddressFeatures.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
	bufferDeviceAddressFeatures.bufferDeviceAddress = VK_TRUE;
	bufferDeviceAddressFeatures.pNext = &rtPipelineFeature;

	accelFeature.pNext = &bufferDeviceAddressFeatures;

	VkPhysicalDeviceFeatures2 deviceFeatures2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
	deviceFeatures2.pNext = &accelFeature;
	printf("Crasheo aqui \n");
	vkGetPhysicalDeviceFeatures2(vk_physical_device_, &deviceFeatures2);
	if (!accelFeature.accelerationStructure || !rtPipelineFeature.rayTracingPipeline) {
		throw std::runtime_error("Ray tracing features are not supported on this device.");
	}

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
		VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
		VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME,
		VK_KHR_BUFFER_DEVICE_ADDRESS_EXTENSION_NAME,
		VK_EXT_DESCRIPTOR_INDEXING_EXTENSION_NAME,
		VK_KHR_SPIRV_1_4_EXTENSION_NAME,
		VK_KHR_SHADER_FLOAT_CONTROLS_EXTENSION_NAME
		// ... otras extensiones que puedas necesitar
	};

	VkDeviceCreateInfo createInfo = {};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();
	createInfo.pEnabledFeatures = NULL;  // Establecer en NULL
	createInfo.pNext = &deviceFeatures2; // Encadenar las características de ray tracing

	// Creación del dispositivo Vulkan
	if (vkCreateDevice(vk_physical_device_, &createInfo, nullptr, &vk_device_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create logical device!");
	}

	VkBool32 surfaceSupport = VK_FALSE;
	vkGetPhysicalDeviceSurfaceSupportKHR(vk_physical_device_, indices.graphicsFamily.value(), surface_, &surfaceSupport);

	if (surfaceSupport != VK_TRUE) {
		throw std::runtime_error("El dispositivo físico no es compatible con la superficie de ventana.");
	}

	// Creación del cargador de Vulkan para raytracing
	//vk_loader_.init();
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
		if (availableFormat.format == VK_FORMAT_R8G8B8A8_UNORM &&
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


	swapchain_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	/*uint32_t queueFamilyIndices[] = {
		indices.graphicsFamily.value(),
		indices.presentFamily.value()
	};*/

	uint32_t queueFamilyIndices[2] = {
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
		uint32_t queueFamilyIndex = indices.graphicsFamily.value(); // Suponiendo que 'indices' es tu estructura de QueueFamilyIndices y que ya has encontrado las familias de colas
		swapchain_info.queueFamilyIndexCount = 1;
		swapchain_info.pQueueFamilyIndices = &queueFamilyIndex;
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

	obtain_swap_images();
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
	// Retrieve the count of swap chain images
	uint32_t imageCount;
	vkGetSwapchainImagesKHR(vk_device_, swapChain, &imageCount, nullptr);

	// Resize vectors to hold the swap chain images and image views
	swap_chain_images.resize(imageCount);
	vk_image_views.resize(imageCount);

	// Retrieve the actual images from the swap chain
	vkGetSwapchainImagesKHR(vk_device_, swapChain, &imageCount, swap_chain_images.data());

	// Loop through each swap chain image to create a corresponding image view
	for (size_t i = 0; i < imageCount; i++) {
		VkImageViewCreateInfo viewInfo{};

		// Set the image format to match the swap chain format
		viewInfo.format = swapchain_info.imageFormat;

		// Define the type of the Vulkan structure being used
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;

		// Associate this image view with an actual image from the swap chain
		viewInfo.image = swap_chain_images.at(i);

		// Specify that this image view is for a 2D image
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;

		// Identity mapping for color channel swizzling
		viewInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		viewInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		// Define the aspects of the image this view will access
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

		// Mipmap settings: we use only one level
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;

		// Array layer settings: only one layer
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		// Create the image view and store it in vk_image_views
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

/*
void window::create_buffers()
{
	buffers_.resize(swap_chain_images.size());
	uniforms_buffers_memory_.resize(swap_chain_images.size());

	buffer_Info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_Info.size = sizeof(Vertex);
	buffer_Info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	buffer_Info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	for (size_t i = 0; i < swap_chain_images.size(); i++) {

		if (vkCreateBuffer(vk_device_, &buffer_Info, nullptr, &buffers_.at(i)) != VK_SUCCESS) {
			throw std::runtime_error("failed to create Uniform Buffer!");
		}
		allocate_memory_for_buffers(&buffers_.at(i), &uniforms_buffers_memory_.at(i), &vertex_, sizeof(vertex_)); // Before test anything fill ubo_ with some data, like camera matrix
	}

}*/

void window::create_framebuffers()
{
	framebuffers_.resize(swap_chain_images.size());

	for (size_t i = 0; i < swap_chain_images.size(); i++) {
		VkImageView attachments[] = {
			vk_image_views[i] 
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass = render_pass_; 
		framebufferInfo.attachmentCount = 1; 
		framebufferInfo.pAttachments = attachments;
		framebufferInfo.width = swapchain_info.imageExtent.width;
		framebufferInfo.height = swapchain_info.imageExtent.height;
		framebufferInfo.layers = 1;


		if (vkCreateFramebuffer(vk_device_, &framebufferInfo, nullptr, &framebuffers_.at(i)) != VK_SUCCESS) {
			throw std::runtime_error("failed to create framebuffer!");
		}
	}
}


void window::create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(vk_device_, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(vk_device_, buffer, &memRequirements);

	VkMemoryAllocateFlagsInfo allocateFlagsInfo = {};
	if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
		allocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
		allocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
	}

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
	if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT) {
		allocInfo.pNext = &allocateFlagsInfo;
	}
	if (vkAllocateMemory(vk_device_, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(vk_device_, buffer, bufferMemory, 0);
}

VkDeviceAddress window::getBufferDeviceAddress(VkDevice device, VkBuffer buffer)
{
	VkBufferDeviceAddressInfo bufferDeviceAI{};
	bufferDeviceAI.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	bufferDeviceAI.buffer = buffer;
	return PpfnVkGetBufferDeviceAddressKHR(device, &bufferDeviceAI);
}

void window::create_draw_buffer()
{
	VkImageCreateInfo imageInfo{};
	imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType = VK_IMAGE_TYPE_2D;
	imageInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
	imageInfo.extent.width = width_;  // Reemplaza con el ancho real de tu ventana
	imageInfo.extent.height = height_; // Reemplaza con el alto real de tu ventana
	imageInfo.extent.depth = 1;
	imageInfo.mipLevels = 1;
	imageInfo.arrayLayers = 1;
	imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageInfo.usage = VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

	vkCreateImage(vk_device_, &imageInfo, nullptr, &draw_image_buffer_);

	// 2. Asignar Memoria para el VkImage
	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(vk_device_, draw_image_buffer_, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = // Necesitas encontrar el tipo de memoria adecuado aquí


	vkAllocateMemory(vk_device_, &allocInfo, nullptr, &draw_image_buffer_memory_);

	vkBindImageMemory(vk_device_, draw_image_buffer_, draw_image_buffer_memory_, 0);

	// 3. Crear un VkImageView para el Buffer de Imagen
	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image = draw_image_buffer_;
	viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format = imageInfo.format;
	viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel = 0;
	viewInfo.subresourceRange.levelCount = 1;
	viewInfo.subresourceRange.baseArrayLayer = 0;
	viewInfo.subresourceRange.layerCount = 1;

	
	vkCreateImageView(vk_device_, &viewInfo, nullptr, &draw_buffer_image_view_);
}



void window::create_TLAS(render* render_) {
	// Create a VkAccelerationStructureBuildGeometryInfoKHR for the TLAS
	TLASGeometry = {};
	TLASGeometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	TLASGeometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
	TLASGeometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
	TLASGeometry.geometry.instances.arrayOfPointers = VK_FALSE;
	TLASGeometry.pNext = nullptr;

	// Structure to hold TLAS build info
	VkAccelerationStructureBuildGeometryInfoKHR_info_ = {};
	VkAccelerationStructureBuildGeometryInfoKHR_info_.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	VkAccelerationStructureBuildGeometryInfoKHR_info_.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
	VkAccelerationStructureBuildGeometryInfoKHR_info_.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	VkAccelerationStructureBuildGeometryInfoKHR_info_.geometryCount = 1;
	VkAccelerationStructureBuildGeometryInfoKHR_info_.pGeometries = &TLASGeometry;
	VkAccelerationStructureBuildGeometryInfoKHR_info_.pNext = nullptr;
	VkAccelerationStructureBuildGeometryInfoKHR_info_.srcAccelerationStructure = VK_NULL_HANDLE;


	// Create the scratch buffer
	// Scrath buffer
	VkAccelerationStructureBuildSizesInfoKHR sizeInfo{};
	sizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
	uint32_t maxPrimitiveCounts = scene_objects_.size();

	pfnVkGetAccelerationStructureBuildSizesKHR(
		vk_device_,
		VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&VkAccelerationStructureBuildGeometryInfoKHR_info_,
		&maxPrimitiveCounts,
		&sizeInfo
	);
	VkBufferCreateInfo scratchBufferInfo{};
	scratchBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	scratchBufferInfo.size = sizeInfo.buildScratchSize;
	scratchBufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	scratchBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(vk_device_, &scratchBufferInfo, nullptr, &scratch_buffer_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create scratch buffer!");
	}

	VkMemoryRequirements scratchMemoryRequirements;
	vkGetBufferMemoryRequirements(vk_device_, scratch_buffer_, &scratchMemoryRequirements);

	VkMemoryAllocateInfo scratchAllocInfo{};
	scratchAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	scratchAllocInfo.allocationSize = scratchMemoryRequirements.size;
	scratchAllocInfo.memoryTypeIndex = findMemoryType(scratchMemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	VkMemoryAllocateFlagsInfo allocateFlagsInfo = {};
	allocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	allocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
	scratchAllocInfo.pNext = &allocateFlagsInfo;
	if (vkAllocateMemory(vk_device_, &scratchAllocInfo, nullptr, &scratch_buffer_memory_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate scratch buffer memory!");
	}

	if (vkBindBufferMemory(vk_device_, scratch_buffer_, scratch_buffer_memory_, 0) != VK_SUCCESS) {
		throw std::runtime_error("Failed to bind scratch buffer memory!");
	}


	// Store the instances for the BLAS that will be used in the TLAS
	vk_acceleration_structure_instances_.resize(scene_objects_.size());

	// Prepare the BLAS instances for inclusion in the TLAS
	for (size_t i = 0; i < scene_objects_.size(); ++i) {
		auto& obj = scene_objects_[i];

		// Reference to the current instance to be updated
		VkAccelerationStructureInstanceKHR& vkInstance = vk_acceleration_structure_instances_[i];

		// Convert glm::mat4 to VkTransformMatrixKHR by transposing it to match Vulkan's layout
		glm::mat4 transform = obj.get_matrix(); // Esto obtiene tu glm::mat4
		glm::vec4 col0 = transform[0];
		glm::vec4 col1 = transform[1];
		glm::vec4 col2 = transform[2];

		glm::mat4x3 transposedMatrix;
		transposedMatrix[0] = glm::vec3(transform[0]);
		transposedMatrix[1] = glm::vec3(transform[1]);
		transposedMatrix[2] = glm::vec3(transform[2]);
		transposedMatrix[3] = glm::vec3(transform[3]);
		glm::mat4x3 transposed = glm::transpose(transposedMatrix);
		memcpy(&vkInstance.transform, &transposed, sizeof(VkTransformMatrixKHR));

		// Fill the remaining fields for the instance
		vkInstance.instanceCustomIndex = obj.blas_id_; // A unique identifier for this instance.
		vkInstance.mask = 0xFF; // The visibility mask for the instance.
		vkInstance.instanceShaderBindingTableRecordOffset = 0; // The offset into the shader binding table.
		vkInstance.flags = VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR; // Instance flags.
		vkInstance.accelerationStructureReference = obj.getBLASDeviceAddress(vk_device_); // The BLAS reference.
		vk_acceleration_structure_instances_.at(i) = (vkInstance);

	}

	// Create the buffer for the TLAS instances
	VkDeviceSize instanceBufferSize = sizeof(VkAccelerationStructureInstanceKHR) * vk_acceleration_structure_instances_.size();

	create_buffer(
		instanceBufferSize,
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT | VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
		instancesBuffer,
		instancesBufferMemory
	);

	// Copy the BLAS instance data into the TLAS buffer
	void* data;
	vkMapMemory(vk_device_, instancesBufferMemory, 0, instanceBufferSize, 0, &data);
	memcpy(data, vk_acceleration_structure_instances_.data(), (size_t)instanceBufferSize);
	vkUnmapMemory(vk_device_, instancesBufferMemory);



	// Get the build sizes for the TLAS
	uint32_t instanceCount = static_cast<uint32_t>(vk_acceleration_structure_instances_.size());
	VkAccelerationStructureBuildSizesInfoKHR buildSizesInfo{};
	buildSizesInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
	buildSizesInfo.pNext = nullptr;
	pfnVkGetAccelerationStructureBuildSizesKHR(
		vk_device_,
		VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&VkAccelerationStructureBuildGeometryInfoKHR_info_,
		&instanceCount,
		&buildSizesInfo
	);

	// Create the buffer and memory for the TLAS

	create_buffer(
		buildSizesInfo.accelerationStructureSize,
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR,
		VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT,
		TLAS_buffer_,
		TLAS_memory_buffer_
	);

	// Create the acceleration structure for the TLAS
	VkAccelerationStructureCreateInfoKHR accelerationStructureCreate{};
	accelerationStructureCreate.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	accelerationStructureCreate.buffer = TLAS_buffer_;
	accelerationStructureCreate.size = buildSizesInfo.accelerationStructureSize;
	accelerationStructureCreate.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;

	// Initialize the TLAS
	
	if (pfnVkCreateAccelerationStructureKHR(vk_device_, &accelerationStructureCreate, nullptr, &TLAS_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create TLAS!");
	}
	VkAccelerationStructureBuildGeometryInfoKHR_info_.dstAccelerationStructure = TLAS_;
	TLASGeometry.geometry.instances.data.deviceAddress = getBufferDeviceAddress(vk_device_, instancesBuffer);

	VkAccelerationStructureBuildGeometryInfoKHR_info_.scratchData.deviceAddress = getBufferDeviceAddress(vk_device_, scratch_buffer_);


	tlasBuildRangeInfo = {};
	tlasBuildRangeInfo.primitiveCount = instanceCount;
	// Build the TLAS using the provided command buffer

	// I moved this piece of the function to the record_command_buffer function inside render class because it needs to be done when we are recording a buffer
	/*for (auto& obj : scene_objects_) {
		VkAccelerationStructureBuildGeometryInfoKHR tlasBuildInfo = VkAccelerationStructureBuildGeometryInfoKHR_info_;
		tlasBuildInfo.dstAccelerationStructure = TLAS_;
		tlasBuildInfo.scratchData.deviceAddress = getBufferDeviceAddress(vk_device_, obj.scratch_buffer_); // Assuming scratchBuffer has already been created.

		// Define the range of geometries to build
		VkAccelerationStructureBuildRangeInfoKHR tlasBuildRangeInfo{};
		tlasBuildRangeInfo.primitiveCount = instanceCount;

		// Assume you're building the TLAS on the GPU and have already set up a commandBuffer
		VkAccelerationStructureBuildRangeInfoKHR* pTlasBuildRangeInfo = &tlasBuildRangeInfo;
		pfnVkCmdBuildAccelerationStructuresKHR(
			render_->command_buffers.at(0), // The VkCommandBuffer that is recording the commands.
			1, // The number of acceleration structures to build.
			&tlasBuildInfo, // Build information for the TLAS.
			&pTlasBuildRangeInfo // Range information for the TLAS build.
		);
	}*/

	
}



uint32_t window::give_blas_id()
{
	if (blas_id_ == 0) {
		return blas_id_;
	}
	else {
		blas_id_++;
		return blas_id_;
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

void window::set_camera_data(camera* c)
{
	cam_position = c->position;
	cam_projectionMatrix = c->projectionMatrix;
	cam_viewMatrix = c->viewMatrix;
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
