#include <iostream>
#include <GL\glew.h>
#include <glm\gtc\constants.hpp>
#include "window.hpp"
#include "heightmap.hpp"
#include <algorithm>


int main()
{
	Window::open(1920 - 100, 1080 - 100);

	Heightmap hm;
	hm.generate();

	while (!Window::shouldClose())
	{
		hm.update();
		hm.draw();

		Window::setTitle("iterations: " + std::to_string(hm.iterations));
		Window::update();
	}
	Window::close();

	return 0;
}