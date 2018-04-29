#include "window.hpp"

#include <stdlib.h>
#include <iostream>
#include <unordered_map>
#include <glm/glm.hpp>
#include <string>

namespace
{
	std::unordered_map<GLFWwindow*, glm::vec2 > scrolls;

	void scrollCallback(GLFWwindow* window, double x, double y)
	{
		scrolls[window] += glm::vec2(x, y);
	}

	void resizeCallback(GLFWwindow* window, int width, int height)
	{
		//height = width * 9.0 / 16.0;
		//glfwSetWindowSize(window, width, height);
		glViewport(0, 0, (GLsizei)width, (GLsizei)height);
	}


	GLFWwindow* window = nullptr;
	glm::vec2 scroll;
	Timer frame_timer;
	double frames;
	std::string title;
	glm::vec2 last_mouse_pos;
	glm::dvec2 mouse_pos;

	void pollEvents()
	{
		glfwPollEvents();
	}
	void swapBuffers()
	{
		glfwSwapBuffers(window);
	}
	void updateMousepos()
	{
		last_mouse_pos = mouse_pos;
		glfwGetCursorPos(window, &mouse_pos.x, &mouse_pos.y);
	}
	void updateScrolls()
	{
		scroll = scrolls[window];
		scrolls[window] = glm::vec2(0, 0);
	}
	void updateTitle()
	{
		if (frame_timer.elapsed() >= 0.5)
		{
			double elapsed = frame_timer.restart();
			double fps = frames / elapsed;

			std::string new_title = title + " | fps: " + std::to_string(fps);
			glfwSetWindowTitle(window, new_title.c_str());
			frames = 0;
		}
		frames += 1.0;
	}
}



bool Window::isInitialized()
{
	if (window)
		return true;
	else
		return false;
}
void Window::open(int width, int height)
{
	if (window)
		return;

	if (!glfwInit())
		exit(EXIT_FAILURE);

	title = "hajgen";


	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);

	//glfwWindowHint(GLFW_SAMPLES, 8);

	window = glfwCreateWindow(width, height, title.c_str(), nullptr, nullptr);
	if (!window)
	{
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
	glfwMakeContextCurrent(window);
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		std::cout << "glewInit failed: " << std::string((const char*)glewGetErrorString(err)) << "\n";
		std::cin.ignore();
		exit(1);
	}

	scrolls[window] = glm::vec2(0, 0);
	glfwSetScrollCallback(window, scrollCallback);
	glfwSetWindowSizeCallback(window, resizeCallback);

	glfwSwapInterval(1);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);



	glfwMaximizeWindow(window);
}

void Window::showCursor(bool val)
{
	if (val)
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	else
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
}

void Window::close()
{
	glfwTerminate();
}

GLFWwindow * Window::getGLFWWindow()
{
	return window;
}

void Window::update()
{
	updateTitle();

	updateScrolls();

	updateMousepos();

	swapBuffers();

	pollEvents();
}

bool Window::shouldClose()
{
	if (!window)
		return true;
	return glfwWindowShouldClose(window);
}

bool Window::mouseButtonDown(int button)
{
	return glfwGetMouseButton(window, button) == GLFW_PRESS;
}

bool Window::keyDown(int key)
{
	return GLFW_PRESS == glfwGetKey(window, key);
}

glm::vec2 Window::mouseMovement()
{
	glm::vec2 pos = mousePosition();
	glm::vec2 result = pos - last_mouse_pos;
	return result;
}

glm::vec2 Window::mousePosition()
{
	return glm::vec2(mouse_pos);
}

glm::vec2 Window::size()
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(window, &width, &height);
	return glm::vec2(width, height);
}

void Window::setTitle(const std::string & _title)
{
	title = _title;
}

/*
void Window::open() {
if (window)
return;
if (!glfwInit()) {
std::cout << "failed to initiate GLFW" << std::endl;
std::cin.ignore();
exit(EXIT_FAILURE);
}
title = "Flyplane";

auto display = glfwGetPrimaryMonitor();
if (!display) {
std::cerr << "No primary monitor found" << std::endl;
std::cin.ignore();
exit(-1);
}
const GLFWvidmode* mode = glfwGetVideoMode(display);

glfwWindowHint(GLFW_RED_BITS, mode->redBits);
glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

window = glfwCreateWindow(mode->width, mode->height, title.c_str(), display, NULL);
if (!window) {
glfwTerminate();
exit(EXIT_FAILURE);
}
glfwMakeContextCurrent(window);
GLenum err = glewInit();
if (err != GLEW_OK) {
std::cout << "glewInit failed: " << std::string((const char*)glewGetErrorString(err)) << "\n";
std::cin.ignore();
}
scrolls[window] = glm::vec2(0, 0);
glfwSetScrollCallback(window, scrollCallback);

glEnable(GL_DEPTH_TEST);
glEnable(GL_CULL_FACE);
}
*/