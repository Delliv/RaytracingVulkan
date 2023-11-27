#include "../include/camera.h"
#include "../include/window.h"

camera::camera(window* w)
{
	window_ = w;
	position = glm::vec3(0.0f, 0.0f, 0.5f);
	viewMatrix = glm::lookAt(position, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)); 
	projectionMatrix = glm::perspective(glm::radians(45.0f), (float)(window_->width_ / window_->height_), 0.1f, 100.0f); // Ejemplo: matriz de proyección perspectiva

	create_buffer();
}

camera::~camera()
{
	vkDestroyBuffer(window_->vk_device_, camera_buffer_, nullptr);
	vkFreeMemory(window_->vk_device_, camera_buffer_memory_, nullptr);
}


void camera::create_buffer()
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size = (sizeof(glm::mat4) * 2) + sizeof(glm::vec3);; // Tamaño del buffer igual al de la estructura CameraData
	bufferInfo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT; // Uso como buffer uniforme
	bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Uso exclusivo para un solo queue

	if (vkCreateBuffer(window_->vk_device_, &bufferInfo, nullptr, &camera_buffer_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to create buffer!");
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(window_->vk_device_, camera_buffer_, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize = memRequirements.size;
	allocInfo.memoryTypeIndex = findMemoryType(window_->vk_physical_device_, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, memRequirements.memoryTypeBits);

	if (vkAllocateMemory(window_->vk_device_, &allocInfo, nullptr, &camera_buffer_memory_) != VK_SUCCESS) {
		throw std::runtime_error("Failed to allocate buffer memory!");
	}

	vkBindBufferMemory(window_->vk_device_, camera_buffer_, camera_buffer_memory_, 0);

	void* data;
	VkResult result = vkMapMemory(window_->vk_device_, camera_buffer_memory_, 0, bufferInfo.size, 0, &data);
	memcpy(data, &viewMatrix, sizeof(glm::mat4));
	memcpy(static_cast<char*>(data) + sizeof(glm::mat4), &projectionMatrix, sizeof(glm::mat4));
	memcpy(static_cast<char*>(data) + (sizeof(glm::mat4) * 2), &position, sizeof(glm::vec3));
	vkUnmapMemory(window_->vk_device_, camera_buffer_memory_);
}

uint32_t camera::findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties) {
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(physicalDevice, &memProperties);

	// Si no se encuentra un tipo de memoria DEVICE_LOCAL, buscar un tipo que sea HOST_VISIBLE y HOST_COHERENT.
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
		if ((typeFilter & (1 << i)) &&
			(memProperties.memoryTypes[i].propertyFlags & (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) == (VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)) {
			return i;
		}
	}

	throw std::runtime_error("Failed to find suitable memory type!");
}
