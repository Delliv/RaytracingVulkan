#ifndef object_class
#define object_class

#include "uniforms.h"
#include "glm.hpp"
#include "vulkan.h"
#include "window.h"
#include "gtc/matrix_transform.hpp"

class object {
public:
	object();
	object(window* w);
	~object();

	void translate(const glm::vec3& offset);
	void rotate(float angulo, const glm::vec3& eje);
	void scale(const glm::vec3& scale);
	void create_BLAS();
	void create_buffers();
	void object::update_model_matrix_buffer();

	glm::mat4 get_matrix() {
		return transform_;
	}
	uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
	VkDeviceAddress getBufferDeviceAddress(VkDevice device, VkBuffer buffer);

	VkDeviceAddress getBLASDeviceAddress(VkDevice d);
	VkBuffer vertex_buffer_;
	VkDeviceMemory vertex_buffer_memory_;
	VkBuffer index_buffer_;
	VkDeviceMemory index_buffer_memory_;
	VkBuffer model_buffer_;
	VkDeviceMemory model_buffer_memory_;

	// BLAS
	VkBuffer blas_buffer_;
	VkDeviceMemory blas_buffer_memory_;

	VkBuffer scratch_buffer_;
	VkDeviceMemory scratch_buffer_memory_;

	VkAccelerationStructureKHR acceleration_structure_;
	uint32_t blas_id_;
private:
	glm::mat4 transform_;
	Vertex vertex_;
	std::vector<uint32_t> indices_;
	window* window_;
};



#endif object_class
