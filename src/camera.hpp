#pragma once

#include <glm\glm.hpp>
#include <glm\gtc\quaternion.hpp>

struct Camera
{
	glm::vec3 position;
	glm::quat orientation;

	float pitch;
	float yaw;

	void update(float dt);

	glm::mat4 getViewProj();
};