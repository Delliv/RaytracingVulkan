#version 460 core
#extension GL_EXT_ray_tracing : enable

struct Vertex {
    vec3 position;
    vec3 color;
    vec3 normal;
};

layout(binding = 0, set = 0) buffer Vertices {
    Vertex vertices[];
};

layout(binding = 1, set = 0) uniform Matrices {
    mat4 modelMatrix;
};

layout(binding = 2, set = 0) uniform accelerationStructureEXT blas;


layout(location = 0) rayPayloadInEXT vec4 payload;

void main() {
    // Aquí puedes acceder a los datos de los vértices y a la matriz del modelo
    // Por ejemplo, para obtener la posición transformada del primer vértice:
    vec3 transformedPosition = (modelMatrix * vec4(vertices[0].position, 1.0)).xyz;

    // Puedes usar la información de los vértices y la BLAS como necesites
    // ...

    // Establece el valor de 'payload' según tus necesidades
    payload = vec4(0.0, 1.0, 0.0, 1.0); // Valor de ejemplo
}

