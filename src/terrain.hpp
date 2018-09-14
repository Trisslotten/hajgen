#pragma once

#include "heightmap.hpp"
#include <queue>
#include <future>
#include <unordered_map>
#include <glm\glm.hpp>
#include "texture.hpp"


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

const int numTrees = int(HEIGHTMAP_SIZE.x * HEIGHTMAP_SIZE.z / 250.f);

const int MAX_PATCHES = 64;

class Terrain
{
	GLuint patch_vao;
	GLuint patch_vbo;


	GLuint tree_vao;
	GLuint tree_vbo;
	ShaderProgram treeShader;
	glm::ivec2 treesPos;
	Texture texTree;


	std::unordered_map<glm::ivec2, Heightmap*> maps;

	struct ErodePair
	{
		std::future<void> future;
		glm::ivec2 pos;
	};
	std::vector<ErodePair> eroding;
	std::vector<glm::ivec2> toErode;


	struct CreatePair
	{
		std::future<Heightmap*> future;
		glm::ivec2 pos;
	};
	std::vector<CreatePair> creating;
	std::vector<glm::ivec2> toCreate;

	std::queue<glm::ivec2> toGenerate;

	bool inCreation(glm::ivec2 pos);
	bool alreadyCreated(glm::ivec2 pos);

	bool canErode(glm::ivec2 pos);
	bool canCreate(glm::ivec2 pos);


	std::unordered_map<std::string, Texture> textures;

	ShaderProgram shader;


	Timer dtimer;
	Camera camera;
	glm::ivec2 lastChunk;

	glm::ivec2 getChunkPos(glm::vec3 pos);

	void generateChunk(glm::ivec2 pos);
public:

	~Terrain();

	void init();

	void update(glm::vec3 pos);

	void draw();
};

