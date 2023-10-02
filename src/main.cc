#include "glfw3.h"
#include "vulkan.h"
#include <memory>
#include <iostream>
#include "../include/window.h"

int main() {
	
	std::unique_ptr<window> window_ = std::make_unique<window>();


	while (!glfwWindowShouldClose(window_->get_window())) {
		glfwPollEvents();
	}

	window_->end();
	return 0;
}