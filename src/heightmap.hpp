#pragma once

#include "shader.hpp"
#include "timer.hpp"
#include <glm\glm.hpp>

class Heightmap
{
	GLuint heightmapTex;

	glm::vec3 scale;

	GLuint quad_vao;
	GLuint quad_vbo;

	ShaderProgram shader;

	size_t size;

	float* noisemap;
	float* noisemap2;
	uint16_t* heightmap;

	Timer timer;

	void dropErodeOnce();

	void upload();
public:

	void generate(); 

	void update();
	void draw();

};




