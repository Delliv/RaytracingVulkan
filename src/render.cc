#include "../include/render.h"
#include "../include/object.h"
#include "../include/camera.h"


render::render()
{
}

render::render(window* w, camera* c)
{
	window_ = w;
	camera_ = c;
	current_frame_ = 0;
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

	std::array<VkPipelineShaderStageCreateInfo, 3> shaderStages = {};

	shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[0].stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
	shaderStages[0].module = raygen;
	shaderStages[0].pName = "main";
	shaderStages[0].flags = 0;

	shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[1].stage = VK_SHADER_STAGE_MISS_BIT_KHR;
	shaderStages[1].module = miss;
	shaderStages[1].pName = "main";
	shaderStages[2].flags = 0;

	shaderStages[2].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	shaderStages[2].stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	shaderStages[2].module = closestHit;
	shaderStages[2].pName = "main";
	shaderStages[2].flags = 0;

	std::array<VkRayTracingShaderGroupCreateInfoKHR, 3> shaderGroups = {};

	// Raygen Shader Group
	shaderGroups[0].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
	shaderGroups[0].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
	shaderGroups[0].generalShader = 0; // Índice del shader raygen en shaderStages
	shaderGroups[0].closestHitShader = VK_SHADER_UNUSED_KHR;
	shaderGroups[0].anyHitShader = VK_SHADER_UNUSED_KHR;
	shaderGroups[0].intersectionShader = VK_SHADER_UNUSED_KHR;

	// Miss Shader Group
	shaderGroups[1].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
	shaderGroups[1].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
	shaderGroups[1].generalShader = 1; // Índice del shader miss en shaderStages
	shaderGroups[1].closestHitShader = VK_SHADER_UNUSED_KHR;
	shaderGroups[1].anyHitShader = VK_SHADER_UNUSED_KHR;
	shaderGroups[1].intersectionShader = VK_SHADER_UNUSED_KHR;

	// Closest Hit Shader Group
	shaderGroups[2].sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
	shaderGroups[2].type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
	shaderGroups[2].generalShader = VK_SHADER_UNUSED_KHR;
	shaderGroups[2].closestHitShader = 2; // Índice del shader closest hit en shaderStages
	shaderGroups[2].anyHitShader = VK_SHADER_UNUSED_KHR;
	shaderGroups[2].intersectionShader = VK_SHADER_UNUSED_KHR;

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
	VkDescriptorSetLayout setLayouts[] = {descriptor_set_layout ,
										   };
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount = sizeof(setLayouts) / sizeof(setLayouts[0]);
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

	VkMemoryAllocateFlagsInfo memoryAllocateFlagsInfo{};
	memoryAllocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	memoryAllocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

	// Información para la asignación de memoria
	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	allocInfo.pNext = &memoryAllocateFlagsInfo;
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

void render::create_command_buffers()
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

	VkCommandBufferAllocateInfo allocInfo_temp_buffer{};
	allocInfo_temp_buffer.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo_temp_buffer.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo_temp_buffer.commandPool = command_pool_; // Usa el mismo command pool que para tus otros command buffers
	allocInfo_temp_buffer.commandBufferCount = 1;

	if (vkAllocateCommandBuffers(window_->vk_device_, &allocInfo_temp_buffer, &temp_command_buffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate temporary command buffer!");
	}
}


void render::record_command_buffers() {
	//vkWaitForFences(window_->vk_device_, 1, &in_flight_fences_[current_frame_], VK_TRUE, UINT64_MAX);
	//vkResetFences(window_->vk_device_, 1, &in_flight_fences_[current_frame_]);

	for (size_t i = 0; i < command_buffers.size(); i++) {
		VkCommandBufferBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		
		if (vkBeginCommandBuffer(command_buffers[i], &beginInfo) != VK_SUCCESS) {
			throw std::runtime_error("failed to begin recording command buffer!");
		}

		// Verificar si necesitas construir/actualizar la TLAS
		VkBufferDeviceAddressInfo bufferDeviceAI = {};
		bufferDeviceAI.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
		bufferDeviceAI.buffer = window_->scratch_buffer_; // Asegúrate de que scratchBuffer es el búfer de scratch correcto
		VkAccelerationStructureBuildRangeInfoKHR* pTlasBuildRangeInfo = &window_->tlasBuildRangeInfo;
		pfnVkCmdBuildAccelerationStructuresKHR(
			command_buffers[i],
			1, // Número de estructuras para construir
			&window_->VkAccelerationStructureBuildGeometryInfoKHR_info_, // Información de la estructura de aceleración
			&pTlasBuildRangeInfo // Información del rango de construcción
		);
			// Asegúrate de añadir cualquier sincronización necesaria aquí
		

		// Inicio del render pass
		VkRenderPassBeginInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass = window_->render_pass_;
		renderPassInfo.framebuffer = window_->framebuffers_[i];
		renderPassInfo.renderArea.offset = { 0, 0 };
		renderPassInfo.renderArea.extent = window_->swapchain_info.imageExtent;

		VkClearValue clearColor = { {{0.0f, 0.0f, 0.0f, 1.0f}} };
		renderPassInfo.clearValueCount = 1;
		renderPassInfo.pClearValues = &clearColor;

		// Transición de layout de la imagen para ray tracing
		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED; // O el layout actual de la imagen
		barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
		barrier.srcAccessMask = 0; // Dependiendo del uso previo de la imagen
		barrier.dstAccessMask = VK_ACCESS_SHADER_WRITE_BIT; // Si vas a escribir en la imagen desde un shader
		barrier.image = window_->draw_image_buffer_; // La VkImage que estás usando para el ray tracing
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		vkCmdPipelineBarrier(
			command_buffers[i],
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, // O la etapa adecuada según el uso previo
			VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);

		vkCmdBeginRenderPass(command_buffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

		// Configuración y trazado de rayos
		vkCmdBindPipeline(command_buffers[i], VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline_);

		vkCmdBindDescriptorSets(command_buffers[i],
			VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
			pipelineLayout,
			0,
			1,
			&descriptorSets_,
			0,
			nullptr);
		if(i == 0){
			updateDescriptorSets();
		}


		vkCmdEndRenderPass(command_buffers[i]);

		if (i != 0) {
			// Configuración de las regiones del SBT para trazado de rayos
			// Asegúrate de que estas regiones estén configuradas correctamente
			VkBufferDeviceAddressInfo bufferDeviceAI_SBT = {};
			bufferDeviceAI_SBT.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
			bufferDeviceAI_SBT.buffer = SBT_buffer_;
			VkDeviceAddress sbtBufferAddress = vkGetBufferDeviceAddress(window_->vk_device_, &bufferDeviceAI_SBT);
			VkStridedDeviceAddressRegionKHR raygenRegion = {};
			raygenRegion.deviceAddress = sbtBufferAddress;
			raygenRegion.stride = stride_size_;
			raygenRegion.size = stride_size_;
			VkStridedDeviceAddressRegionKHR missRegion = {};
			missRegion.deviceAddress = sbtBufferAddress + 1 * stride_size_;
			missRegion.stride = stride_size_;
			missRegion.size = stride_size_;
			VkStridedDeviceAddressRegionKHR hitRegion = {};
			hitRegion.deviceAddress = sbtBufferAddress + 2 * stride_size_;
			hitRegion.stride = stride_size_;
			hitRegion.size = stride_size_;
			VkStridedDeviceAddressRegionKHR callableRegion = { };

			pfnVkCmdTraceRaysKHR(
				command_buffers[i],
				&raygenRegion,
				&missRegion,
				&hitRegion,
				&callableRegion,
				window_->swapchain_info.imageExtent.width,
				window_->swapchain_info.imageExtent.height,
				1
			);
		}

		if (vkEndCommandBuffer(command_buffers[i]) != VK_SUCCESS) {
			throw std::runtime_error("failed to record command buffer!");
		}
	}
}

void render::render_scene() {
	
	vkWaitForFences(window_->vk_device_, 1, &in_flight_fences_[current_frame_], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(window_->vk_device_, window_->swapChain, UINT64_MAX, imageAvailableSemaphores[current_frame_], VK_NULL_HANDLE, &imageIndex);

	if (in_flight_images_[imageIndex] != VK_NULL_HANDLE) {
		vkWaitForFences(window_->vk_device_, 1, &in_flight_images_[imageIndex], VK_TRUE, UINT64_MAX);
	}

	in_flight_images_[imageIndex] = in_flight_fences_[current_frame_];



	// Iniciar un command buffer temporal para la copia y las transiciones de layout
	vkWaitForFences(window_->vk_device_, 1, &temp_com_buffer_fence_, VK_TRUE, UINT64_MAX);
	vkResetFences(window_->vk_device_, 1, &temp_com_buffer_fence_);
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	vkBeginCommandBuffer(temp_command_buffer, &beginInfo);


	VkImageMemoryBarrier barrierToSrc = {};
	barrierToSrc.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrierToSrc.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED; // Asumir un layout inicial desconocido
	barrierToSrc.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
	barrierToSrc.srcAccessMask = 0; // No hay operaciones pendientes que esperar
	barrierToSrc.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT; // La imagen será leída en una operación de transferencia
	barrierToSrc.image = window_->draw_image_buffer_;
	barrierToSrc.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	barrierToSrc.subresourceRange.baseMipLevel = 0;
	barrierToSrc.subresourceRange.levelCount = 1;
	barrierToSrc.subresourceRange.baseArrayLayer = 0;
	barrierToSrc.subresourceRange.layerCount = 1;

	vkCmdPipelineBarrier(
		temp_command_buffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, // No esperar ninguna etapa específica
		VK_PIPELINE_STAGE_TRANSFER_BIT,    // Barrera antes de la etapa de transferencia
		0,
		0, nullptr,
		0, nullptr,
		1, &barrierToSrc
	);



	// Preparar la imagen del swap chain para la copia
	VkImageMemoryBarrier swapchainImageBarrierToDst = {};
	swapchainImageBarrierToDst.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	swapchainImageBarrierToDst.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	swapchainImageBarrierToDst.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	swapchainImageBarrierToDst.srcAccessMask = 0;
	swapchainImageBarrierToDst.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	swapchainImageBarrierToDst.image = window_->swap_chain_images[imageIndex];
	swapchainImageBarrierToDst.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	swapchainImageBarrierToDst.subresourceRange.baseMipLevel = 0;
	swapchainImageBarrierToDst.subresourceRange.levelCount = 1;
	swapchainImageBarrierToDst.subresourceRange.baseArrayLayer = 0;
	swapchainImageBarrierToDst.subresourceRange.layerCount = 1;

	vkCmdPipelineBarrier(
		temp_command_buffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &swapchainImageBarrierToDst
	);

	// Copiar la imagen del ray tracing a la imagen del swap chain

	VkImageCopy copyRegion = {};
	copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	copyRegion.srcSubresource.mipLevel = 0;
	copyRegion.srcSubresource.baseArrayLayer = 0;
	copyRegion.srcSubresource.layerCount = 1;
	copyRegion.dstSubresource = copyRegion.srcSubresource;
	copyRegion.srcOffset = { 0, 0, 0 };
	copyRegion.dstOffset = { 0, 0, 0 };
	copyRegion.extent.width = window_->swapchain_info.imageExtent.width;
	copyRegion.extent.height = window_->swapchain_info.imageExtent.height;
	copyRegion.extent.depth = 1;

	vkCmdCopyImage(
		temp_command_buffer,
		window_->draw_image_buffer_, // Reemplazar con tu imagen de ray tracing
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		swapchainImageBarrierToDst.image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&copyRegion
	);

	// Preparar la imagen del swap chain para la presentación
	swapchainImageBarrierToDst.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	swapchainImageBarrierToDst.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
	swapchainImageBarrierToDst.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	swapchainImageBarrierToDst.dstAccessMask = 0;
	
	vkCmdPipelineBarrier(
		temp_command_buffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		0,
		0, nullptr,
		0, nullptr,
		1, &swapchainImageBarrierToDst
	);

	vkEndCommandBuffer(temp_command_buffer);

	// Enviar el command buffer de copia
	VkSubmitInfo copySubmitInfo{};
	copySubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	copySubmitInfo.commandBufferCount = 1;
	copySubmitInfo.pCommandBuffers = &temp_command_buffer;
	VkSemaphore copySignalSemaphores[] = { copyFinishedSemaphore };
	copySubmitInfo.signalSemaphoreCount = 1;
	copySubmitInfo.pSignalSemaphores = copySignalSemaphores;

	vkQueueSubmit(window_->graphicsQueue, 1, &copySubmitInfo, temp_com_buffer_fence_);

	VkSemaphore waitSemaphores[] = { imageAvailableSemaphores[current_frame_], copyFinishedSemaphore };
	VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT }; // Ajusta según tus necesidades
	VkSubmitInfo renderSubmitInfo{};
	renderSubmitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
	renderSubmitInfo.waitSemaphoreCount = 2; // Esperando a dos semáforos
	renderSubmitInfo.pWaitSemaphores = waitSemaphores;
	renderSubmitInfo.pWaitDstStageMask = waitStages;
	renderSubmitInfo.commandBufferCount = 1;
	renderSubmitInfo.pCommandBuffers = &command_buffers[imageIndex];
	VkSemaphore renderSignalSemaphores[] = { renderFinishedSemaphores[current_frame_] };
	renderSubmitInfo.signalSemaphoreCount = 1;
	renderSubmitInfo.pSignalSemaphores = renderSignalSemaphores;

	vkWaitForFences(window_->vk_device_, 1, &in_flight_fences_[current_frame_], VK_TRUE, UINT64_MAX);
	vkResetFences(window_->vk_device_, 1, &in_flight_fences_[current_frame_]);
	vkQueueSubmit(window_->graphicsQueue, 1, &renderSubmitInfo, in_flight_fences_[current_frame_]);

	// Continuar con la ejecución y presentación normal
	/*renderSubmitInfo.pWaitSemaphores = waitSemaphores;
	renderSubmitInfo.pWaitDstStageMask = waitStages;
	renderSubmitInfo.commandBufferCount = 1;
	renderSubmitInfo.pCommandBuffers = &command_buffers[imageIndex];
	renderSubmitInfo.pSignalSemaphores = signalSemaphores;

	//vkResetFences(window_->vk_device_, 1, &in_flight_fences_[current_frame_]);

	//vkWaitForFences(window_->vk_device_, 1, &normal_execution_fence_, VK_TRUE, UINT64_MAX);
	//vkResetFences(window_->vk_device_, 1, &normal_execution_fence_);
	if (vkQueueSubmit(window_->graphicsQueue, 1, &renderSubmitInfo, VK_NULL_HANDLE) != VK_SUCCESS) {
		throw std::runtime_error("Failed to submit draw command buffer!");
	}*/

	
	// Presentar el frame
	VkPresentInfoKHR presentInfo{};
	presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount = 1;
	presentInfo.pWaitSemaphores = renderSignalSemaphores; // Espera a que el renderizado termine
	presentInfo.swapchainCount = 1;
	presentInfo.pSwapchains = &window_->swapChain;
	presentInfo.pImageIndices = &imageIndex;

	result = vkQueuePresentKHR(window_->graphicsQueue, &presentInfo);

	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR /* || resizeWindow */) {
		// recreateSwapChain();
	}
	else if (result != VK_SUCCESS) {
		throw std::runtime_error("Failed to present swap chain image!");
	}

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

	if (vkCreateFence(window_->vk_device_, &fenceInfo, nullptr, &temp_com_buffer_fence_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create synchronization objects for a frame!");
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

	if (vkCreateSemaphore(window_->vk_device_, &semaphoreInfo, nullptr, &copyFinishedSemaphore) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create copy finished semaphore!");
	}

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
		if (vkCreateSemaphore(window_->vk_device_, &semaphoreInfo, nullptr, &signalSemaphores[i]) != VK_SUCCESS) {
			throw std::runtime_error("Failed to create render finished semaphore!");
		}
	}
}

void render::createSpecificDescriptorSetLayouts()
{
	// Define seis bindings en el layout de descriptor set
	std::array<VkDescriptorSetLayoutBinding, 5> layoutBindings{};

	// Configuración de cada binding

	// Binding 0: Posiciones de vértices
	layoutBindings[0].binding = 0;
	layoutBindings[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	layoutBindings[0].descriptorCount = 1;
	layoutBindings[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	layoutBindings[0].pImmutableSamplers = nullptr;

	// Binding 1: Matriz modelo
	layoutBindings[1].binding = 1;
	layoutBindings[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	layoutBindings[1].descriptorCount = 1;
	layoutBindings[1].stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	layoutBindings[1].pImmutableSamplers = nullptr;
	// Binding 2: TLAS
	layoutBindings[2].binding = 2;
	layoutBindings[2].descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	layoutBindings[2].descriptorCount = 1;
	layoutBindings[2].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	layoutBindings[2].pImmutableSamplers = nullptr;

	// Binding 3: Datos de la cámara
	layoutBindings[3].binding = 3;
	layoutBindings[3].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	layoutBindings[3].descriptorCount = 1;
	layoutBindings[3].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
	layoutBindings[3].pImmutableSamplers = nullptr;

	// Binding 4: Image to draw color result
	layoutBindings[4].binding = 4;
	layoutBindings[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
	layoutBindings[4].descriptorCount = 1;
	layoutBindings[4].stageFlags = VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
	layoutBindings[4].pImmutableSamplers = nullptr;


	// Crea un único descriptor set layout que incluya todos los bindings
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
	layoutInfo.pBindings = layoutBindings.data();

	if (vkCreateDescriptorSetLayout(window_->vk_device_, &layoutInfo, nullptr, &descriptor_set_layout) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor set layout!");
	}
}

void render::createDescriptorPool()
{
	std::array<VkDescriptorPoolSize, 3> poolSizes{};
	poolSizes[0].type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	poolSizes[0].descriptorCount = 10;
	poolSizes[1].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[1].descriptorCount = 10;
	poolSizes[2].type = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	poolSizes[2].descriptorCount = 1; // Si solo necesitas una estructura de aceleración
	//poolSizes[3].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	//poolSizes[3].descriptorCount = 1; // Si necesitas más storage buffers

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes = poolSizes.data();
	poolInfo.maxSets = 100; // Asumiendo que necesitas 31 sets en total

	if (vkCreateDescriptorPool(window_->vk_device_, &poolInfo, nullptr, &descriptor_pool) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create descriptor pool!");
	}
}

void render::createDescriptorSets()
{
	VkDescriptorSetAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool = descriptor_pool;
	allocInfo.descriptorSetCount = 1;
	allocInfo.pSetLayouts = &descriptor_set_layout;

	// Asignar los descriptor sets
	if (vkAllocateDescriptorSets(window_->vk_device_, &allocInfo, &descriptorSets_) != VK_SUCCESS) {
		throw std::runtime_error("failed to allocate descriptor sets!");
	}
}

void render::updateDescriptorSets()
{
	// Información del buffer de vértices
	VkDescriptorBufferInfo vertexBufferInfo = {};
	vertexBufferInfo.buffer = window_->scene_objects_.at(0).vertex_buffer_;
	vertexBufferInfo.offset = 0;
	vertexBufferInfo.range = sizeof(Vertex) * window_->scene_objects_.at(0).vertex_.vertices.size();

	// Información del buffer de la cámara
	VkDescriptorBufferInfo cameraBufferInfo = {};
	cameraBufferInfo.buffer = camera_->camera_buffer_;
	cameraBufferInfo.offset = 0;
	cameraBufferInfo.range = VK_WHOLE_SIZE;

	// Información del buffer de la matriz modelo
	VkDescriptorBufferInfo modelMatrixBufferInfo = {};
	modelMatrixBufferInfo.buffer = window_->scene_objects_.at(0).vertex_buffer_;
	modelMatrixBufferInfo.offset = sizeof(Vertex) * window_->scene_objects_.at(0).vertex_.vertices.size();
	modelMatrixBufferInfo.range = sizeof(glm::mat4);

	// Información de la TLAS
	VkWriteDescriptorSetAccelerationStructureKHR tlasInfo = {};
	tlasInfo.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
	tlasInfo.accelerationStructureCount = 1;
	tlasInfo.pAccelerationStructures = &window_->TLAS_;

	// Información del buffer de la cámara
	VkDescriptorImageInfo  imageInfo = {};
	imageInfo.imageView = window_->draw_buffer_image_view_;
	imageInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

	std::array<VkWriteDescriptorSet, 5> writeDescriptorSets = {};

	// Descriptor para el buffer de vértices
	writeDescriptorSets[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSets[0].dstSet = descriptorSets_;
	writeDescriptorSets[0].dstBinding = 0; // Asegúrate de que este índice coincida con tu layout
	writeDescriptorSets[0].descriptorCount = 1;
	writeDescriptorSets[0].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	writeDescriptorSets[0].pBufferInfo = &vertexBufferInfo;

	// Descriptor para el buffer de la matriz modelo
	writeDescriptorSets[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSets[1].dstSet = descriptorSets_;
	writeDescriptorSets[1].dstBinding = 1; // Asegúrate de que este índice coincida con tu layout
	writeDescriptorSets[1].descriptorCount = 1;
	writeDescriptorSets[1].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
	writeDescriptorSets[1].pBufferInfo = &modelMatrixBufferInfo;
	// Descriptor para el buffer de la cámara
	writeDescriptorSets[2].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSets[2].dstSet = descriptorSets_;
	writeDescriptorSets[2].dstBinding = 3; // Asegúrate de que este índice coincida con tu layout
	writeDescriptorSets[2].descriptorCount = 1;
	writeDescriptorSets[2].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	writeDescriptorSets[2].pBufferInfo = &cameraBufferInfo;


	// Descriptor para la TLAS
	writeDescriptorSets[3].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSets[3].dstSet = descriptorSets_;
	writeDescriptorSets[3].dstBinding = 2; // Asegúrate de que este índice coincida con tu layout
	writeDescriptorSets[3].descriptorCount = 1;
	writeDescriptorSets[3].descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
	writeDescriptorSets[3].pNext = &tlasInfo;

	// Descriptor para la image view
	writeDescriptorSets[4].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	writeDescriptorSets[4].dstSet = descriptorSets_; 
	writeDescriptorSets[4].dstBinding = 4;
	writeDescriptorSets[4].descriptorCount = 1;
	writeDescriptorSets[4].descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE; 
	writeDescriptorSets[4].pImageInfo = &imageInfo; 

	// Actualizar los descriptor sets
	vkUpdateDescriptorSets(window_->vk_device_, writeDescriptorSets.size(), writeDescriptorSets.data(), 0, nullptr);
}
