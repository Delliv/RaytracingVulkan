#include "../include/vulkan_loader.h"

VulkanLoader::VulkanLoader()
{
    // Carga dinámica de Vulkan:
    vulkanLib = LoadLibrary(TEXT("vulkan-1.dll"));
   
    if (!vulkanLib) {
        throw std::runtime_error("Failed to load vulkan-1.dll");
    }

}

void VulkanLoader::init()
{
   
   /* pfnVkCreateRayTracingPipelinesKHR = (PFN_vkCreateRayTracingPipelinesKHR)vkGetDeviceProcAddr(, "vkCreateRayTracingPipelinesKHR");
    if (!pfnVkCreateRayTracingPipelinesKHR) {
        throw std::runtime_error("Failed to load vkCreateRayTracingPipelinesKHR");
    }*/
}

PFN_vkCreateRayTracingPipelinesKHR VulkanLoader::getVkCreateRayTracingPipelinesKHR() const
{
    return pfnVkCreateRayTracingPipelinesKHR;
}

VulkanLoader::~VulkanLoader()
{
    if (vulkanLib) {
        FreeLibrary(vulkanLib);
    }
}
