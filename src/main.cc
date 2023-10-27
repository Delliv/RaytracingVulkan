#include "glfw3.h"
#include "vulkan.h"
#include <memory>
#include <iostream>
#include "../include/window.h"
#include "../include/render.h"

int main() {
	
	std::unique_ptr<window> window_ = std::make_unique<window>();
	std::unique_ptr<render> render_ = std::make_unique<render>(window_.get());

	while (!glfwWindowShouldClose(window_->get_window())) {
		glfwPollEvents();
	}

	window_->end();
	return 0;
}