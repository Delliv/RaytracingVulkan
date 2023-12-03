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

layout(binding = 1, set = 0) buffer Matrices {
    mat4 modelMatrix;
};

layout(binding = 2, set = 0) uniform accelerationStructureEXT TLAS;


layout(location = 0) rayPayloadInEXT vec4 payload;
layout(location = 1) rayPayloadInEXT vec3 hitNormal;
layout(location = 2) rayPayloadInEXT vec3 hitColor;
*/
layout(binding = 4, rgba8) uniform writeonly image2D resultadoImagen;

void main() {
    /*vec3 normal = normalize((modelMatrix * vec4(hitNormal, 0.0)).xyz);
    float intensity = dot(normal, -gl_WorldRayDirectionEXT);
    intensity = max(intensity, 0.0);
    //vec3 transformedPosition = (modelMatrix * vec4(vertices[0].position, 1.0)).xyz;


    // Establece el valor de 'payload' según tus necesidades
    //payload = vec4(hitColor * intensity, 1.0); // Valor de ejemplo
    
    ivec2 pixelCoord = ivec2(gl_LaunchIDEXT.xy);
    imageStore(resultadoImagen, pixelCoord, vec4(1.0, 0.0, 0.0, 1.0)); // Almacenar un co;*/
}

