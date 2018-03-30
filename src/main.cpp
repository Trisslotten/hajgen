#include <iostream>
#include <GL\glew.h>
#include "window.hpp"

#ifdef _DEBUG
#define DPRINT(x) std::cout << x
#else
#define DPRINT(x)
#endif


int main() {
	Window::open();

	while (!Window::shouldClose()) {

		Window::update();
	}

	return 0;

}