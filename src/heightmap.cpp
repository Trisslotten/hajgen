#include "heightmap.hpp"
#include <GL\glew.h>
#include "FastNoise.h"
#include <iostream>
#include <limits>
#include <stdlib.h>
#include <time.h>
#include <glm\glm.hpp>
#include <glm\gtx\quaternion.hpp>
#include "window.hpp"
#include "lodepng.h"
#include "misc.hpp"

size_t index(int x, int y, int size)
{
	if (x < 0)
		x = 0;
	if (x >= size)
		x = size-1;
	if (y < 0)
		y = 0;
	if (y >= size)
		y = size-1;
	return x + y * size;
}



Heightmap::Heightmap(glm::vec2 _pos)
{
	pos = _pos;

	seed = 6050;

	size = HEIGHTMAP_SIZE;
	scale = 0.5f;


	resolution = HEIGHTMAP_RESOLUTION;
	frequency = 500.5f / size.y;


	noisemap = new float[resolution*resolution];
	smoothTemp = new float[resolution*resolution];
	heightmap = new uint16_t[resolution*resolution];


	generate();
	/*
	while (iterations < maxIterations)
	{
	iterate();
	}
	*/
}

Heightmap::~Heightmap()
{
	delete[] noisemap;
	delete[] smoothTemp;
	delete[] heightmap;
}


void Heightmap::upload()
{
	for (int i = 0; i < resolution*resolution; i++)
	{
		heightmap[i] = glm::clamp(noisemap[i], 0.f, 1.f) * float(std::numeric_limits<uint16_t>::max());
	}

	glGenTextures(1, &heightmapTex);
	glBindTexture(GL_TEXTURE_2D, heightmapTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, resolution, resolution, 0, GL_RED, GL_UNSIGNED_SHORT, &heightmap[0]);
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Heightmap::bind(ShaderProgram& shader)
{
	shader.uniform("heightmap", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, heightmapTex);
	shader.uniform("size", size);
	shader.uniform("hmPos", glm::vec2(pos));
}

void Heightmap::bindHeightmapTexture(unsigned int slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, heightmapTex);
}


glm::vec2 Heightmap::gradientAt(glm::vec2 pos)
{
	return glm::vec2(
		heightAt(pos - glm::vec2(0.01, 0)) - heightAt(pos + glm::vec2(0.01, 0)),
		heightAt(pos - glm::vec2(0, 0.01)) - heightAt(pos + glm::vec2(0, 0.01))
	);
}



void Heightmap::configureNoises()
{
	float mult = frequency * size.x / (resolution * scale);

	ns.mountains.SetNoiseType(FastNoise::NoiseType::SimplexFractal);
	ns.mountains.SetSeed(seed);
	ns.mountains.SetFractalOctaves(7);
	ns.mountains.SetFrequency(0.0005f *  mult);


	ns.fields.SetNoiseType(FastNoise::NoiseType::SimplexFractal);
	ns.fields.SetSeed(seed);
	ns.fields.SetFractalOctaves(3);
	ns.fields.SetFrequency(0.001f * mult);


	ns.biome.SetNoiseType(FastNoise::NoiseType::SimplexFractal);
	ns.biome.SetSeed(seed);
	ns.biome.SetFractalOctaves(1);
	ns.biome.SetFrequency(0.00005f * mult);


	ns.detail.SetNoiseType(FastNoise::NoiseType::SimplexFractal);
	ns.detail.SetSeed(seed);
	ns.detail.SetFractalOctaves(1);
	ns.detail.SetFrequency(5.f * mult);
}

float Heightmap::getBiome(float x, float y)
{
	float biome = 0.5f*ns.biome.GetNoise(x, y) + 0.5f;
	biome = glm::smoothstep(0.1f, 0.6f, biome);
	//biome = glm::pow(biome, 2.2f);

	return biome;
}

float Heightmap::fieldNoise(float x, float y)
{
	float fields = 0.5f*ns.fields.GetNoise(x, y) + 0.5f;
	fields *= 0.3f;

	return fields;
}

float Heightmap::mountainNoise(float x, float y)
{
	float mountains = 0.5f*ns.mountains.GetNoise(x, y) + 0.5f;
	//mountains = pow(mountains, 2.f);
	mountains = 1.f - glm::abs(2.f*mountains - 1.f);
	mountains = pow(mountains, 5.f);
	float mountains2 = 0.5f*ns.mountains.GetNoise(-y, -x) + 0.5f;
	mountains *= 0.3f;
	return mountains + 1.5f*mountains2;
}



