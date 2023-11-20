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
	pfnVkGetRayTracingShaderGroupHandlesKHR = (PFN_vkGetRayTracingShaderGroupHandlesKHR)vkGetDeviceProcAddr(window_->vk_device_, "vkGetRayTracingShaderGroupHandlesKHR");
	pfnVkCmdBuildAccelerationStructuresKHR = (PFN_vkCmdBuildAccelerationStructuresKHR)vkGetDeviceProcAddr(window_->vk_device_, "vkCmdBuildAccelerationStructuresKHR");
	pfnVkCmdTraceRaysKHR = (PFN_vkCmdTraceRaysKHR)vkGetDeviceProcAddr(window_->vk_device_, "vkCmdTraceRaysKHR");

	VkPhysicalDeviceRayTracingPipelinePropertiesKHR rtProperties = {};
	rtProperties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;

	// Crear la estructura para las propiedades del dispositivo físico
	VkPhysicalDeviceProperties2 deviceProperties2 = {};
	deviceProperties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	deviceProperties2.pNext = &rtProperties;

	// Obtener las propiedades del dispositivo físico
	vkGetPhysicalDeviceProperties2(window_->vk_physical_device_, &deviceProperties2);

	// Ahora puedes acceder a las propiedades específicas de ray tracing
	stride_size_ = rtProperties.shaderGroupBaseAlignment;



}

render::~render()
{
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		vkDestroySemaphore(window_->vk_device_, renderFinishedSemaphores[i], nullptr);
		vkDestroySemaphore(window_->vk_device_, imageAvailableSemaphores[i], nullptr);
		vkDestroyFence(window_->vk_device_, in_flight_fences_[i], nullptr);
	}
}


