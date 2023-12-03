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
	PpfnVkGetBufferDeviceAddressKHR = (PFN_vkGetBufferDeviceAddressKHR)vkGetDeviceProcAddr(window_->vk_device_, "vkGetBufferDeviceAddressKHR");
	
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
	VkDeviceAddress baseAddress = getBufferDeviceAddress(window_->vk_device_, vertex_buffer_);
	VkDeviceAddress vertexAddress = baseAddress;
	VkDeviceAddress indexAddress = vertexAddress + sizeof(vertex_.vertices[0]) * vertex_.vertices.size();


	VkAccelerationStructureGeometryKHR geometry{};
	geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
	geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
	geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
	geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT; 
	geometry.geometry.triangles.vertexData.deviceAddress = vertexAddress;
	geometry.geometry.triangles.vertexStride = sizeof(Vertex); 
	geometry.geometry.triangles.maxVertex = vertex_.vertices.size(); 
	geometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32; 
	geometry.geometry.triangles.indexData.deviceAddress = indexAddress;
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

	VkMemoryAllocateFlagsInfo allocateFlagsInfo = {};
	allocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	allocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
	scratchAllocInfo.pNext = &allocateFlagsInfo;
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
	/*VkDeviceSize vertex_bufferSize = sizeof(vertex_.vertices[0]) * vertex_.vertices.size();
	createBuffer(vertex_bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, vertices_buffer_, vertices_buffer_memory_);
	uploadDataToBuffer(vertex_.vertices.data(), vertices_buffer_memory_, vertex_bufferSize);

	// Crear y configurar el buffer de índices
	VkDeviceSize index_bufferSize = sizeof(indices_[0]) * indices_.size();
	createBuffer(index_bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, index_buffer_, index_buffer_memory_);
	uploadDataToBuffer(indices_.data(), index_buffer_memory_, index_bufferSize);

	// Crear y configurar el buffer de normales
	VkDeviceSize normal_bufferSize = sizeof(vertex_.normals[0]) * vertex_.normals.size();
	createBuffer(normal_bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, normal_buffer_, normal_buffer_memory_);
	uploadDataToBuffer(vertex_.normals.data(), normal_buffer_memory_, normal_bufferSize);

	// Crear y configurar el buffer de colores (si es necesario)
	VkDeviceSize color_bufferSize = sizeof(vertex_.color);
	createBuffer(color_bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, color_buffer_, color_buffer_memory_);
	uploadDataToBuffer(&vertex_.color, color_buffer_memory_, color_bufferSize);
	
	// Crear y configurar el buffer para la matriz del modelo
	VkDeviceSize model_bufferSize = sizeof(glm::mat4);
	createBuffer(model_bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, model_buffer_, model_buffer_memory_);
	uploadDataToBuffer(&transform_, model_buffer_memory_, model_bufferSize);*/

	VkDeviceSize vertex_buffer_size = sizeof(Vertex) * vertex_.vertices.size() +  sizeof(glm::mat4) + sizeof(uint32_t) * indices_.size();
	createBuffer(vertex_buffer_size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
		VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | 
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,

		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VK_MEMORY_PROPERTY_HOST_COHERENT_BIT , 
		vertex_buffer_, vertex_buffer_memory_);

	uploadAllDataToSingleBuffer(
		vertex_.vertices.data(), sizeof(vertex_.vertices[0]) * vertex_.vertices.size(), // Datos y tamaño de los vértices
		indices_.data(), sizeof(indices_[0]) * indices_.size(), // Datos y tamaño de los índices
		vertex_.normals.data(), sizeof(vertex_.normals[0]) * vertex_.normals.size(), // Datos y tamaño de las normales
		&vertex_.color, sizeof(vertex_.color), // Datos y tamaño del color
		&transform_, sizeof(glm::mat4), // Datos y tamaño de la matriz de transformación
		vertex_buffer_memory_ // Memoria del buffer combinado
	);

}

