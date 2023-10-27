#include "../include/render.h"




render::render()
{
}

render::render(window* w)
{
	window_ = w;
}

render::~render()
{
}

void render::create_command_pool()
{
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	if (window_->indices.graphicsFamily.has_value()) {
		poolInfo.queueFamilyIndex = window_->indices.graphicsFamily.value();
	}
	poolInfo.flags = 0;

	VkCommandPool commandPool;
	if (vkCreateCommandPool(window_->vk_device_, &poolInfo, nullptr, &commandPool) != VK_SUCCESS) {
		throw std::runtime_error("failed to create command pool!");
	}
}

void render::create_command_buffers()
{
	command_buffers.resize(window_->swap_chain_images.size());

	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool = command_pool_;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	for (size_t i = 0; i < window_->swap_chain_images.size(); i++) {
		if (vkAllocateCommandBuffers(window_->vk_device_, &allocInfo, &command_buffers.at(i)) != VK_SUCCESS) {
			throw std::runtime_error("failed to allocate command buffers!");
		}
	}
}

void render::record_command_buffers()
{

}

void render::create_pipeline()
{
	// When i have the shaders compiled and saved where they have to
	//auto vertShaderCode = readFile("vertex.spv");
	//auto fragShaderCode = readFile("fragment.spv");
	//fragment_shader_ = createShaderModule(window_->vk_device, fragShaderCode);
	//vertex_shader = createShaderModule(window_->vk_device, vertShaderCode);
}

VkShaderModule render::createShaderModule(VkDevice device, const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	// La especificación Vulkan requiere que el puntero al código del shader sea de uint32_t
	// Esto es seguro siempre que el tamaño del vector sea múltiplo de 4, lo que readFile garantiza
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(device, &createInfo, nullptr, &shaderModule) != VK_SUCCESS) {
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}

std::vector<char> render::readFile(const std::string& filename) {
	std::ifstream file(filename, std::ios::ate | std::ios::binary);

	size_t fileSize = (size_t)file.tellg();
	std::vector<char> buffer(fileSize);
	file.seekg(0);
	file.read(buffer.data(), fileSize);
	file.close();
	return buffer;
}



