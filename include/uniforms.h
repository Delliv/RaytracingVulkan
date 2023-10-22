#include <glm.hpp>


#ifndef uniforms_
#define uniforms_ 
struct UniformBufferObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 proj;
};
#endif