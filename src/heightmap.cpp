#include "heightmap.hpp"
#include <GL\glew.h>
#include "FastNoise.h"
#include <iostream>
#include <limits>
#include <stdlib.h>
#include <time.h>
#include <glm\glm.hpp>

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

void Heightmap::dropErodeOnce()
{
	float sediment = 0.f;
	glm::vec2 vel;
	glm::vec2 pos;
	pos.x = float(size) * float(rand()) / float(RAND_MAX);
	pos.y = float(size) * float(rand()) / float(RAND_MAX);
	for (int j = 0; j < 200; j++)
	{
		int x = glm::round(pos.x);
		int y = glm::round(pos.y);
		if (x < 0 || x >= size || y < 0 || y >= size)
			break;


		glm::vec4 h;

		h[0] = noisemap2[index(x, y - 1, size)];
		h[1] = noisemap2[index(x - 1, y, size)];
		h[2] = noisemap2[index(x + 1, y, size)];
		h[3] = noisemap2[index(x, y + 1, size)];

		float ratio = 0.001f;
		glm::vec3 n;
		n.z = ratio * (h[0] - h[3]);
		n.x = ratio * (h[1] - h[2]);
		n.y = 2.f*ratio*ratio;
		n = normalize(n);


		glm::vec2 acc;
		acc.x = n.x;
		acc.y = n.z;
		acc *= 0.2f;

		vel += acc;
		vel -= 0.09f*vel;


		float stolen = 0.02f * glm::smoothstep(0.f, 0.6f, length(vel));
		noisemap[x + y * size] -= stolen;
		sediment += stolen;

		
		float returned = 0.05f * sediment;
		noisemap[x + y * size] += returned;
		sediment -= returned;

		if (n.y > 0.9999f)
		{
			noisemap[x + y * size] += sediment;
			break;
		}
		pos += vel;
	}

	memcpy(noisemap2, noisemap, sizeof(float) * size*size);

	float* temp = noisemap;
	noisemap = noisemap2;
	noisemap2 = temp;
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

void Heightmap::generate()
{
	srand(time(NULL));

	FastNoise noise;
	noise.SetNoiseType(FastNoise::NoiseType::SimplexFractal);

	noise.SetFrequency(0.0005f);

	size = 800;

	noisemap = new float[size*size];
	noisemap2 = new float[size*size];
	
	for (int y = 0; y < size; y++)
	{
		for (int x = 0; x < size; x++)
		{
			noisemap[x + y * size]= 0.5f*noise.GetNoise(x, y) + 0.5f;
			noisemap2[x + y * size] = noisemap[x + y * size];
		}
	}

	heightmap = new uint16_t[size*size];

	for (int i = 0; i < size*size; i++)
	{
		heightmap[i] = glm::max(noisemap[i], 0.f) * float(std::numeric_limits<uint16_t>::max());
	}

	glGenTextures(1, &heightmapTex);
	glBindTexture(GL_TEXTURE_2D, heightmapTex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_R16, size, size, 0, GL_RED, GL_UNSIGNED_SHORT, &heightmap[0]);
	glBindTexture(GL_TEXTURE_2D, 0);


	float vertexbuffer[] = {
		-1.0,  1.0, 0.0,
		0.0,  1.0,

		1.0,  1.0, 0.0,
		1.0,  1.0,

		-1.0, -1.0, 0.0,
		0.0,  0.0,

		1.0, -1.0, 0.0,
		1.0,  0.0
	};
	glGenVertexArrays(1, &quad_vao);
	glBindVertexArray(quad_vao);
	glGenBuffers(1, &quad_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
	glBufferData(GL_ARRAY_BUFFER, 20 * sizeof(float), vertexbuffer, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (GLvoid*)0);
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (GLvoid*)(sizeof(float) * 3));


	shader.add(GL_VERTEX_SHADER, "testvert.glsl");
	shader.add(GL_FRAGMENT_SHADER, "testfrag.glsl");
	shader.compile();
}

void Heightmap::update()
{
	for (int i = 0; i < 300; i++)
	{
		dropErodeOnce();
	}
	if (timer.elapsed() > 0.1)
	{
		timer.restart();
		upload();
	}
}

void Heightmap::draw()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shader.use();
	shader.uniform("texSampler", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, heightmapTex);

	glBindVertexArray(quad_vao);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}
