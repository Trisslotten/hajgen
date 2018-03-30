#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>
#include "timer.hpp"

namespace Window {

	bool isInitialized();
	void open(int width = 1280, int height = 720);
	//void open();
	void close();
	GLFWwindow *getGLFWWindow();
	void update();

	bool shouldClose();

	bool mouseButtonDown(int button);
	bool keyDown(int key);

	void showCursor(bool val);

	glm::vec2 mouseMovement();
	glm::vec2 mousePosition();
	glm::vec2 size();
};