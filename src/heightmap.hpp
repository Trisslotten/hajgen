#pragma once

#include "shader.hpp"
#include "timer.hpp"
#include <glm\glm.hpp>

class Heightmap
{
	GLuint heightmapTex;

	glm::vec3 scale;

	GLuint patch_vao;
	GLuint patch_vbo;

	ShaderProgram shader;

	size_t size;

	float* noisemap;

	uint16_t* heightmap;

	Timer timer;
	Timer dtimer;

	double erodeTimeAccum = 0;

	void dropErodeOnce();

	void upload();

	float heightAt(int x, int y);

	float heightAt(glm::vec3 pos);
	float heightAt(glm::vec2 pos);

	glm::vec3 normalAt(glm::vec2 pos);

	void addHeightAt(int x, int y, float height);
	void addHeightAt(glm::vec2 pos, float height);


public:

	void generate(); 

	void update();
	void draw();

};




