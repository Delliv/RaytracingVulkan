#include "../include/object.h"

object::object()
{
	transform_ = glm::mat4(1.0);

	vertex_.vertices = {
		// Front face
		glm::vec3(-0.5f, -0.5f, 0.5f),
		glm::vec3(0.5f, -0.5f, 0.5f),
		glm::vec3(0.5f, 0.5f, 0.5f),
		glm::vec3(-0.5f, 0.5f, 0.5f),
		// Back face
		glm::vec3(-0.5f, -0.5f, -0.5f),
		glm::vec3(0.5f, -0.5f, -0.5f),
		glm::vec3(0.5f, 0.5f, -0.5f),
		glm::vec3(-0.5f, 0.5f, -0.5f),
		// Up face
		glm::vec3(-0.5f, 0.5f, -0.5f),
		glm::vec3(0.5f, 0.5f, -0.5f),
		glm::vec3(0.5f, 0.5f, 0.5f),
		glm::vec3(-0.5f, 0.5f, 0.5f),
		// Down face
		glm::vec3(-0.5f, -0.5f, -0.5f),
		glm::vec3(0.5f, -0.5f, -0.5f),
		glm::vec3(0.5f, -0.5f, 0.5f),
		glm::vec3(-0.5f, -0.5f, 0.5f),
		// Right face
		glm::vec3(0.5f, -0.5f, -0.5f),
		glm::vec3(0.5f, 0.5f, -0.5f),
		glm::vec3(0.5f, 0.5f, 0.5f),
		glm::vec3(0.5f, -0.5f, 0.5f),
		// Left face
		glm::vec3(-0.5f, -0.5f, -0.5f),
		glm::vec3(-0.5f, 0.5f, -0.5f),
		glm::vec3(-0.5f, 0.5f, 0.5f),
		glm::vec3(-0.5f, -0.5f, 0.5f),
	};

	vertex_.normals = {
		// Front face
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		// Back face
		glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(0.0f, 0.0f, -1.0f),
		// Top face
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		// Down face
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(0.0f, -1.0f, 0.0f),
		// Right face
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		// Left face
		glm::vec3(-1.0f, 0.0f, 0.0f),
		glm::vec3(-1.0f, 0.0f, 0.0f),
		glm::vec3(-1.0f, 0.0f, 0.0f),
		glm::vec3(-1.0f, 0.0f, 0.0f),
	};

	vertex_.color = { 1.0f, 1.0f, 1.0f };

	indices_ = {
		 0, 1, 2, 2, 3, 0,       // Front
		 4, 5, 6, 6, 7, 4,       // Back
		 8, 9, 10, 10, 11, 8,    // Top
		 12, 13, 14, 14, 15, 12, // Down
		 16, 17, 18, 18, 19, 16, // Right
		 20, 21, 22, 22, 23, 20  // Left
	};

}

object::object(window* w)
{
	window_ = w;
	blas_id_ = window_->give_blas_id();
	
	pfnVkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(window_->vk_device_, "vkCreateAccelerationStructureKHR"));
	if (!pfnVkCreateAccelerationStructureKHR) {
		throw std::runtime_error("Failed to load vkCreateAccelerationStructureKHR");
	}
	pfnVkDestroyAccelerationStructureKHR = (PFN_vkDestroyAccelerationStructureKHR)vkGetDeviceProcAddr(window_->vk_device_, "vkDestroyAccelerationStructureKHR");
	pfnVkGetAccelerationStructureDeviceAddressKHR = (PFN_vkGetAccelerationStructureDeviceAddressKHR)vkGetDeviceProcAddr(window_->vk_device_, "vkGetAccelerationStructureDeviceAddressKHR");
	pfnVkGetAccelerationStructureBuildSizesKHR = (PFN_vkGetAccelerationStructureBuildSizesKHR)vkGetDeviceProcAddr(window_->vk_device_, "vkGetAccelerationStructureBuildSizesKHR");

	transform_ = glm::mat4(1.0);

	vertex_.vertices = {
		// Front face
		glm::vec3(-0.5f, -0.5f, 0.5f),
		glm::vec3(0.5f, -0.5f, 0.5f),
		glm::vec3(0.5f, 0.5f, 0.5f),
		glm::vec3(-0.5f, 0.5f, 0.5f),
		// Back face
		glm::vec3(-0.5f, -0.5f, -0.5f),
		glm::vec3(0.5f, -0.5f, -0.5f),
		glm::vec3(0.5f, 0.5f, -0.5f),
		glm::vec3(-0.5f, 0.5f, -0.5f),
		// Up face
		glm::vec3(-0.5f, 0.5f, -0.5f),
		glm::vec3(0.5f, 0.5f, -0.5f),
		glm::vec3(0.5f, 0.5f, 0.5f),
		glm::vec3(-0.5f, 0.5f, 0.5f),
		// Down face
		glm::vec3(-0.5f, -0.5f, -0.5f),
		glm::vec3(0.5f, -0.5f, -0.5f),
		glm::vec3(0.5f, -0.5f, 0.5f),
		glm::vec3(-0.5f, -0.5f, 0.5f),
		// Right face
		glm::vec3(0.5f, -0.5f, -0.5f),
		glm::vec3(0.5f, 0.5f, -0.5f),
		glm::vec3(0.5f, 0.5f, 0.5f),
		glm::vec3(0.5f, -0.5f, 0.5f),
		// Left face
		glm::vec3(-0.5f, -0.5f, -0.5f),
		glm::vec3(-0.5f, 0.5f, -0.5f),
		glm::vec3(-0.5f, 0.5f, 0.5f),
		glm::vec3(-0.5f, -0.5f, 0.5f),
	};

	vertex_.normals = {
		// Front face
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		glm::vec3(0.0f, 0.0f, 1.0f),
		// Back face
		glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(0.0f, 0.0f, -1.0f),
		glm::vec3(0.0f, 0.0f, -1.0f),
		// Top face
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 1.0f, 0.0f),
		// Down face
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec3(0.0f, -1.0f, 0.0f),
		// Right face
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		glm::vec3(1.0f, 0.0f, 0.0f),
		// Left face
		glm::vec3(-1.0f, 0.0f, 0.0f),
		glm::vec3(-1.0f, 0.0f, 0.0f),
		glm::vec3(-1.0f, 0.0f, 0.0f),
		glm::vec3(-1.0f, 0.0f, 0.0f),
	};

	vertex_.color = { 1.0f, 1.0f, 1.0f };

	indices_ = {
		 0, 1, 2, 2, 3, 0,       // Front
		 4, 5, 6, 6, 7, 4,       // Back
		 8, 9, 10, 10, 11, 8,    // Top
		 12, 13, 14, 14, 15, 12, // Down
		 16, 17, 18, 18, 19, 16, // Right
		 20, 21, 22, 22, 23, 20  // Left
	};
}

object::~object()
{
	if (acceleration_structure_ != VK_NULL_HANDLE) {
		pfnVkDestroyAccelerationStructureKHR(window_->vk_device_, acceleration_structure_, nullptr);
		acceleration_structure_ = VK_NULL_HANDLE;
	}

	if (blas_buffer_ != VK_NULL_HANDLE) {
		vkDestroyBuffer(window_->vk_device_, blas_buffer_, nullptr);
		blas_buffer_ = VK_NULL_HANDLE;
	}

	if (blas_buffer_memory_ != VK_NULL_HANDLE) {
		vkFreeMemory(window_->vk_device_, blas_buffer_memory_, nullptr);
		blas_buffer_memory_ = VK_NULL_HANDLE;
	}
}

void object::translate(const glm::vec3& offset)
{
	transform_ = glm::translate(glm::mat4(1.0f), offset) * transform_;
}

void object::rotate(float angle, const glm::vec3& axis)
{
	transform_ = glm::rotate(glm::mat4(1.0f), glm::radians(angle), axis) * transform_;
}

void object::scale(const glm::vec3& scale)
{
	transform_ = glm::scale(glm::mat4(1.0f), scale) * transform_;
}

