#version 460
#extension GL_NV_ray_tracing : enable

layout(location = 0) rayPayloadNV vec3 hitColor;

void main() {
    hitColor = vec3(0.5, 0.5, 0.5);  // Color de fondo, por ejemplo un gris claro
}