float Heightmap::getNoise(float x, float y)
{
	
	float mountains = mountainNoise(x, y);

	float fields = fieldNoise(x, y);

	float biome = getBiome(x, y);

	float result = glm::mix(mountains, fields, biome);

	result += 0.2f;

	return result;
}


void Heightmap::generate()
{
	srand(seed);
	
	configureNoises();
	
	
	for (int iy = 0; iy < resolution; iy++)
	{
		for (int ix = 0; ix < resolution; ix++)
		{
			// share edges between heightmaps;
			float x = ix - 3 + (int(resolution) - 3) * pos.x;
			float y = iy - 3 + (int(resolution) - 3) * pos.y;
			noisemap[ix + iy * resolution] = scale * getNoise(x, y);
		}
	}
}

void Heightmap::smoothen()
{
	// sigma 1.0
	int kernelSize = 5;
	float kernel[] = { 0.00135,	0.157305,	0.68269,	0.157305,	0.00135 };

	for (int y = 0; y < resolution; y++)
	{
		for (int x = 0; x < resolution; x++) 
		{
			float smoothed = 0.f;
			for (int i = 0; i < kernelSize; i++)
			{
				int offset = i - kernelSize / 2;

				smoothed += kernel[i] * noisemap[index(x, y + offset, resolution)];
			}

			smoothTemp[x + y * resolution] = smoothed;
		}
	}

	std::swap(noisemap, smoothTemp);

	for (int y = 0; y < resolution; y++)
	{
		for (int x = 0; x < resolution; x++)
		{
			float smoothed = 0.f;
			for (int i = 0; i < kernelSize; i++)
			{
				int offset = i - kernelSize / 2;

				smoothed += kernel[i] * noisemap[index(x + offset, y, resolution)];
			}
			smoothTemp[x + y * resolution] = smoothed;
			
		}
	}

	std::swap(noisemap, smoothTemp);
}

void Heightmap::addDetail()
{
	for (int y = 0; y < resolution; y++)
	{
		for (int x = 0; x < resolution; x++)
		{
			noisemap[x + y * resolution] += 0.0001f*ns.detail.GetNoise(x, y);
		}
	}
}

void Heightmap::addRockRoughness()
{
	static Image image("assets/textures/rock_displacement.png");

	//std::cout << "Roughness " << image.colorAt(glm::vec2(0.5)).r << "\n";

	memcpy(smoothTemp, noisemap, sizeof(float) * resolution * resolution);

	for (int y = 0; y < resolution; y++)
	{
		for (int x = 0; x < resolution; x++)
		{
			
			//vec2 uvMedium = teposition.xz / 1000.0;
			glm::vec2 worldPos = glm::vec2(x, y) / float(resolution);
			worldPos *= glm::vec2(size.x, size.z);
			worldPos += glm::vec2(size.x, size.z) * glm::vec2(pos);
			glm::vec2 uv = worldPos / 1000.f;

			glm::vec3 normal = normalAt(worldPos);

			float threshold = 0.9;
			float hinterval = 0.1;
			float weight = glm::smoothstep(threshold + hinterval, threshold - hinterval, normal.y);
			glm::vec3 disp = 5.f * normal * weight * (2.f*image.colorAt(uv).r - 1.f);
			float height = heightAt(worldPos) + disp.y;
			
			std::swap(noisemap, smoothTemp);
			//setHeightAt(worldPos + glm::vec2(disp.x, disp.z), height);
			addHeightAt(x,y, 4.f*(2.f*image.colorAt(uv).r - 1.f) * weight);
			//addHeightAt(x, y, 5.f*(2.f*image.colorAt(uv).r - 1.f));
			std::swap(noisemap, smoothTemp);
		}
	}
	std::swap(noisemap, smoothTemp);
}




float Heightmap::heightAt(int x, int y)
{

	float result = size.y * glm::max(noisemap[index(x, y, resolution)], 0.f);
	if (x < 0 || x >= resolution || y < 0 || y >= resolution)
	{
		//result += 0.2;
	}
	return result;
}

