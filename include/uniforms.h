#include <glm.hpp>
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

#endif