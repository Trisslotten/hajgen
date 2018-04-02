#include "heightmap.hpp"
#include <GL\glew.h>
#include <glm\gtc\matrix_transform.hpp>
#include "FastNoise.h"
#include <iostream>
#include <limits>
#include <stdlib.h>
#include <time.h>
#include <glm\glm.hpp>
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

void Heightmap::upload()
{
	for (int i = 0; i < size*size; i++)
	{
		heightmap[i] = glm::max(noisemap[i], 0.f) * float(std::numeric_limits<uint16_t>::max());
	}
	glBindTexture(GL_TEXTURE_2D, heightmapTex);
	glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, size, size, GL_RED, GL_UNSIGNED_SHORT, &heightmap[0]);

}

float Heightmap::heightAt(int x, int y)
{
	return scale.y * glm::max(noisemap[index(x, y, size)], 0.f);
}

float Heightmap::heightAt(glm::vec3 pos)
{
	pos /= scale;

	int x = size * pos.x;
	int y = size * pos.z;

	float tx = glm::fract(size * pos.x);
	float ty = glm::fract(size * pos.z);
	
	float xlerp1 = glm::mix(heightAt(x, y), heightAt(x + 1, y), tx);
	float xlerp2 = glm::mix(heightAt(x, y+1), heightAt(x + 1, y+1), tx);

	float height = glm::mix(xlerp1, xlerp2, ty);

	return height;
}

float Heightmap::heightAt(glm::vec2 pos)
{
	return heightAt(glm::vec3(pos.x, 0, pos.y));
}

glm::vec3 Heightmap::normalAt(glm::vec2 pos)
{
	float t = 1.f / size;

	glm::vec4 h;
	h[0] = heightAt(pos + glm::vec2(0, -1) / scale.x);
	h[1] = heightAt(pos + glm::vec2(-1, 0) / scale.x);
	h[2] = heightAt(pos + glm::vec2(1, 0) / scale.x);
	h[3] = heightAt(pos + glm::vec2(0, 1) / scale.x);

	float ratioX = t * scale.x / (scale.y);
	float ratioZ = t * scale.z / (scale.y);
	glm::vec3 n;
	n.x = ratioX * (h[1] - h[2]);
	n.z = ratioZ * (h[0] - h[3]);
	n.y = 2 * ratioX*ratioZ;
	return normalize(n);
}

void Heightmap::addHeightAt(int x, int y, float height)
{
	if(x >= 0 || x < size || y >= 0 || y < size)
		noisemap[index(x, y, size)] += height;
}

void Heightmap::addHeightAt(glm::vec2 pos, float height)
{
	int x = size * pos.x / scale.x;
	int y = size * pos.y / scale.z;

	float tx = glm::fract(size * pos.x);
	float ty = glm::fract(size * pos.y);

	addHeightAt(x, y, height * tx * ty);
	addHeightAt(x+1, y, height * (1.f - tx) * ty);
	addHeightAt(x, y+1, height * tx * (1.f - ty));
	addHeightAt(x+1, y+1, height * (1.f - tx)*(1.f - ty));
}

void Heightmap::generate()
{
	srand(time(NULL));

	scale = glm::vec3(5, 5, 5);

	FastNoise noise;
	noise.SetNoiseType(FastNoise::NoiseType::SimplexFractal);

	noise.SetFrequency(0.001f);

	size = 800;

	noisemap = new float[size*size];
	
	for (int y = 0; y < size; y++)
	{
		for (int x = 0; x < size; x++)
		{
			noisemap[x + y * size]= 0.5f*noise.GetNoise(x, y) + 0.5f;
		}
	}

	heightmap = new uint16_t[size*size];

	for (int i = 0; i < size*size; i++)
	{
		heightmap[i] = glm::max(noisemap[i]/ 100.f, 0.f) * float(std::numeric_limits<uint16_t>::max());
	}

	glGenTextures(1, &heightmapTex);
	glBindTexture(GL_TEXTURE_2D, heightmapTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, size, size, 0, GL_RED, GL_UNSIGNED_SHORT, &heightmap[0]);
	glBindTexture(GL_TEXTURE_2D, 0);


	float patch_vertices[] = {
		0.0, 0.0,
		1.0, 0.0,
		0.0, 1.0,
		1.0, 1.0
	};

	glGenVertexArrays(1, &patch_vao);
	glBindVertexArray(patch_vao);
	glGenBuffers(1, &patch_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, patch_vbo);
	glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), patch_vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (GLvoid*)0);

	shader.add(GL_VERTEX_SHADER, "testvert.glsl");
	shader.add(GL_TESS_CONTROL_SHADER, "testctrl.glsl");
	shader.add(GL_TESS_EVALUATION_SHADER, "testeval.glsl");
	shader.add(GL_FRAGMENT_SHADER, "testfrag.glsl");
	shader.compile();
}

void Heightmap::update()
{
	double dt = dtimer.restart();
	erodeTimeAccum += dt;
	while(erodeTimeAccum > 0.f)
	{
		dropErodeOnce();
		erodeTimeAccum -= 1.0 / 10000;
	}
	if (timer.elapsed() > 1.0 / 15.0)
	{
		timer.restart();
		upload();
	}
}


void Heightmap::dropErodeOnce()
{
	float sediment = 0.f;
	glm::vec2 vel;
	glm::vec2 pos;
	pos.x = scale.x * float(rand()) / float(RAND_MAX);
	pos.y = scale.z * float(rand()) / float(RAND_MAX);

	glm::vec3 n = normalAt(pos);

	vel.x = n.x;
	vel.y = n.z;
	vel = normalize(vel);

	for (int j = 0; j < 300; j++)
	{
		n = normalAt(pos);

		glm::vec2 acc;
		acc.x = n.x;
		acc.y = n.z;
		acc *= 0.5f;

		vel += acc;
		vel.x = n.x;
		vel.y = n.z;

		float stolen = 0.01f * length(vel);
		stolen = 0.0001f;
		addHeightAt(pos, -stolen);
		sediment += stolen;


		float returned = 0.9f * sediment;
		returned = 0.f;
		//addHeightAt(pos, returned);
		//sediment -= returned;

		pos += 1.0f * scale.x * normalize(vel)/float(size);
	}
	//addHeightAt(pos, sediment);
}


void Heightmap::draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shader.use();
	shader.uniform("heightmap", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, heightmapTex);


	auto size = Window::size();
	glm::mat4 proj = glm::perspective(glm::radians(60.f), size.x / size.y, 1.f, 500.f);
	glm::mat4 view = glm::lookAt(glm::vec3(0,5,0), scale*0.5f, glm::vec3(0, 1, 0));
	shader.uniform("proj_view", proj*view);

	shader.uniform("scale", scale);

	

	glBindVertexArray(patch_vao);
	glPatchParameteri(GL_PATCH_VERTICES, 4);

	int patches = 50;
	float patchSize = scale.x/patches;
	shader.uniform("patchSize", patchSize);
	for (int y = 0; y < patches; y++)
	{
		for (int x = 0; x < patches; x++)
		{
			shader.uniform("patchPos", patchSize * glm::vec2(x, y));
			glDrawArrays(GL_PATCHES, 0, 4);
		}
	}
}
