#version 460 core
#extension GL_EXT_ray_tracing : enable
/*
struct Vertex {
    vec3 position;
    vec3 normal;
    vec3 color;
};

layout(binding = 0, set = 0) buffer Vertices {
    Vertex vertices[];
};
*/
layout(binding = 1, set = 0) buffer Matrices {
    mat4 modelMatrix;
};

struct RayPayload {
    vec3 rayDir;
};

// Crea una instancia de tu payload
layout(location = 5) rayPayloadInEXT RayPayload RayInformationpayload;
layout(set = 0, binding = 4, rgba8) uniform image2D img;
layout(binding = 2, set = 0) uniform accelerationStructureEXT TLAS;


layout(location = 0) rayPayloadInEXT vec4 payload;
//layout(location = 1) rayPayloadInEXT vec3 hitNormal;
layout(location = 2) rayPayloadInEXT vec3 hitColor;


hitAttributeEXT vec3 hitNormal;
hitAttributeEXT float hitDistance;

void main() {
   vec3 rayOrigin = vec3(0.0f, 0.0f, -0.5f);
   vec3 WorldPosition = rayOrigin + RayInformationpayload.rayDir * hitDistance;

   vec3 worldNormal = normalize(hitNormal);
   vec3 lightPos = vec3(10.0f, 10.0f, 10.0f);
   vec3 lightDir = normalize(lightPos - WorldPosition);
   float lightIntensity = 1.0f;

   float diff = max(dot(worldNormal, lightDir), 0.0);
    vec3 diffuse = diff * vec3(1.0f,1.0f,0.0f) * lightIntensity;

    hitColor = diffuse;
}

