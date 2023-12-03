#version 460 core
#extension GL_EXT_ray_tracing : enable

layout(binding = 4, rgba8) uniform writeonly image2D resultadoImagen;

void main() {
    //ivec2 pixelCoord = ivec2(gl_LaunchIDEXT.xy);
    //imageStore(resultadoImagen, pixelCoord, vec4(1.0, 0.0, 0.0, 1.0)); // Almacenar un co;
}