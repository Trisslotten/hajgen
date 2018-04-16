#pragma once

#include "shader.hpp"
#include "timer.hpp"
#include <glm\glm.hpp>
#include "FastNoise.h"
#include "camera.hpp"


class Heightmap
{
	GLuint heightmapTex;


	glm::vec3 size;
	float frequency;
	float scale;
	size_t resolution;
	int seed;
	glm::vec2 pos;
	uint64_t maxIterations;
	uint64_t smoothInterval;
	uint64_t iterations = 0;


	float* noisemap;
	float* smoothTemp;
	uint16_t* heightmap;


	void generate();
	void iterate();
	void dropErodeOnce();
	void smoothen();
	void addDetail();


	float heightAt(int x, int y);

	glm::vec3 normalAt(glm::vec2 pos);

	void addHeightAt(int x, int y, float height);
	void addHeightAt(glm::vec2 pos, float height);
	void addHeightAt(glm::vec2 pos, float radius, float volume);

	glm::vec2 gradientAt(glm::vec2 pos);

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

	void upload();

	float heightAt(glm::vec3 pos);
	float heightAt(glm::vec2 pos);
	
	void bind(ShaderProgram& shader);

	auto getSize()
	{
		return size;
	}
};

Heightmap* generateHeightmap(glm::vec2 pos);



