#pragma once

#include "shader.hpp"
#include "timer.hpp"
#include <glm\glm.hpp>
#include "FastNoise.h"
#include "camera.hpp"

const glm::vec3 HEIGHTMAP_SIZE(4000.f, 5000.f, 4000.f);
const size_t HEIGHTMAP_RESOLUTION = 1024;
const int HEIGHTMAP_MAX_ITERATIONS = 0 * HEIGHTMAP_RESOLUTION * HEIGHTMAP_RESOLUTION;

size_t index(int x, int y, int size);


class Image
{
	unsigned int width = 0, height = 0;
	std::vector<unsigned char> image;
public:
	Image(const std::string& path);
	glm::vec4 colorAt(int x, int y);
	glm::vec4 colorAt(glm::vec2 pixel);
};

class Heightmap
{
	GLuint heightmapTex;


	glm::vec3 size;
	float frequency;
	float scale;
	size_t resolution;
	int seed;
	glm::ivec2 pos;

	
	float maxHeight;
	float minHeight;
	float* noisemap;
	float* smoothTemp;
	uint16_t* heightmap;



	void generate();



	glm::vec3 normalAt(glm::vec2 pos);



	struct
	{
		FastNoise mountains;
		FastNoise mountains2;
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
	bool eroded = false;

	Heightmap(glm::vec2 _pos);
	
	~Heightmap();


	float heightAt(int x, int y);
	float heightAt(glm::vec3 pos);
	float heightAt(glm::vec2 pos);

	void setHeightAt(int x, int y, float height);
	void setHeightAt(glm::vec2 pos, float height);

	void addHeightAt(int x, int y, float height);
	void addHeightAt(glm::vec2 pos, float height);

	glm::vec2 gradientAt(glm::vec2 pos);

	void smoothen();
	void addDetail();
	void addRockRoughness();


	void upload();
	void bind(ShaderProgram& shader);

	void bindHeightmapTexture(unsigned int slot);

	auto getSize()
	{
		return size;
	}
	auto getPos()
	{
		return pos;
	}
};