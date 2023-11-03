#include "../include/render.h"




render::render()
{
}

render::render(window* w)
{
	window_ = w;
	//vulkanLoader = &window_->vk_loader_;

	pfnVkCreateRayTracingPipelinesKHR = (PFN_vkCreateRayTracingPipelinesKHR)vkGetDeviceProcAddr(window_->vk_device_, "vkCreateRayTracingPipelinesKHR");

	if (!pfnVkCreateRayTracingPipelinesKHR) {
		throw std::runtime_error("Failed to load vkCreateRayTracingPipelinesKHR");
	}
}

render::~render()
{
}

void render::create_command_pool()
{
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	if (window_->indices.graphicsFamily.has_value()) {
		poolInfo.queueFamilyIndex = window_->indices.graphicsFamily.value();
	}
	poolInfo.flags = 0;

	if (vkCreateCommandPool(window_->vk_device_, &poolInfo, nullptr, &command_pool_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}
}

void render::create_command_buffers()
{
	command_buffers.resize(window_->swap_chain_images.size());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = command_pool_;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	for (size_t i = 0; i < window_->swap_chain_images.size(); i++) {
		if (vkAllocateCommandBuffers(window_->vk_device_, &allocInfo, &command_buffers.at(i)) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}
}

void render::record_command_buffers()
{

}

void render::create_pipeline()
{
	// When i have the shaders compiled and saved where they have to
	auto raygen_shader_code = readFile("raygen.spv");
	auto miss_shader_code = readFile("miss.spv");
	auto closest_hit_shader_code = readFile("closesthit.spv");

	raygen = createShaderModule(window_->vk_device_, raygen_shader_code);
	miss = createShaderModule(window_->vk_device_, miss_shader_code);
	closestHit = createShaderModule(window_->vk_device_, closest_hit_shader_code);

	std::array<VkPipelineShaderStageCreateInfo, 3> shaderStages;

	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
	shaderStages[0].module = raygen;
	shaderStages[0].pName = "main";

	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage = VK_SHADER_STAGE_MISS_BIT_KHR;
	shaderStages[1].module = miss;
	shaderStages[1].pName = "main";

	shaderStages[2].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[2].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	shaderStages[2].module = closestHit;
	shaderStages[2].pName = "main";

	std::array<VkRayTracingShaderGroupCreateInfoKHR, 3> shaderGroups;

	// Raygen Shader Group
	shaderGroups[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
	shaderGroups[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
	shaderGroups[0].generalShader = 0; // Índice del shader raygen en shaderStages
	shaderGroups[0].closestHitShader = VK_SHADER_UNUSED_KHR;
	shaderGroups[0].anyHitShader = VK_SHADER_UNUSED_KHR;
	shaderGroups[0].intersectionShader = VK_SHADER_UNUSED_KHR;

	// Closest Hit Shader Group
	shaderGroups[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
	shaderGroups[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
	shaderGroups[1].generalShader = VK_SHADER_UNUSED_KHR;
	shaderGroups[1].closestHitShader = 1; // Índice del shader closest hit en shaderStages
	shaderGroups[1].anyHitShader = VK_SHADER_UNUSED_KHR;
	shaderGroups[1].intersectionShader = VK_SHADER_UNUSED_KHR;

	// Miss Shader Group
	shaderGroups[2].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
	shaderGroups[2].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
	shaderGroups[2].generalShader = 2; // Índice del shader MISS en shaderStages
	shaderGroups[2].closestHitShader = VK_SHADER_UNUSED_KHR;
	shaderGroups[2].anyHitShader = VK_SHADER_UNUSED_KHR;
	shaderGroups[2].intersectionShader = VK_SHADER_UNUSED_KHR;

	//--------------------------------------------------

	//---------------- Input assembly -----------------
	/*VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable = VK_FALSE;*/
	//--------------------------------------------------

	//---------------- Viewport configuration -----------------
	VkViewport viewport{};
	viewport.x = 0.0f;
	viewport.y = 0.0f;
	viewport.width = (float)window_->swapchain_info.imageExtent.width;
	viewport.height = (float)window_->swapchain_info.imageExtent.height;
	viewport.minDepth = 0.0f;
	viewport.maxDepth = 1.0f;

	// Configuración del recorte (scissor)
	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = { window_->swapchain_info.imageExtent.width, window_->swapchain_info.imageExtent.height };

	// Estado del viewport
	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount = 1;
	viewportState.pViewports = &viewport;
	viewportState.scissorCount = 1;
	viewportState.pScissors = &scissor;

	//--------------------------------------------------

	//---------------- Rasterizer configuration -----------------
	/*VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable = VK_FALSE; 
	rasterizer.rasterizerDiscardEnable = VK_FALSE; 
	rasterizer.polygonMode = VK_POLYGON_MODE_FILL; 
	rasterizer.lineWidth = 1.0f; 
	rasterizer.cullMode = VK_CULL_MODE_BACK_BIT; 
	rasterizer.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE; 
	rasterizer.depthBiasEnable = VK_FALSE;
	//--------------------------------------------------
	
	//---------------- Multisampling configuration -----------------
	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable = VK_FALSE; 
	multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;*/
	//--------------------------------------------------

	//---------------- Colorblending configuration -----------------
	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable = VK_FALSE;
	colorBlending.logicOp = VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount = 1;
	colorBlending.pAttachments = &colorBlendAttachment;
	//--------------------------------------------------

	//---------------- Layout configuration -----------------
	VkDescriptorSetLayout setLayouts[] = { window_->descriptor_set_layout };
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = 1;
	pipelineLayoutInfo.pSetLayouts = setLayouts;
	pipelineLayoutInfo.pushConstantRangeCount = 0; 
	pipelineLayoutInfo.pPushConstantRanges = nullptr; 

	
	if (vkCreatePipelineLayout(window_->vk_device_, &pipelineLayoutInfo, nullptr, &pipelineLayout) != VK_SUCCESS) {
		throw std::runtime_error("failed to create pipeline layout!");
	}
	//--------------------------------------------------


	// Pipeline Config
	VkRayTracingPipelineCreateInfoKHR rayPipelineInfo{};
	rayPipelineInfo.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
	rayPipelineInfo.stageCount = static_cast<uint32_t>(shaderStages.size());  // Número de etapas de shader
	rayPipelineInfo.pStages = shaderStages.data();  // Etapas de shader
	rayPipelineInfo.groupCount = static_cast<uint32_t>(shaderGroups.size());  // Número de grupos de shader
	rayPipelineInfo.pGroups = shaderGroups.data();  // Grupos de shader
	rayPipelineInfo.layout = pipelineLayout;  // Layout de la pipeline

	// Crear la pipeline
	
	if (pfnVkCreateRayTracingPipelinesKHR(window_->vk_device_, VK_NULL_HANDLE, VK_NULL_HANDLE, 1, &rayPipelineInfo, nullptr, &pipeline_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create ray tracing pipeline!");
	}

	// No olvides liberar los módulos de sombreado cuando ya no los necesites
	vkDestroyShaderModule(window_->vk_device_, raygen, nullptr);
	vkDestroyShaderModule(window_->vk_device_, closestHit, nullptr);
	vkDestroyShaderModule(window_->vk_device_, miss, nullptr);
}

void render::create_raytracing_buffers()
{

}

VkShaderModule render::createShaderModule(VkDevice device, const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	// La especificación Vulkan requiere que el puntero al código del shader sea de uint32_t
	// Esto es seguro siempre que el tamaño del vector sea múltiplo de 4, lo que readFile garantiza
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

std::vector<char> render::readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();
	return buffer;
}



