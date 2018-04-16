#include <iostream>
#include <GL\glew.h>
#include <glm\gtc\constants.hpp>
#include "window.hpp"
#include "terrain.hpp"
#include <algorithm>


int main()
{
	Window::open(1920 - 100, 1080 - 100);

	Terrain terrain;
	terrain.init();

	while (!Window::shouldClose())
	{
		terrain.update(glm::vec3());

		terrain.draw();

		Window::update();
	}
	Window::close();

	return 0;
}