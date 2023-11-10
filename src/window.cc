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

	VkPhysicalDeviceAccelerationStructureFeaturesKHR accelFeature{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR };
	VkPhysicalDeviceRayTracingPipelineFeaturesKHR rtPipelineFeature{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR };
	accelFeature.pNext = &rtPipelineFeature;

	VkPhysicalDeviceFeatures2 deviceFeatures2{ VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2 };
	deviceFeatures2.pNext = &accelFeature;

	// Comprobación de soporte para ray tracing
	vkGetPhysicalDeviceFeatures2(vk_physical_device_, &deviceFeatures2);
	if (!accelFeature.accelerationStructure || !rtPipelineFeature.rayTracingPipeline) {
		throw std::runtime_error("Ray tracing features are not supported on this device.");
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	VkDeviceCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
	createInfo.pQueueCreateInfos = queueCreateInfos.data();
	createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

	const std::vector<const char*> deviceExtensions = {
		VK_KHR_SWAPCHAIN_EXTENSION_NAME,
		VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME,
		VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME,
		VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME
	};

	createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	createInfo.ppEnabledExtensionNames = deviceExtensions.data();
	createInfo.pEnabledFeatures = &deviceFeatures;
	createInfo.pNext = &deviceFeatures2; // Añadir las características de ray tracing

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

	VkResult a = vkCreateSwapchainKHR(vk_device_, &swapchain_info, nullptr, &swapChain);
	/*
	if (vkCreateSwapchainKHR(vk_device_, &swapchain_info, nullptr, &swapChain) != VK_SUCCESS) {
		throw std::runtime_error("Error while creating the swap chain.");
	}*/

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

void window::define_descriptors()
{
	/*
	// Binding para la estructura de aceleración (Acceleration Structure)
	VkDescriptorSetLayoutBinding asLayoutBinding{};
	asLayoutBinding.binding = 0;
	asLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	asLayoutBinding.descriptorCount = 1;
	asLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;

	// Binding para un Storage Buffer
	VkDescriptorSetLayoutBinding storageBufferBinding{};
	storageBufferBinding.binding = 1;
	storageBufferBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	storageBufferBinding.descriptorCount = 1;
	storageBufferBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

	// Combinamos los bindings
	std::array<VkDescriptorSetLayoutBinding, 2> bindings = { asLayoutBinding, storageBufferBinding };

	// Información para el layout del set de descriptores
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings = bindings.data();

	if (vkCreateDescriptorSetLayout(vk_device_, &layoutInfo, nullptr, &descriptor_set_layout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor set layout!");
	}

	// Configuración del pool de descriptores
	std::array<VkDescriptorPoolSize, 2> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	poolSizes[0].descriptorCount = 1;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSizes[1].descriptorCount = 1;

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = 1;

	if (vkCreateDescriptorPool(vk_device_, &poolInfo, nullptr, &descriptor_pool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create descriptor pool!");
	}

	// Alocación de sets de descriptores
	layouts.push_back(descriptor_set_layout);
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptor_pool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
	allocInfo.pSetLayouts = layouts.data();

	if (vkAllocateDescriptorSets(vk_device_, &allocInfo, &descriptor_set) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}
	*/
	// Una vez comience a generar las estructuras para la escena debo actualizar los descriptores aqui
}

void window::create_buffers()
{
	buffers_.resize(swap_chain_images.size());
	uniforms_buffers_memory_.resize(swap_chain_images.size());

	buffer_Info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	buffer_Info.size = sizeof(UniformBufferObject);
	buffer_Info.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	buffer_Info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	for (size_t i = 0; i < swap_chain_images.size(); i++) {

		if (vkCreateBuffer(vk_device_, &buffer_Info, nullptr, &buffers_.at(i)) != VK_SUCCESS) {
			throw std::runtime_error("failed to create Uniform Buffer!");
		}
		allocate_memory_for_buffers(&buffers_.at(i), &uniforms_buffers_memory_.at(i), &ubo_, sizeof(ubo_)); // Before test anything fill ubo_ with some data, like camera matrix
	}

}

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

void window::create_command_pool()
{
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	if (indices.graphicsFamily.has_value()) {
		poolInfo.queueFamilyIndex = indices.graphicsFamily.value();
	}
	poolInfo.flags = 0;

	VkCommandPool commandPool;
	if (vkCreateCommandPool(vk_device_, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}
}

void window::create_command_buffers()
{
	command_buffers.resize(swap_chain_images.size());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = command_pool_;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	for (size_t i = 0; i < swap_chain_images.size(); i++) {
		if (vkAllocateCommandBuffers(vk_device_, &allocInfo, &command_buffers.at(i)) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}
}

void window::record_command_buffers()
{

}

void window::createDescriptorSetLayout()
{
	// Layout para la matriz modelo
	VkDescriptorSetLayoutBinding modelMatrixLayoutBinding{};
	modelMatrixLayoutBinding.binding = 0;
	modelMatrixLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	modelMatrixLayoutBinding.descriptorCount = 1;
	modelMatrixLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutCreateInfo layoutInfoModelMatrix{};
	layoutInfoModelMatrix.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfoModelMatrix.bindingCount = 1;
	layoutInfoModelMatrix.pBindings = &modelMatrixLayoutBinding;

	if (vkCreateDescriptorSetLayout(vk_device_, &layoutInfoModelMatrix, nullptr, &descriptorSetLayoutModelMatrix) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor set layout for model matrix!");
	}

	// Layout para la estructura Vertex
	VkDescriptorSetLayoutBinding vertexLayoutBinding{};
	vertexLayoutBinding.binding = 0;
	vertexLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	vertexLayoutBinding.descriptorCount = 1;
	vertexLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

	VkDescriptorSetLayoutCreateInfo layoutInfoVertex{};
	layoutInfoVertex.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfoVertex.bindingCount = 1;
	layoutInfoVertex.pBindings = &vertexLayoutBinding;

	if (vkCreateDescriptorSetLayout(vk_device_, &layoutInfoVertex, nullptr, &descriptorSetLayoutVertex) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor set layout for vertices!");
	}

	// Layout para la BLAS
	VkDescriptorSetLayoutBinding blasLayoutBinding{};
	blasLayoutBinding.binding = 0;
	blasLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	blasLayoutBinding.descriptorCount = 1;
	blasLayoutBinding.stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;

	VkDescriptorSetLayoutCreateInfo layoutInfoBLAS{};
	layoutInfoBLAS.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfoBLAS.bindingCount = 1;
	layoutInfoBLAS.pBindings = &blasLayoutBinding;

	if (vkCreateDescriptorSetLayout(vk_device_, &layoutInfoBLAS, nullptr, &descriptorSetLayoutBLAS) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor set layout for BLAS!");
	}
	
}

void window::createDescriptorPool()
{
	// Define el tamaño del pool basado en la cantidad de descriptores
	std::array<VkDescriptorPoolSize, 2> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount = static_cast<uint32_t>(modelMatrixBuffers.size());
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSizes[1].descriptorCount = static_cast<uint32_t>(vertexBuffers.size());

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = static_cast<uint32_t>(modelMatrixBuffers.size() + vertexBuffers.size());

	if (vkCreateDescriptorPool(vk_device_, &poolInfo, nullptr, &descriptor_pool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor pool!");
	}
}
void window::createDescriptorSets() {
	// Calcular el número total de sets de descriptores necesarios
	size_t totalDescriptorSets = scene_objects_.size() * 3; // 3 sets por objeto (modelo, vértices, BLAS)

	// Crear un vector de layouts con el tamaño calculado
	std::vector<VkDescriptorSetLayout> layouts(totalDescriptorSets);

	// Rellenar el vector con los layouts correspondientes
	for (size_t i = 0; i < scene_objects_.size(); ++i) {
		layouts[i * 3] = descriptorSetLayoutModelMatrix;
		layouts[i * 3 + 1] = descriptorSetLayoutVertex;
		layouts[i * 3 + 2] = descriptorSetLayoutBLAS;
	}

	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptor_pool;
	allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
	allocInfo.pSetLayouts = layouts.data();

	std::vector<VkDescriptorSet> descriptorSets(totalDescriptorSets);
	if (vkAllocateDescriptorSets(vk_device_, &allocInfo, descriptorSets.data()) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate descriptor sets!");
	}

	// Actualiza los descriptor sets con los buffers de cada objeto
	for (size_t i = 0; i < scene_objects_.size(); ++i) {
		auto& obj = scene_objects_[i];

		// Actualizar el descriptor set para la matriz modelo
		VkDescriptorBufferInfo modelMatrixBufferInfo{};
		modelMatrixBufferInfo.buffer = obj->modelMatrixBuffer; // Reemplaza esto con el buffer real de la matriz modelo
		modelMatrixBufferInfo.offset = 0;
		modelMatrixBufferInfo.range = sizeof(glm::mat4);

		VkWriteDescriptorSet descriptorWriteModelMatrix{};
		descriptorWriteModelMatrix.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWriteModelMatrix.dstSet = descriptorSets[i * 3];
		descriptorWriteModelMatrix.dstBinding = 0;
		descriptorWriteModelMatrix.dstArrayElement = 0;
		descriptorWriteModelMatrix.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWriteModelMatrix.descriptorCount = 1;
		descriptorWriteModelMatrix.pBufferInfo = &modelMatrixBufferInfo;

		// Actualizar el descriptor set para los vértices
		VkDescriptorBufferInfo vertexBufferInfo{};
		vertexBufferInfo.buffer = obj->vertexBuffer; // Reemplaza esto con el buffer real de los vértices
		vertexBufferInfo.offset = 0;
		vertexBufferInfo.range = sizeof(Vertex); // Asegúrate de que esto coincida con el tamaño real de tus datos de vértices

		VkWriteDescriptorSet descriptorWriteVertex{};
		descriptorWriteVertex.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWriteVertex.dstSet = descriptorSets[i * 3 + 1];
		descriptorWriteVertex.dstBinding = 0;
		descriptorWriteVertex.dstArrayElement = 0;
		descriptorWriteVertex.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptorWriteVertex.descriptorCount = 1;
		descriptorWriteVertex.pBufferInfo = &vertexBufferInfo;

		// Actualizar el descriptor set para la BLAS
		VkWriteDescriptorSetAccelerationStructureKHR asWrite{};
		asWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
		asWrite.accelerationStructureCount = 1;
		asWrite.pAccelerationStructures = &obj->acceleration_structure_; // Reemplaza esto con el handle real de la BLAS

		VkWriteDescriptorSet descriptorWriteBLAS{};
		descriptorWriteBLAS.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWriteBLAS.dstSet = descriptorSets[i * 3 + 2];
		descriptorWriteBLAS.dstBinding = 0;
		descriptorWriteBLAS.dstArrayElement = 0;
		descriptorWriteBLAS.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		descriptorWriteBLAS.descriptorCount = 1;
		descriptorWriteBLAS.pNext = &asWrite;

		std::array<VkWriteDescriptorSet, 3> writeDescriptorSets = { descriptorWriteModelMatrix, descriptorWriteVertex, descriptorWriteBLAS };
		vkUpdateDescriptorSets(vk_device_, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);
	}
}


void window::updateDescriptorSets()
{
	for (size_t i = 0; i < scene_objects_.size(); ++i) {
		// Actualizar el descriptor set para la matriz modelo
		// (suponiendo que el primer conjunto de descriptorSetsModelMatrix está en el índice 0)
		VkDescriptorBufferInfo modelMatrixBufferInfo{};
		modelMatrixBufferInfo.buffer = scene_objects_[i].modelMatrixBuffer;
		modelMatrixBufferInfo.offset = 0;
		modelMatrixBufferInfo.range = sizeof(glm::mat4);

		VkWriteDescriptorSet descriptorWriteModelMatrix{};
		descriptorWriteModelMatrix.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWriteModelMatrix.dstSet = descriptorSets[i]; // Aquí asumimos que i corresponde al descriptor set correcto
		descriptorWriteModelMatrix.dstBinding = 0;
		descriptorWriteModelMatrix.dstArrayElement = 0;
		descriptorWriteModelMatrix.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWriteModelMatrix.descriptorCount = 1;
		descriptorWriteModelMatrix.pBufferInfo = &modelMatrixBufferInfo;

		// Actualizar el descriptor set para los datos de vértices
		VkDescriptorBufferInfo vertexBufferInfo{};
		vertexBufferInfo.buffer = scene_objects_[i].vertexBuffer;
		vertexBufferInfo.offset = 0;
		vertexBufferInfo.range = VK_WHOLE_SIZE; // O el tamaño específico de tus datos de vértices

		VkWriteDescriptorSet descriptorWriteVertex{};
		descriptorWriteVertex.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWriteVertex.dstSet = descriptorSets[scene_objects_.size() + i]; // Asumiendo que los descriptor sets de vértices siguen a los de matriz modelo
		descriptorWriteVertex.dstBinding = 0;
		descriptorWriteVertex.dstArrayElement = 0;
		descriptorWriteVertex.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		descriptorWriteVertex.descriptorCount = 1;
		descriptorWriteVertex.pBufferInfo = &vertexBufferInfo;


		VkWriteDescriptorSetAccelerationStructureKHR descriptorAccelerationStructureInfo{};
		descriptorAccelerationStructureInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
		descriptorAccelerationStructureInfo.accelerationStructureCount = 1;
		descriptorAccelerationStructureInfo.pAccelerationStructures = &scene_objects_[i].acceleration_structure_;

		VkWriteDescriptorSet descriptorWriteBLAS{};
		descriptorWriteBLAS.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWriteBLAS.dstSet = descriptorSets[2 * scene_objects_.size() + i]; // Asumiendo que los descriptor sets de BLAS siguen a los de matriz modelo y vértices
		descriptorWriteBLAS.dstBinding = 0;
		descriptorWriteBLAS.dstArrayElement = 0;
		descriptorWriteBLAS.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
		descriptorWriteBLAS.descriptorCount = 1;
		descriptorWriteBLAS.pNext = &descriptorAccelerationStructureInfo; // Puntero a la información de la estructura de aceleración

		std::array<VkWriteDescriptorSet, 3> writeDescriptorSets = { descriptorWriteModelMatrix, descriptorWriteVertex, descriptorWriteBLAS };

		vkUpdateDescriptorSets(vk_device_, static_cast<uint32_t>(writeDescriptorSets.size()), writeDescriptorSets.data(), 0, nullptr);

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


