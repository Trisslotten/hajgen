#pragma once

#include "shader.hpp"
#include "timer.hpp"
#include <glm\glm.hpp>
#include "FastNoise.h"
#include "camera.hpp"

const glm::vec3 HEIGHTMAP_SIZE(2048.f, 1000.f, 2048.f);
const size_t HEIGHTMAP_RESOLUTION = 128;
const int HEIGHTMAP_MAX_ITERATIONS = 2 * HEIGHTMAP_RESOLUTION * HEIGHTMAP_RESOLUTION;

class Heightmap
{
	GLuint heightmapTex;


	glm::vec3 size;
	float frequency;
	float scale;
	size_t resolution;
	int seed;
	glm::ivec2 pos;

	float* noisemap;
	float* smoothTemp;
	uint16_t* heightmap;


	void generate();
	void smoothen();
	void addDetail();


	float heightAt(int x, int y);

	glm::vec3 normalAt(glm::vec2 pos);

	void setHeightAt(int x, int y, float height);
	void addHeightAt(int x, int y, float height);


	struct
	{
		FastNoise mountains;
		FastNoise fields;
		FastNoise biome;

		FastNoise detail;
	} ns;
	float fieldNoise(float x, float y);
	float mountainNoise(float x, float y);

	void configureNoises();
	float getBiome(float x, float y);
	float getNoise(float x, float y);

public:

	Heightmap(glm::vec2 _pos);
	
	~Heightmap();


	float heightAt(glm::vec3 pos);
	float heightAt(glm::vec2 pos);
	glm::vec2 gradientAt(glm::vec2 pos);

	void setHeightAt(glm::vec2 pos, float height);
	void addHeightAt(glm::vec2 pos, float height);


	void upload();
	void bind(ShaderProgram& shader);


	auto getSize()
	{
		return size;
	}
	auto getPos()
	{
		return pos;
	}
};