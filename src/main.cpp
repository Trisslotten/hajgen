#include <iostream>
#include <GL\glew.h>
#include <glm\gtc\constants.hpp>
#include "window.hpp"
#include "heightmap.hpp"


int main()
{
	Window::open(1280, 720);

	Heightmap hm;
	hm.generate();


	while (!Window::shouldClose())
	{
		hm.update();
		hm.draw();
		Window::update();
	}
	Window::close();

	return 0;
}