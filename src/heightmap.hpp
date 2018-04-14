#pragma once

#include "shader.hpp"
#include "timer.hpp"
#include <glm\glm.hpp>
#include "FastNoise.h"
#include "camera.hpp"

class Heightmap
{
	GLuint heightmapTex;
	GLuint patch_vao;
	GLuint patch_vbo;

	ShaderProgram shader;


	glm::vec3 size;
	float frequency;
	float scale;
	size_t resolution;


	Camera camera;


	float* noisemap;
	float* smoothTemp;
	uint16_t* heightmap;

	uint64_t smoothInterval;

	Timer dtimer;
	Timer uploadTimer;
	Timer gt;
	Timer erodeTimer;

	float* watermap;
	float* sedimentmap;

	void updateWater();

	void dropErodeOnce();
	void smoothen();

	void upload();

	float heightAt(int x, int y);
	float heightAt(glm::vec3 pos);
	float heightAt(glm::vec2 pos);

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
	} ns;
	float fieldNoise(float x, float y);
	float mountainNoise(float x, float y);

	void configureNoises();
	float getBiome(float x, float y);
	float getNoise(float x, float y);

public:

	Heightmap();

	unsigned long iterations = 0;

	void generate(); 

	void update();
	void draw();

};




