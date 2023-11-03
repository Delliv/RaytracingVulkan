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

	glm::mat4 get_matrix() {
		return transform_;
	}
	uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
	VkBuffer vertex_buffer_;
	VkDeviceMemory vertex_buffer_memory_;
	VkBuffer index_buffer_;
	VkDeviceMemory index_buffer_memory_;

private:
	glm::mat4 transform_;
	Vertex vertex_;
	std::vector<uint32_t> indices_;
	window* window_;
};



#endif object_class
