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
// [0,1]
float random()
{
	return float(rand()) / float(RAND_MAX);
}


Heightmap::Heightmap(glm::vec2 _pos)
{
	pos = _pos;

	seed = 602;

	size = glm::vec3(2048, 1000, 2048);
	scale = 1.0f;


	resolution = 2048;
	frequency = 500.5f / size.y;

	
	smoothInterval = 2000UL;
	maxIterations = 2 * resolution * resolution;
	maxIterations = 0;

	generate();
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

float Heightmap::heightAt(int x, int y)
{
	return size.y * glm::max(noisemap[index(x, y, resolution)], 0.f);
}

float Heightmap::heightAt(glm::vec3 pos)
{
	pos /= size;

	int x = resolution * pos.x;
	int y = resolution * pos.z;

	float tx = glm::fract(resolution * pos.x);
	float ty = glm::fract(resolution * pos.z);

	float h00 = heightAt(x, y);
	float h10 = heightAt(x+1, y);
	float h01 = heightAt(x, y+1);
	float h11 = heightAt(x+1, y+1);

	float height = 0;
	height += h00 * (1.f - tx)* (1.f - ty);
	height += h10 * tx * (1.f - ty);
	height += h01 * (1.f - tx) * ty;
	height += h11 * tx * ty;

	return height;
}

float Heightmap::heightAt(glm::vec2 pos)
{
	return heightAt(glm::vec3(pos.x, 0, pos.y));
}

void Heightmap::bind(ShaderProgram& shader)
{
	shader.uniform("heightmap", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, heightmapTex);
	shader.uniform("size", size);
	shader.uniform("hmPos", pos);
}

glm::vec3 Heightmap::normalAt(glm::vec2 pos)
{
	float t = 1.f / resolution;

	glm::vec4 h;
	h[0] = heightAt(pos + glm::vec2(0, -1) / size.x) / size.y;
	h[1] = heightAt(pos + glm::vec2(-1, 0) / size.x) / size.y;
	h[2] = heightAt(pos + glm::vec2(1, 0) / size.x) / size.y;
	h[3] = heightAt(pos + glm::vec2(0, 1) / size.x) / size.y;

	float ratioX = t * size.x / (size.y);
	float ratioZ = t * size.z / (size.y);
	glm::vec3 n;
	n.x = ratioX * (h[1] - h[2]);
	n.z = ratioZ * (h[0] - h[3]);
	n.y = 2 * ratioX*ratioZ;
	return normalize(n);
}

void Heightmap::addHeightAt(int x, int y, float height)
{
	if(x >= 0 && x < resolution && y >= 0 && y < resolution)
		noisemap[index(x, y, resolution)] += height;
}

void Heightmap::addHeightAt(glm::vec2 pos, float height)
{
	int x = glm::floor(resolution * pos.x / size.x);
	int y = glm::floor(resolution * pos.y / size.z);

	height /= size.y;

	float tx = glm::fract(resolution * pos.x / size.x);
	float ty = glm::fract(resolution * pos.y / size.z);

	
	addHeightAt(x, y, height * (1.f - tx)*(1.f - ty));
	addHeightAt(x, y+1, height * (1.f - tx) * ty);
	addHeightAt(x+1, y, height * tx * (1.f - ty));
	addHeightAt(x+1, y+1, height * tx * ty);
	
}

void Heightmap::addHeightAt(glm::vec2 pos, float radius, float volume)
{
	float pixelArea = size.x*1.f / resolution;
	pixelArea *= pixelArea;

	float totalVolume = 0.f;


	int ir = resolution * radius / size.x;

	if (ir <= 0)
		ir = 1;


	float step = 1.0f;

	float mult = 3.333333f * step*step * volume / (radius * radius * glm::pi<float>());

	int px = glm::floor(resolution * pos.x / size.x);
	int py = glm::floor(resolution * pos.y / size.z);

	glm::vec2 offset;
	for (int iy = -ir-1; iy <= ir+1; iy += 1)
	{
		offset.y = iy * radius / ir;
		int width = glm::sqrt(ir*ir - iy * iy);
		for (int ix = -width-1; ix <= width+1; ix += 1)
		{
			offset.x = ix * radius / ir;
			float h = mult * glm::smoothstep(radius, 0.f, length(offset));

			h = mult * (1.f - glm::clamp(length(offset)/radius, 0.f, 1.f));

			addHeightAt(px + ix, py + iy, h / size.y);

			totalVolume += h * pixelArea;
		}
	}

	//std::cout << "totalVolume = " << totalVolume << "\n";
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
	ns.mountains.SetFractalOctaves(20);
	ns.mountains.SetFrequency(0.001f *  mult);


	ns.fields.SetNoiseType(FastNoise::NoiseType::SimplexFractal);
	ns.fields.SetSeed(seed);
	ns.fields.SetFractalOctaves(20);
	ns.fields.SetFrequency(0.001f * mult);


	ns.biome.SetNoiseType(FastNoise::NoiseType::SimplexFractal);
	ns.biome.SetSeed(seed);
	ns.biome.SetFractalOctaves(2);
	ns.biome.SetFrequency(0.0005f * mult);


	ns.detail.SetNoiseType(FastNoise::NoiseType::SimplexFractal);
	ns.detail.SetSeed(seed);
	ns.detail.SetFractalOctaves(1);
	ns.detail.SetFrequency(5.f * mult);
}

float Heightmap::getBiome(float x, float y)
{
	float biome = 0.5f*ns.biome.GetNoise(x, y) + 0.5f;
	biome = glm::smoothstep(0.1f, 0.8f, biome);
	biome = glm::pow(biome, 0.5f);

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
	float mountains2 = 0.5f*ns.mountains.GetNoise(-y, x) + 0.5f;
	mountains *= 0.6;

	return mountains + 0.5f*mountains2;
}



float Heightmap::getNoise(float x, float y)
{
	
	float mountains = mountainNoise(x, y);

	float fields = fieldNoise(x, y);

	float biome = getBiome(x, y);

	float result = glm::mix(mountains, fields, biome);

	result += 0.2f;

	//result = 0.01f * length(fgrad);

	return result;
}



void Heightmap::generate()
{
	srand(time(NULL));


	noisemap = new float[resolution*resolution];
	smoothTemp = new float[resolution*resolution];
	
	configureNoises();
	
	
	for (int iy = 0; iy < resolution; iy++)
	{
		for (int ix = 0; ix < resolution; ix++)
		{
			float x = ix + resolution * pos.x;
			float y = iy + resolution * pos.y;

			noisemap[ix + iy * resolution]= scale * getNoise(x, y);
		}
	}
	while (iterations < maxIterations)
	{
		iterate();
	}
	heightmap = new uint16_t[resolution*resolution];
}

void Heightmap::iterate()
{
	dropErodeOnce();
	iterations++;
		
	if (iterations % smoothInterval == 0)
	{
		smoothen();
	}


	if (iterations % (smoothInterval) == 0)
	{
		addDetail();
	}


	if (iterations % (20 * smoothInterval) == 0)
	{
		smoothInterval *= 2;
	}
}



void Heightmap::dropErodeOnce()
{
	glm::vec2 vel;
	glm::vec2 pos;

	bool done = false;
	while (!done)
	{
		float rn1 = random();
		float rn2 = random();

		float biome = getBiome(rn1 * resolution, rn2 * resolution);
		//if (biome < 1.0)//random())
			done = true;
		pos.x = size.x * rn1;
		pos.y = size.z * rn2;
	}

	float stepSize = 1.f;
	float erosionRatio = 0.9f;
	float depositRatio = 0.5f;


	float sediment = 0.f;


	for (int j = 0; j < 100; j++)
	{
		if (pos.x <= 0 || pos.x >= size.x || pos.y <= 0 || pos.y >= size.z)
		{
			break;
		}

		glm::vec2 dir = gradientAt(pos);
		vel += 0.5f * dir;
		glm::vec2 step = stepSize * size.x * normalize(vel) / float(resolution);
		vel *= 0.5f;

		float currHeight = heightAt(pos);
		float nextHeight = heightAt(pos + step);

		float eroded = erosionRatio *glm::max(currHeight - nextHeight, 0.f) * glm::smoothstep(0.f,5.f, float(j));
		eroded -= depositRatio * sediment;
		addHeightAt(pos, -eroded);
		sediment += eroded;

		pos += step;
	}

	if (sediment > 0)
	{
		//addHeightAt(pos, sediment);
	}
}

void Heightmap::smoothen()
{
	float kernel[] = { 0.00135,	0.157305,	0.68269,	0.157305,	0.00135 };

	for (int y = 0; y < resolution; y++)
	{
		for (int x = 0; x < resolution; x++) 
		{
			float smoothed = 0.f;
			for (int i = 0; i < 5; i++)
			{
				int offset = i - 2;

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
			for (int i = 0; i < 5; i++)
			{
				int offset = i - 2;

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

Heightmap * generateHeightmap(glm::vec2 pos)
{
	Heightmap* result = new Heightmap(pos);
	return result;
}
