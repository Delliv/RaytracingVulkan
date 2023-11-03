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
}

void object::create_buffers()
{
	//Vertex buffer
	VkDeviceSize bufferSize = sizeof(vertex_.vertices[0]) * vertex_.vertices.size();

	VkBufferCreateInfo vertex_buffer_info = {};
	vertex_buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	vertex_buffer_info.size = bufferSize; 
	vertex_buffer_info.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
	vertex_buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(window_->vk_device_, &vertex_buffer_info, nullptr, &vertex_buffer_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create vertex buffer!");
	}

	VkMemoryRequirements vertex_memory_requirements;
	vkGetBufferMemoryRequirements(window_->vk_device_, vertex_buffer_, &vertex_memory_requirements);

	VkMemoryAllocateInfo allocInfo = {};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = vertex_memory_requirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(window_->vk_physical_device_, vertex_memory_requirements.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	if (vkAllocateMemory(window_->vk_device_, &allocInfo, nullptr, &vertex_buffer_memory_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate vertex buffer memory!");
	}

	void* data;
	vkMapMemory(window_->vk_device_, vertex_buffer_memory_, 0, bufferSize, 0, &data);
	memcpy(data, vertex_.vertices.data(), (size_t)bufferSize);
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

