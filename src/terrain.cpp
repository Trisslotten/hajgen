#include "terrain.hpp"

Terrain::~Terrain()
{
	for (auto& future : generating)
	{
		future.
	}
}

void Terrain::init()
{
	for (int y = -5; y <= 5; y++)
	{
		for (int x = -5; x <= 5; x++)
		{
			toGenerate.push(glm::vec2(x, y));
		}
	}



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


void Terrain::update(glm::vec3 pos)
{
	for (int i = 0; i < generating.size(); i++)
	{
		auto status = generating[i].wait_for(std::chrono::nanoseconds(1));
		if (status == std::future_status::ready)
		{
			heightmaps.push_back(generating[i].get());
			heightmaps.back()->upload();
			generating.erase((generating.begin() + i));
			i--;
		}
	}

	int slots = 3 - generating.size();
	for (int i = 0; i < slots && !toGenerate.empty(); i++)
	{
		auto pos = toGenerate.front();
		toGenerate.pop();

		generating.push_back(std::async(generateHeightmap, pos));
	}


	double dt = dtimer.restart();
	camera.update(dt);
}

void Terrain::draw()
{
	glClearColor(47.f / 255, 141.f / 255, 255.f / 255, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shader.use();

	shader.uniform("proj_view", camera.getViewProj());


	glBindVertexArray(patch_vao);
	glPatchParameteri(GL_PATCH_VERTICES, 4);

	for (auto hm : heightmaps)
	{
		hm->bind(shader);
		int patches = 15;
		float patchSize = hm->getSize().x / patches;
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


	
}