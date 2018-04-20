#pragma once

#include "heightmap.hpp"
#include <queue>
#include <future>
#include <unordered_map>
#include <glm\glm.hpp>


namespace std
{
	template<> struct hash<glm::ivec2>
	{
		size_t operator()(glm::ivec2 const& v) const noexcept
		{
			size_t x = *(unsigned int*)&v.x;
			size_t y = *(unsigned int*)&v.y;
			return x | (y << sizeof(int));
		}
	};
}


class Terrain
{
	GLuint patch_vao;
	GLuint patch_vbo;

	std::unordered_map<glm::ivec2, Heightmap*> maps;

	std::vector<std::future<Heightmap*>> generating;
	std::vector<std::future<glm::ivec2>> eroding;

	std::queue<glm::ivec2> toErode;
	std::queue<glm::ivec2> toGenerate;

	ShaderProgram shader;

	Timer dtimer;
	Camera camera;
public:

	~Terrain();

	void init();

	void update(glm::vec3 pos);

	void draw();
};

