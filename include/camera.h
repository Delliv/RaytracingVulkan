#ifndef camera_class
#define camera_class

#include "glm.hpp"
#include "vulkan.h"
#include <optional>
#include "gtc/matrix_transform.hpp"

class window;
class render;
class camera {
public:
	camera(window *w);
	~camera();

	//void translate(const glm::vec3& offset);
	//void rotate(float angulo, const glm::vec3& eje);
	//void scale(const glm::vec3& scale);
	
	void create_buffer();
	uint32_t findMemoryType(VkPhysicalDevice physicalDevice, uint32_t typeFilter, VkMemoryPropertyFlags properties);
	friend window;
	friend render;
private:

	glm::vec3 position;
	glm::mat4 viewMatrix;
	glm::mat4 projectionMatrix;

	VkBuffer camera_buffer_;
	VkDeviceMemory camera_buffer_memory_;

	window* window_;
};



#endif object_class
