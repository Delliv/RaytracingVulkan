#version 460
#extension GL_EXT_ray_tracing : enable

layout(set = 0, binding = 1) buffer Matrices {
    mat4 modelMatrix;
} matrices;

layout(set = 0, binding = 3) uniform CameraData {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec3 cameraPosition;
} cameraData;

layout(location = 0) rayPayloadInEXT RayPayload {
    vec4 color;
} payload;

void main() {
    // Color básico para la intersección
    vec3 hitColor = vec3(1.0, 0.0, 0.0); // Rojo, por ejemplo

    // Asigna el color al payload
    payload.color = vec4(hitColor, 1.0);
}
