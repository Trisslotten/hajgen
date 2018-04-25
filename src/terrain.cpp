#include "terrain.hpp"

#include "eroder.hpp"
#include <iostream>
#include "window.hpp"

void Terrain::generateChunk(glm::ivec2 pos)
{
	auto item = maps.find(pos);

	if (item == maps.end())
	{
		maps[pos] = nullptr;
		toGenerate.push(pos);
	}
}

Terrain::~Terrain()
{

}

void Terrain::init()
{
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
	shader.add(GL_GEOMETRY_SHADER, "testgeom.glsl");
	shader.add(GL_FRAGMENT_SHADER, "testfrag.glsl");
	shader.compile();



	int chunkX = glm::floor(camera.position.x / HEIGHTMAP_SIZE.x);
	int chunkY = glm::floor(camera.position.z / HEIGHTMAP_SIZE.z);
	float radius = 20;
	for (int i = -radius; i <= radius; i++)
	{
		int width = sqrt(radius*radius - i * i);
		for (int j = -width; j <= width; j++)
		{
			generateChunk(glm::ivec2(chunkX + i, chunkY + j));
		}
	}
}


void Terrain::update(glm::vec3 pos)
{
	
	int chunkX = glm::floor(camera.position.x / HEIGHTMAP_SIZE.x);
	int chunkY = glm::floor(camera.position.z / HEIGHTMAP_SIZE.z);
	float radius = 3;
	for (int i = -radius; i <= radius; i++)
	{
		int width = sqrt(radius*radius - i * i);
		for (int j = -width; j <= width; j++)
		{
			generateChunk(glm::ivec2(chunkX + i, chunkY + j));
		}
	}


	int slots = 1 - generating.size();
	for (int i = 0; i < slots && !toGenerate.empty(); i++)
	{
		auto pos = toGenerate.front();
		toGenerate.pop();

		toErode.push_back(pos);

		for (int y = -1; y <= 1; y++)
		{
			for (int x = -1; x <= 1; x++)
			{
				auto p = pos + glm::ivec2(x, y);
				auto item = maps.find(p);
				if (item == maps.end())
				{ // not found
					generating.push_back(std::async(std::launch::async, [p]()
					{
						return new Heightmap(p);
					}));
				}
				else if (!item->second)
				{
					generating.push_back(std::async(std::launch::async, [p]()
					{
						return new Heightmap(p);
					}));
				}
			}
		}
	}


	for (int i = 0; i < generating.size(); i++)
	{
		auto status = generating[i].wait_for(std::chrono::nanoseconds(1));

		if (status == std::future_status::ready)
		{
			auto hm = generating[i].get();
			hm->upload();
			maps[hm->getPos()] = hm;

			generating.erase(generating.begin() + i);
			i--;
		}
	}


	for(int i = 0; i < toErode.size() && eroding.size() < 3; i++)
	{
		auto pos = toErode[i];

		bool canStart = true;

		Heightmap** param = new Heightmap*[3*3];
		for (int y = 0; y < 3 && canStart; y++)
		{
			for (int x = 0; x < 3 && canStart; x++)
			{
				auto p = pos + glm::ivec2(x-1, y-1);
				auto item = maps.find(p);
				if (item != maps.end())
				{
					if (item->second)
					{
						param[x + y * 3] = item->second;
					}
					else
					{
						canStart = false;
					}
				}
				else
				{
					canStart = false;
				}
			}
		}
		if (!canStart)
			continue;

		for (auto& p : erodingPositions)
		{
			glm::ivec2 dist = abs(pos - p);
			if (dist.x < 3 && dist.y < 3)
			{
				canStart = false;
				break;
			}
		}

		if (canStart)
		{
			toErode.erase(toErode.begin() + i);
			eroding.push_back(std::async(std::launch::async, erodeCenterHeightmap, param));
			erodingPositions.push_back(pos);
		}
	}

	for(int i = 0; i < eroding.size(); i++)
	{
		auto& item = eroding[i];

		auto status = item.wait_for(std::chrono::nanoseconds(1));

		if (status == std::future_status::ready)
		{
			auto pos = item.get();
			for (int y = -1; y <= 1; y++)
			{
				for (int x = -1; x <= 1; x++)
				{
					auto p = pos + glm::ivec2(x, y);
					auto iter = maps.find(p);

					if (iter != maps.end())
					{
						if (iter->second)
						{
							maps[p]->upload();
						}
					}
				}
			}
			eroding.erase(eroding.begin() + i);
			erodingPositions.erase(erodingPositions.begin() + i);
			break;
		}
	}





	double dt = dtimer.restart();
	camera.update(dt);

	auto iter = maps.find(glm::ivec2(chunkX, chunkY));

	//std::cout << "camera (" << camera.position.x << ", " << camera.position.z << ")\n";
	//std::cout << "chunk (" << chunkX << ", " << chunkY << ")\n\n";
	if (iter != maps.end())
	{
		if (iter->second)
		{
			camera.position.y = iter->second->heightAt(camera.position) + 1.8;
		}
	}
}

void Terrain::draw()
{
	glClearColor(47.f / 255, 141.f / 255, 255.f / 255, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	shader.use();

	shader.uniform("proj_view", camera.getViewProj());


	glBindVertexArray(patch_vao);
	glPatchParameteri(GL_PATCH_VERTICES, 4);

	shader.uniform("fov", camera.fov);
	shader.uniform("cameraPos", camera.position);
	shader.uniform("windowSize", Window::size());

	int patches = 128/64;
	shader.uniform("numPatches", float(patches));
	for (auto item : maps)
	{
		auto hm = item.second;
		if (hm)
		{
			hm->bind(shader);

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


	
}