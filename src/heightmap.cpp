#include "heightmap.hpp"
#include <GL\glew.h>
#include <glm\gtc\matrix_transform.hpp>
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

	float h00 = heightAt(x, y);
	float h10 = heightAt(x+1, y);
	float h01 = heightAt(x, y+1);
	float h11 = heightAt(x+1, y+1);

	float height = 0;
	height += h00 * tx * ty;
	height += h10 * (1.f-tx) * ty;
	height += h01 * tx * (1.f - ty);
	height += h11 * (1.f - tx)* (1.f - ty);

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

void Heightmap::addHeightAt(glm::vec2 pos, float radius, float height)
{
	int ir = size * radius / scale.x;

	if (ir <= 0)
		ir = 1;

	float r = radius/ scale.x;
	float area = r * r * glm::pi<float>();

	float step = 1.0f;

	float div = 1.f*area;
	float k = 0.3*step * step * height / scale.y;
	
	glm::vec2 offset;
	for (float iy = -ir; iy <= ir; iy += step)
	{
		offset.y = iy * radius / ir;
		int width = glm::sqrt(ir*ir - iy * iy);
		for (float ix = -width; ix <= width; ix += step)
		{
			offset.x = ix * radius / ir;
			float h = k * glm::smoothstep(radius, 0.f, length(offset));
			addHeightAt(pos + offset, h);
		}
	}
}

void Heightmap::generate()
{
	srand(time(NULL));

	scale = glm::vec3(10, 6, 10);

	size = 1000;

	FastNoise noise;
	noise.SetNoiseType(FastNoise::NoiseType::SimplexFractal);
	noise.SetFractalOctaves(10);
	noise.SetFrequency(1.0f / size);


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

		const float erodeFreq = 700.0;
		erodeTimeAccum -= 1.0 / erodeFreq;
	}
	if (timer.elapsed() > 1.0 / 15.0)
	{
		timer.restart();
		upload();
	}
}


void Heightmap::dropErodeOnce()
{
	float Kd = 1.0f;
	float Kr = 1.0f;
	float Kw = 0.01f;
	float Kq = 0.1f;
	float stepSize = 2.0f;
	float minSlope = 0.2f;

	Kq = 10; Kw = 0.001f; Kr = 0.9f; Kd = 0.02f; minSlope = 0.05f;

	float sediment = 0.f;
	float water = 0.2f;

	glm::vec2 vel;
	glm::vec2 pos;
	pos.x = scale.x * float(rand()) / float(RAND_MAX);
	pos.y = scale.z * float(rand()) / float(RAND_MAX);

	glm::vec3 n = normalAt(pos);


	for (int j = 0; j < 200; j++)
	{
		if (pos.x <= 0 || pos.x >= scale.x || pos.y <= 0 || pos.y >= scale.z)
		{
			break;
		}

		n = normalAt(pos);
		glm::vec2 acc;
		acc.x = n.x;
		acc.y = n.z;

		//vel += acc;
		vel = glm::mix(vel, acc, 0.5f);
		

		if (length(vel) < 0.001)
			break;

		glm::vec2 next = pos + stepSize * scale.x * normalize(vel) / float(size);

		float currHeight = heightAt(pos);
		float nextHeight = heightAt(next);


		float capacity = water - sediment;

		float slope = (1 - n.y);
		float val = length(vel);

		
		float eroded = 0.5f*capacity * glm::smoothstep(0.f, 10.f, val);
		addHeightAt(pos, 0.05f, -eroded);
		sediment += eroded;

		//std::cout << sediment << "\n";

		//addHeightAt(pos, 25.f * scale.x / size, -0.05f);

		pos = next;

		water *= 1.f - Kw;
	}

	if (sediment > 0)
	{
		addHeightAt(pos, 0.1f, sediment);
	}
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

	glm::vec3 target = 0.5f * scale;
	glm::vec3 eye = glm::vec3(0, scale.y*1.5, 0);

	eye = glm::quat(glm::vec3(0, 0.2*gt.elapsed(), 0)) * (eye - target) + target;


	glm::mat4 view = glm::lookAt(eye, target, glm::vec3(0, 1, 0));
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