void render::create_pipeline()
{
	// When i have the shaders compiled and saved where they have to
	auto raygen_shader_code = readFile("../raygen.spv");
	auto miss_shader_code = readFile("../miss.spv");
	auto closest_hit_shader_code = readFile("../closesthit.spv");

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



uint32_t render::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(window_->vk_physical_device_, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("failed to find suitable memory type!");
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
void render::create_SBT_buffer()
{
	// Calcular el tamaño del SBT
	SBT_size_ = stride_size_ * 3; // Tres shaders: raygen, miss y hit

	// Crear el buffer
	VkBufferCreateInfo bufferInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferInfo.size = SBT_size_;
	bufferInfo.usage = VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(window_->vk_device_, &bufferInfo, nullptr, &SBT_buffer_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create SBT buffer!");
	}

	// Obtener los requerimientos de memoria para el buffer
	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(window_->vk_device_, SBT_buffer_, &memRequirements);

	// Información para la asignación de memoria
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	// Asignar memoria
	if (vkAllocateMemory(window_->vk_device_, &allocInfo, nullptr, &SBT_buffer_memory_) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	// Vincular memoria al buffer
	vkBindBufferMemory(window_->vk_device_, SBT_buffer_, SBT_buffer_memory_, 0);

	const uint32_t groupCount = 3; // Raygen, Miss, Hit
	std::vector<uint8_t> shaderHandleStorage(groupCount * stride_size_);
	if (pfnVkGetRayTracingShaderGroupHandlesKHR(window_->vk_device_, pipeline_, 0, groupCount, shaderHandleStorage.size(), shaderHandleStorage.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to get ray tracing shader group handles");
	}

	// Mapear y rellenar el buffer del SBT
	void* mappedData;
	vkMapMemory(window_->vk_device_, SBT_buffer_memory_, 0, SBT_size_, 0, &mappedData);

	// Copiar las direcciones de los shaders al buffer del SBT
	memcpy((char*)mappedData + 0 * stride_size_, shaderHandleStorage.data() + 0 * stride_size_, stride_size_); // Raygen
	memcpy((char*)mappedData + 1 * stride_size_, shaderHandleStorage.data() + 1 * stride_size_, stride_size_); // Miss
	memcpy((char*)mappedData + 2 * stride_size_, shaderHandleStorage.data() + 2 * stride_size_, stride_size_); // Hit

	vkUnmapMemory(window_->vk_device_, SBT_buffer_memory_);
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


void render::create_command_pool()
{
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	if (window_->indices.graphicsFamily.has_value()) {
		poolInfo.queueFamilyIndex = window_->indices.graphicsFamily.value();
	}
	if (vkCreateCommandPool(window_->vk_device_, &poolInfo, nullptr, &command_pool_) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}

}


void render::record_command_buffers()
{
	command_buffers.resize(window_->swap_chain_images.size());
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = command_pool_;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = static_cast<uint32_t>(command_buffers.size());

	if (vkAllocateCommandBuffers(window_->vk_device_, &allocInfo, command_buffers.data()) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate command buffers!");
	}


	for (size_t i = 0; i < command_buffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

		if (vkBeginCommandBuffer(command_buffers[i], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}


		// Aquí asumimos que ya tienes preparada la información de construcción de la TLAS
		VkAccelerationStructureBuildGeometryInfoKHR tlasBuildInfo = window_->VkAccelerationStructureBuildGeometryInfoKHR_info_;
		// Configura tlasBuildInfo según lo que hiciste en create_TLAS()
		uint32_t instanceCount = static_cast<uint32_t>(window_->blas_instances_.size());
		VkAccelerationStructureBuildRangeInfoKHR tlasRangeInfo{};
		tlasRangeInfo.primitiveCount = instanceCount;  // El número de instancias
		tlasRangeInfo.primitiveOffset = 0;             // Offset de la primera instancia
		tlasRangeInfo.firstVertex = 0;                 // Usualmente 0 para TLAS
		tlasRangeInfo.transformOffset = 0;             // Offset para una matriz de transformación, si se usa

		// Grabar comando para construir la TLAS
		VkAccelerationStructureBuildRangeInfoKHR* pTlasBuildRangeInfo = &tlasRangeInfo;
		pfnVkCmdBuildAccelerationStructuresKHR(
			command_buffers[i],
			1, // Número de estructuras para construir
			&tlasBuildInfo, // Información de la estructura de aceleración
			&pTlasBuildRangeInfo // Información del rango de construcción
		);

		// Comandos para configurar y realizar el trazado de rayos
	   // Bind del pipeline de ray tracing
		vkCmdBindPipeline(command_buffers[i], VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline_);

		for (int j = 0; j < window_->descriptorSets_.size(); j++) {
			// Bind de sets de descriptores (asegúrate de haberlos configurado previamente)
			vkCmdBindDescriptorSets(command_buffers[i],
				VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
				pipelineLayout,
				0,
				1,
				&window_->descriptorSets_.at(j),
				0,
				nullptr);
		}

		// Inicio del render pass
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = window_->render_pass_; // Asegúrate de tener esta referencia en window
		renderPassInfo.framebuffer = window_->framebuffers_[i]; // Usa el framebuffer adecuado
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = window_->swapchain_info.imageExtent; // Usa el extent de tu swap chain

		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} }; // Color de fondo
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		vkCmdBeginRenderPass(command_buffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Trazado de rayos
		VkBufferDeviceAddressInfo bufferDeviceAI{};
		bufferDeviceAI.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		bufferDeviceAI.buffer = SBT_buffer_;  // SBT_buffer es tu VkBuffer
		VkDeviceAddress SBR_address = vkGetBufferDeviceAddress(window_->vk_device_, &bufferDeviceAI);
		VkStridedDeviceAddressRegionKHR raygenRegion = { };
		raygenRegion.deviceAddress = SBR_address + 0 * stride_size_;
		raygenRegion.stride = stride_size_;
		raygenRegion.size = stride_size_;
		VkStridedDeviceAddressRegionKHR missRegion = {  };
		missRegion.deviceAddress = SBR_address + 1 * stride_size_;
		missRegion.size = stride_size_;
		missRegion.stride = stride_size_;
		VkStridedDeviceAddressRegionKHR hitRegion = {  };
		hitRegion.deviceAddress = SBR_address + 2 * stride_size_;
		hitRegion.size = stride_size_;
		hitRegion.stride = stride_size_;
		VkStridedDeviceAddressRegionKHR callableRegion = {/* ... */ };
		pfnVkCmdTraceRaysKHR(
			command_buffers[i],
			&raygenRegion,
			&missRegion,
			&hitRegion,
			&callableRegion,
			window_->swapchain_info.imageExtent.width,  // Ancho del framebuffer
			window_->swapchain_info.imageExtent.height, // Altura del framebuffer
			1       // Profundidad
		);

		vkCmdEndRenderPass(command_buffers[i]);

		if (vkEndCommandBuffer(command_buffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}

void render::render_scene()
{
	vkWaitForFences(window_->vk_device_, 1, &in_flight_fences_[current_frame_], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(window_->vk_device_, window_->swapChain, UINT64_MAX, imageAvailableSemaphores[current_frame_], VK_NULL_HANDLE, &imageIndex);

	if (in_flight_images_[imageIndex] != VK_NULL_HANDLE) {
		vkWaitForFences(window_->vk_device_, 1, &in_flight_images_[imageIndex], VK_TRUE, UINT64_MAX);
	}

	in_flight_images_[imageIndex] = in_flight_fences_[current_frame_];

	window_->updateDescriptorSets();

	record_command_buffers();

	// Enviar el command buffer a la cola para su ejecución
	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[current_frame_] };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
	submitInfo.waitSemaphoreCount = 1;
	submitInfo.pWaitSemaphores = waitSemaphores;
	submitInfo.pWaitDstStageMask = waitStages;
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &command_buffers[imageIndex];

	VkSemaphore signalSemaphores[] = { renderFinishedSemaphores[current_frame_] };
	submitInfo.signalSemaphoreCount = 1;
	submitInfo.pSignalSemaphores = signalSemaphores;

	vkResetFences(window_->vk_device_, 1, &in_flight_fences_[current_frame_]);

	if (vkQueueSubmit(window_->graphicsQueue, 1, &submitInfo, in_flight_fences_[current_frame_]) != VK_SUCCESS) {
		throw std::runtime_error("Failed to submit draw command buffer!");
	}

	// Presentar el frame
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = signalSemaphores;

	VkSwapchainKHR swapChains[] = { window_->swapChain };
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = swapChains;
	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(window_->graphicsQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR /* || resizeWindow */) {
		// recreateSwapChain();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to present swap chain image!");
	}

	// Prepararse para el siguiente frame
	current_frame_ = (current_frame_ + 1) % MAX_FRAMES_IN_FLIGHT;
}

void render::init_fences()
{
	in_flight_fences_.resize(MAX_FRAMES_IN_FLIGHT);
	in_flight_images_.resize(window_->swap_chain_images.size(), VK_NULL_HANDLE);

	VkFenceCreateInfo fenceInfo = {};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateFence(window_->vk_device_, &fenceInfo, nullptr, &in_flight_fences_[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create synchronization objects for a frame!");
		}
	}


}

void render::init_semaphore()
{

	imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo = {};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(window_->vk_device_, &semaphoreInfo, nullptr, &imageAvailableSemaphores[i]) != VK_SUCCESS ||
			vkCreateSemaphore(window_->vk_device_, &semaphoreInfo, nullptr, &renderFinishedSemaphores[i]) != VK_SUCCESS) {

			throw std::runtime_error("failed to create synchronization objects for a frame!");
		}
	}
}
