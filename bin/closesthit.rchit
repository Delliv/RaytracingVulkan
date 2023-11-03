#version 460
#extension GL_NV_ray_tracing : enable

// Definiendo un tamaño máximo para los arrays. 
// Ajusta este número según tus necesidades.
const int MAX_VERTICES = 1000;

layout(set = 0, binding = 2, std430) buffer VertexAttributes {
    vec3 colors[MAX_VERTICES];
    vec3 normals[MAX_VERTICES];
};

layout(location = 0) rayPayloadNV vec3 hitColor;

void main() {
    uint vertexIndex = gl_PrimitiveID * 3 + gl_InstanceCustomIndexNV;  // Asumiendo triángulos
    vec3 vertexColor = colors[vertexIndex];
    vec3 vertexNormal = normals[vertexIndex];

    // Iluminación básica usando el modelo de Lambert
    float lightIntensity = max(dot(vertexNormal, normalize(vec3(1.0, 1.0, -1.0))), 0.0);
    hitColor = vertexColor * lightIntensity;
}

