#pragma once

#include "heightmap.hpp"
#include <queue>
#include <future>

class Terrain
{
	GLuint patch_vao;
	GLuint patch_vbo;


	std::vector<Heightmap*> heightmaps;


	std::vector<std::future<Heightmap*>> generating;
	std::queue<glm::vec2> toGenerate;

	ShaderProgram shader;

	Timer dtimer;
	Camera camera;
public:

	~Terrain();

	void init();

	void update(glm::vec3 pos);

	void draw();
};

