#version 460 core
#extension GL_EXT_ray_tracing : enable

layout(binding = 4, rgba8) uniform writeonly image2D img;
layout(location = 0) rayPayloadInEXT vec4 payload;
void main() {
    payload = vec4(0.0, 1.0, 0.0, 1.0);
}