void object::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = size;
	bufferInfo.usage = usage;
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(window_->vk_device_, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(window_->vk_device_, buffer, &memRequirements);

	VkMemoryAllocateFlagsInfo allocateFlagsInfo = {};
	allocateFlagsInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
	allocateFlagsInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(window_->vk_physical_device_, memRequirements.memoryTypeBits, properties);
	allocInfo.pNext = &allocateFlagsInfo;

	if (vkAllocateMemory(window_->vk_device_, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate buffer memory!");
	}

	vkBindBufferMemory(window_->vk_device_, buffer, bufferMemory, 0);
}

void object::uploadDataToBuffer(const void* data, VkDeviceMemory bufferMemory, VkDeviceSize size)
{
	void* mappedData;
	vkMapMemory(window_->vk_device_, bufferMemory, 0, size, 0, &mappedData);
	memcpy(mappedData, data, static_cast<size_t>(size));
	vkUnmapMemory(window_->vk_device_, bufferMemory);
}

void object::update_model_matrix_buffer()
{
	void* data;
	vkMapMemory(window_->vk_device_, model_buffer_memory_, 0, sizeof(glm::mat4), 0, &data);
	memcpy(data, &transform_, sizeof(glm::mat4));
	vkUnmapMemory(window_->vk_device_, model_buffer_memory_);
}

void object::uploadAllDataToSingleBuffer(const void* verticesData, VkDeviceSize verticesSize,
	const void* normalsData, VkDeviceSize normalsSize,
	const void* colorData, VkDeviceSize colorSize,
	const void* modelData, VkDeviceSize modelSize,
	const void* indicesData, VkDeviceSize indicesSize,
	VkDeviceMemory bufferMemory)
{
	// Mapea la memoria una sola vez
	void* mappedData;
	vkMapMemory(window_->vk_device_, bufferMemory, 0, verticesSize + normalsSize + colorSize + modelSize + indicesSize, 0, &mappedData);

	// Copia los datos de los vértices
	memcpy(mappedData, verticesData, static_cast<size_t>(verticesSize));

	// Copia los datos de las normales (teniendo en cuenta el offset)
	memcpy(static_cast<char*>(mappedData) + verticesSize, normalsData, static_cast<size_t>(normalsSize));

	// Copia los datos de los colores (teniendo en cuenta los offsets anteriores)
	memcpy(static_cast<char*>(mappedData) + verticesSize + normalsSize, colorData, static_cast<size_t>(colorSize));

	// Copia los datos de la matriz modelo (teniendo en cuenta los offsets anteriores)
	memcpy(static_cast<char*>(mappedData) + verticesSize + normalsSize + colorSize, modelData, static_cast<size_t>(modelSize));

	// Copia los datos de los índices (teniendo en cuenta todos los offsets anteriores)
	memcpy(static_cast<char*>(mappedData) + verticesSize + normalsSize + colorSize + modelSize, indicesData, static_cast<size_t>(indicesSize));

	// Desmapea la memoria
	vkUnmapMemory(window_->vk_device_, bufferMemory);
}

uint32_t object::findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	// Primero, intentar encontrar un tipo de memoria que sea DEVICE_LOCAL y cumpla con los requisitos básicos.
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) &&
			(memProperties.memoryTypes[i].propertyFlags & (properties | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) == (properties | VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)) {
			return i;
		}
	}

	// Si no se encuentra, buscar un tipo de memoria que simplemente cumpla con los requisitos básicos.
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
	return PpfnVkGetBufferDeviceAddressKHR(device, &bufferDeviceAI);
}

VkDeviceAddress object::getBLASDeviceAddress(VkDevice d)
{
	VkAccelerationStructureDeviceAddressInfoKHR addressInfo{};
	addressInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
	addressInfo.accelerationStructure = acceleration_structure_; // Asegúrate de que 'acceleration_structure_' sea la BLAS de este objeto

	return pfnVkGetAccelerationStructureDeviceAddressKHR(d, &addressInfo);
}

