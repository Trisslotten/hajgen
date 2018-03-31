#include <iostream>
#include <GL\glew.h>
#include "window.hpp"
#include "heightmap.hpp"


int main()
{
	Window::open(800, 800);

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