void object::create_BLAS()
{
	VkAccelerationStructureGeometryKHR geometry{};
	geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
	geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
	geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT; 
	geometry.geometry.triangles.vertexData.deviceAddress = getBufferDeviceAddress(window_->vk_device_,vertex_buffer_);
	geometry.geometry.triangles.vertexStride = sizeof(Vertex); 
	geometry.geometry.triangles.maxVertex = vertex_.vertices.size(); 
	geometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32; 
	geometry.geometry.triangles.indexData.deviceAddress = getBufferDeviceAddress(window_->vk_device_, index_buffer_);
	geometry.geometry.triangles.transformData.deviceAddress = 0; 

	VkAccelerationStructureBuildGeometryInfoKHR buildInfo{};
	buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
	buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
	buildInfo.geometryCount = 1; 
	buildInfo.pGeometries = &geometry; 
	buildInfo.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR; 
	buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR; 
	buildInfo.srcAccelerationStructure = VK_NULL_HANDLE;

	
	VkAccelerationStructureBuildSizesInfoKHR sizeInfo{};
	sizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

	uint32_t maxPrimitiveCounts = vertex_.vertices.size() / 3;

	pfnVkGetAccelerationStructureBuildSizesKHR(
		window_->vk_device_,
		VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		&buildInfo, 
		&maxPrimitiveCounts, 
		&sizeInfo 
	);

	//BLAS buffer;
	VkBufferCreateInfo blas_buffer_info = {};
	blas_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	blas_buffer_info.size = sizeInfo.accelerationStructureSize;
	blas_buffer_info.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
	blas_buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(window_->vk_device_, &blas_buffer_info, nullptr, &blas_buffer_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create BLAS buffer!");
	}

	VkMemoryRequirements blas_memory_requirements;
	vkGetBufferMemoryRequirements(window_->vk_device_, blas_buffer_, &blas_memory_requirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = blas_memory_requirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(window_->vk_physical_device_, blas_memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vkAllocateMemory(window_->vk_device_, &allocInfo, nullptr, &blas_buffer_memory_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate BLAS buffer memory!");
	}
	if (vkBindBufferMemory(window_->vk_device_, blas_buffer_, blas_buffer_memory_, 0) != VK_SUCCESS) {
		throw std::runtime_error("Failed to bind BLAS buffer memory!");
	}

	// Scrath buffer
	VkBufferCreateInfo scratchBufferInfo{};
	scratchBufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	scratchBufferInfo.size = sizeInfo.buildScratchSize; 
	scratchBufferInfo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT; 
	scratchBufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
;
	if (vkCreateBuffer(window_->vk_device_, &scratchBufferInfo, nullptr, &scratch_buffer_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create scratch buffer!");
	}

	VkMemoryRequirements scratchMemoryRequirements;
	vkGetBufferMemoryRequirements(window_->vk_device_, scratch_buffer_, &scratchMemoryRequirements);

	VkMemoryAllocateInfo scratchAllocInfo{};
	scratchAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	scratchAllocInfo.allocationSize = scratchMemoryRequirements.size;
	scratchAllocInfo.memoryTypeIndex = findMemoryType(window_->vk_physical_device_, scratchMemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vkAllocateMemory(window_->vk_device_, &scratchAllocInfo, nullptr, &scratch_buffer_memory_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate scratch buffer memory!");
	}

	if (vkBindBufferMemory(window_->vk_device_, scratch_buffer_, scratch_buffer_memory_, 0) != VK_SUCCESS) {
		throw std::runtime_error("Failed to bind scratch buffer memory!");
	}

	VkDeviceAddress scratchBufferAddress = getBufferDeviceAddress(window_->vk_device_, scratch_buffer_);

	buildInfo.scratchData.deviceAddress = scratchBufferAddress;



	VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo{};
	accelerationStructureCreateInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
	accelerationStructureCreateInfo.buffer = blas_buffer_; // El buffer para la BLAS
	accelerationStructureCreateInfo.size = sizeInfo.accelerationStructureSize;
	accelerationStructureCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;

	if (pfnVkCreateAccelerationStructureKHR(window_->vk_device_, &accelerationStructureCreateInfo, nullptr, &acceleration_structure_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create acceleration structure!");
	}
	/*
	vkDestroyBuffer(window_->vk_device_, scratch_buffer_, nullptr);
	vkFreeMemory(window_->vk_device_, scratch_buffer_memory_, nullptr);

	scratch_buffer_ = VK_NULL_HANDLE;
	scratch_buffer_memory_ = VK_NULL_HANDLE;*/
}

void object::create_buffers()
{
	//Vertex buffer
	VkDeviceSize vertex_bufferSize = sizeof(vertex_.vertices[0]) * vertex_.vertices.size();

	VkBufferCreateInfo vertex_buffer_info = {};
	vertex_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertex_buffer_info.size = vertex_bufferSize;
	vertex_buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	vertex_buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(window_->vk_device_, &vertex_buffer_info, nullptr, &vertex_buffer_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create vertex buffer!");
	}

	VkMemoryRequirements vertex_memory_requirements;
	vkGetBufferMemoryRequirements(window_->vk_device_, vertex_buffer_, &vertex_memory_requirements);

	VkMemoryAllocateInfo vertex_allocInfo = {};
	vertex_allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	vertex_allocInfo.allocationSize = vertex_memory_requirements.size;
	vertex_allocInfo.memoryTypeIndex = findMemoryType(window_->vk_physical_device_, vertex_memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	if (vkAllocateMemory(window_->vk_device_, &vertex_allocInfo, nullptr, &vertex_buffer_memory_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate vertex buffer memory!");
	}

	void* data;
	vkMapMemory(window_->vk_device_, vertex_buffer_memory_, 0, vertex_bufferSize, 0, &data);
	memcpy(data, vertex_.vertices.data(), (size_t)vertex_bufferSize);
	vkUnmapMemory(window_->vk_device_, vertex_buffer_memory_);

	if (vkBindBufferMemory(window_->vk_device_, vertex_buffer_, vertex_buffer_memory_, 0) != VK_SUCCESS) {
		throw std::runtime_error("Failed to bind vertex buffer memory!");
	}

	// Index buffer
	VkBufferCreateInfo index_buffer_info = {};
	index_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	index_buffer_info.size = sizeof(indices_[0]) * indices_.size();
	index_buffer_info.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
	index_buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(window_->vk_device_, &index_buffer_info, nullptr, &index_buffer_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create index buffer!");
	}

	VkMemoryRequirements index_memory_requirements;
	vkGetBufferMemoryRequirements(window_->vk_device_, index_buffer_, &index_memory_requirements);

	VkMemoryAllocateInfo index_alloc_info = {};
	index_alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	index_alloc_info.allocationSize = index_memory_requirements.size;
	index_alloc_info.memoryTypeIndex = findMemoryType(window_->vk_physical_device_, index_memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	if (vkAllocateMemory(window_->vk_device_, &index_alloc_info, nullptr, &index_buffer_memory_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate index buffer memory!");
	}

	void* index_data;
	vkMapMemory(window_->vk_device_, index_buffer_memory_, 0, index_buffer_info.size, 0, &index_data);
	memcpy(index_data, indices_.data(), (size_t)index_buffer_info.size);
	vkUnmapMemory(window_->vk_device_, index_buffer_memory_);

	if (vkBindBufferMemory(window_->vk_device_, index_buffer_, index_buffer_memory_, 0) != VK_SUCCESS) {
		throw std::runtime_error("Failed to bind index buffer memory!");
	}

	// Model buffer
	VkDeviceSize model_bufferSize = sizeof(glm::mat4);

	// Crear el buffer uniforme para la matriz modelo
	VkBufferCreateInfo bufferInfo = {};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = model_bufferSize;
	bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(window_->vk_device_, &bufferInfo, nullptr, &model_buffer_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create model matrix buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(window_->vk_device_, model_buffer_, &memRequirements);

	VkMemoryAllocateInfo model_allocInfo = {};
	model_allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	model_allocInfo.allocationSize = memRequirements.size;
	model_allocInfo.memoryTypeIndex = findMemoryType(window_->vk_physical_device_, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	if (vkAllocateMemory(window_->vk_device_, &model_allocInfo, nullptr, &model_buffer_memory_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate model matrix buffer memory!");
	}

	if (vkBindBufferMemory(window_->vk_device_, model_buffer_, model_buffer_memory_, 0) != VK_SUCCESS) {
		throw std::runtime_error("Failed to bind model matrix buffer memory!");
	}

}

void object::update_model_matrix_buffer()
{
	void* data;
	vkMapMemory(window_->vk_device_, model_buffer_memory_, 0, sizeof(glm::mat4), 0, &data);
	memcpy(data, &transform_, sizeof(glm::mat4));
	vkUnmapMemory(window_->vk_device_, model_buffer_memory_);
}

uint32_t object::findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	throw std::runtime_error("Failed to find suitable memory type!");
}

VkDeviceAddress object::getBufferDeviceAddress(VkDevice device, VkBuffer buffer)
{
	VkBufferDeviceAddressInfo bufferDeviceAI{};
	bufferDeviceAI.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
	bufferDeviceAI.buffer = buffer;
	return vkGetBufferDeviceAddress(device, &bufferDeviceAI);
}

VkDeviceAddress object::getBLASDeviceAddress(VkDevice d)
{
	VkAccelerationStructureDeviceAddressInfoKHR addressInfo{};
	addressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
	addressInfo.accelerationStructure = acceleration_structure_; // Asegúrate de que 'acceleration_structure_' sea la BLAS de este objeto

	return pfnVkGetAccelerationStructureDeviceAddressKHR(d, &addressInfo);
}