float Heightmap::heightAt(glm::vec3 _pos)
{
	_pos /= size;
	_pos -= glm::vec3(pos.x, 0, pos.y);


	float texel = 1.0f / float(resolution);
	_pos -= 0.5f*texel;

	glm::vec3 border = glm::vec3(1.5f*texel);
	_pos = (1.0f - 2.0f*border)*_pos + border;

	int x = resolution * _pos.x;
	int y = resolution * _pos.z;

	float tx = glm::fract(resolution * _pos.x);
	float ty = glm::fract(resolution * _pos.z);

	float h00 = heightAt(x, y);
	float h10 = heightAt(x + 1, y);
	float h01 = heightAt(x, y + 1);
	float h11 = heightAt(x + 1, y + 1);

	float height = 0;
	height += h00 * (1.f - tx) * (1.f - ty);
	height += h10 * tx * (1.f - ty);
	height += h01 * (1.f - tx) * ty;
	height += h11 * tx * ty;

	return height;
}

float Heightmap::heightAt(glm::vec2 pos)
{
	return heightAt(glm::vec3(pos.x, 0, pos.y));
}

glm::vec3 Heightmap::normalAt(glm::vec2 pos)
{
	float t = 1.f / resolution;

	float wt = t * size.x;

	glm::vec4 h;
	h[0] = heightAt(pos + glm::vec2(0, -1) * wt) / size.y;
	h[1] = heightAt(pos + glm::vec2(-1, 0) * wt) / size.y;
	h[2] = heightAt(pos + glm::vec2(1, 0) * wt) / size.y;
	h[3] = heightAt(pos + glm::vec2(0, 1) * wt) / size.y;

	float ratioX = t * size.x / (size.y);
	float ratioZ = t * size.z / (size.y);
	glm::vec3 n;
	n.x = ratioX * (h[1] - h[2]);
	n.z = ratioZ * (h[0] - h[3]);
	n.y = 2 * ratioX*ratioZ;
	return normalize(n);
}

void Heightmap::setHeightAt(int x, int y, float height)
{
	if (x >= 0 && x < resolution && y >= 0 && y < resolution)
		noisemap[index(x, y, resolution)] = height / size.y;
}

void Heightmap::addHeightAt(int x, int y, float height)
{
	if (x >= 0 && x < resolution && y >= 0 && y < resolution)
		noisemap[index(x, y, resolution)] += height / size.y;
}

void Heightmap::setHeightAt(glm::vec2 _pos, float height)
{
	_pos /= size.x;
	_pos -= glm::vec2(pos.x, pos.y);

	float texel = 1.0f / float(resolution);
	_pos -= 0.5f*texel;

	glm::vec2 border = glm::vec2(1.5f*texel);
	_pos = (1.0f - 2.0f*border)*_pos + border;

	int x = resolution * _pos.x;
	int y = resolution * _pos.y;

	setHeightAt(x, y, height);
}

void Heightmap::addHeightAt(glm::vec2 _pos, float height)
{
	_pos /= size.x;
	_pos -= glm::vec2(pos.x, pos.y);

	float texel = 1.0f / float(resolution);
	_pos -= 0.5f*texel;

	glm::vec2 border = glm::vec2(1.5f*texel);
	_pos = (1.0f - 2.0f*border)*_pos + border;

	int x = resolution * _pos.x;
	int y = resolution * _pos.y;

	float tx = glm::fract(resolution * _pos.x);
	float ty = glm::fract(resolution * _pos.y);

	addHeightAt(x, y, height * (1.f - tx)*(1.f - ty));
	addHeightAt(x, y + 1, height * (1.f - tx) * ty);
	addHeightAt(x + 1, y, height * tx * (1.f - ty));
	addHeightAt(x + 1, y + 1, height * tx * ty);

}

Image::Image(const std::string & path)
{
	unsigned error = lodepng::decode(image, width, height, path);
}

glm::vec4 Image::colorAt(int x, int y)
{
	x = x % width;
	y = y % height;
	int i = 4 * x + y * width * 4;
	unsigned char r = image[i];
	unsigned char g = image[i + 1];
	unsigned char b = image[i + 2];
	unsigned char a = image[i + 3];
	return glm::vec4(r,g,b,a) / 255.f;
}

glm::vec4 Image::colorAt(glm::vec2 uv)
{
	int x = uv.x * width;
	int y = uv.y * height;

	float tx = glm::fract(width * uv.x);
	float ty = glm::fract(height* uv.y);

	glm::vec4 c00 = colorAt(x, y);
	glm::vec4 c10 = colorAt(x + 1, y);
	glm::vec4 c01 = colorAt(x, y + 1);
	glm::vec4 c11 = colorAt(x + 1, y + 1);

	glm::vec4 result;
	result += c00 * (1.f - tx) * (1.f - ty);
	result += c10 * tx * (1.f - ty);
	result += c01 * (1.f - tx) * ty;
	result += c11 * tx * ty;
	return result;
}
