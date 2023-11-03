#ifndef vulkan_loader
#define vulkan_loader

#include "vulkan.h"
#include <Windows.h>
#include <stdexcept>
class VulkanLoader {
public:
    VulkanLoader();
    void init();
    PFN_vkCreateRayTracingPipelinesKHR getVkCreateRayTracingPipelinesKHR() const;

    ~VulkanLoader();

    private:
    HMODULE vulkanLib;
    PFN_vkCreateRayTracingPipelinesKHR pfnVkCreateRayTracingPipelinesKHR;
};

#endif