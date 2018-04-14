#pragma once

#include <glm\glm.hpp>

namespace Input
{
	bool initialize();
	bool isKeyDown(int key);
	bool isMouseButtonDown(int key);
	bool isButtonPressed(int button);
	bool isKeyPressed(int key);
	void reset();
	glm::vec2 mouseMov();
	glm::vec2 mousePos();

	bool gamepad_present();
	float gamepad_axis(int axis);
	bool gamepad_button_pressed(int button);
};

