#include "glfw3.h"
#include "vulkan.h"
#include <memory>
#include <iostream>
#include <filesystem>
#include "../include/window.h"
#include "../include/render.h"
#include "../include/object.h"
#include "../include/vulkan_loader.h"
#include "../include/camera.h"

int main() {
	std::cout << "Current path: " << std::filesystem::current_path() << std::endl;
	std::shared_ptr<window> window_ = std::make_unique<window>();
	std::unique_ptr<render> render_ = std::make_unique<render>(window_.get());
	std::unique_ptr<camera> camera_ = std::make_unique<camera>(window_.get());
	

	object obj(window_.get());
	obj.create_buffers();
	obj.create_BLAS();

	window_->scene_objects_.push_back(obj);
	window_->createSpecificDescriptorSetLayouts();
	window_->createDescriptorPool();
	render_->create_command_pool();
	render_->create_command_buffers();

	window_->create_TLAS(render_.get());
	//window_->createDescriptorSets();

	render_->create_pipeline();
	render_->create_SBT_buffer();
	render_->init_fences();
	render_->init_semaphore();
	render_->record_command_buffers();


	while (!glfwWindowShouldClose(window_->get_window())) {
		render_->render_scene();
		glfwPollEvents();
	}

	window_->end();
	return 0;
}