#include <glm.hpp>
#include <vulkan.h>
#include <vector>

#ifndef uniforms_
#define uniforms_ 
struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;

};

struct Vertex {
    std::vector<glm::vec3> vertices;;
    glm::vec3 color;
    std::vector<glm::vec3> normals;
    glm::uvec2 blasHandle;
};

struct BLASInstance {
    uint32_t id_;
    glm::mat4x4 transform_;

    uint32_t mask_;
    uint32_t intance_shader_binding_table_record_offset_;
    VkGeometryInstanceFlagsKHR flags_;
    VkDeviceAddress accelerationStructureReference;
};

#endif