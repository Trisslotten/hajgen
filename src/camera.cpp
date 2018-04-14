#include "camera.hpp"
#include <glm\glm.hpp>
#include "window.hpp"
#include "input.hpp"
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtx\transform.hpp>

void Camera::update(float dt)
{
	// DEBUG FPS CAMERA
	auto dm = Input::mouseMov();
	pitch += dm.y * 0.005f;
	yaw -= dm.x * 0.005f;

	float hpi = glm::half_pi<float>() - 0.001;
	pitch = glm::clamp(pitch, -hpi, hpi);

	float speed = 15.f;
	if (Input::isKeyDown(GLFW_KEY_LEFT_SHIFT))
		speed = 50.f;



	glm::vec3 walk_dir;
	if (Input::isKeyDown(GLFW_KEY_W))
		walk_dir += glm::vec3(0, 0, 1);
	if (Input::isKeyDown(GLFW_KEY_S))
		walk_dir += glm::vec3(0, 0, -1);
	if (Input::isKeyDown(GLFW_KEY_A))
		walk_dir += glm::vec3(1, 0, 0);
	if (Input::isKeyDown(GLFW_KEY_D))
		walk_dir += glm::vec3(-1, 0, 0);

	orientation = glm::quat(glm::vec3(0, yaw, 0))*glm::quat(glm::vec3(pitch, 0, 0));

	walk_dir = orientation * walk_dir;

	position += walk_dir * dt * speed;
	if (Input::isKeyDown(GLFW_KEY_SPACE))
		position.y += speed * dt;
	if (Input::isKeyDown(GLFW_KEY_LEFT_CONTROL))
		position.y -= speed * dt;
}

glm::mat4 Camera::getViewProj()
{
	auto size = Window::size();
	glm::mat4 proj = glm::infinitePerspective(glm::radians(70.f), size.x / size.y, 0.1f);
	glm::mat4 view = glm::inverse(glm::translate(position) * glm::mat4_cast(glm::rotate(orientation, glm::pi<float>(), glm::vec3(0, 1, 0))));
	return proj * view;
}