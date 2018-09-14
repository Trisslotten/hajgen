#include "terrain.hpp"

#include <random>
#include "eroder.hpp"
#include <iostream>
#include "window.hpp"
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtx\transform.hpp>



glm::ivec2 Terrain::getChunkPos(glm::vec3 pos)
{
	int x = glm::floor(pos.x / HEIGHTMAP_SIZE.x);
	int y = glm::floor(pos.z / HEIGHTMAP_SIZE.z);
	return glm::ivec2(x, y);
}

void Terrain::generateChunk(glm::ivec2 pos)
{
	toGenerate.push(pos);
}

Terrain::~Terrain()
{

}

void Terrain::init()
{
	glm::vec2 *patch_vertices = new glm::vec2[MAX_PATCHES * MAX_PATCHES];

	int counter = 0;
	for (int i = 0; i < MAX_PATCHES; i++)
	{
		for (int j = 0; j <= i; j++)
		{
			patch_vertices[counter] = glm::vec2(j, i);
			counter++;
		}
		for (int j = 0; j < i; j++)
		{
			patch_vertices[counter] = glm::vec2(i, j);
			counter++;
		}
	}



	glGenVertexArrays(1, &patch_vao);
	glBindVertexArray(patch_vao);
	glGenBuffers(1, &patch_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, patch_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2)*MAX_PATCHES*MAX_PATCHES, patch_vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(glm::vec2), (GLvoid*)0);

	delete[] patch_vertices;

	shader.add(GL_VERTEX_SHADER,          "testvert.glsl");
	shader.add(GL_TESS_CONTROL_SHADER,    "testctrl.glsl");
	shader.add(GL_TESS_EVALUATION_SHADER, "testeval.glsl");
	shader.add(GL_GEOMETRY_SHADER,        "testgeom.glsl");
	shader.add(GL_FRAGMENT_SHADER,        "testfrag.glsl");
	shader.compile();


	treeShader.add(GL_VERTEX_SHADER,   "treesvert.glsl");
	treeShader.add(GL_GEOMETRY_SHADER, "treesgeom.glsl");
	treeShader.add(GL_FRAGMENT_SHADER, "treesfrag.glsl");
	treeShader.compile();


	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dist(0.0, HEIGHTMAP_SIZE.x);
	float* treePositions = new float[2*numTrees];
	for (int i = 0; i < numTrees; i++)
	{
		float x = dist(gen);
		float y = dist(gen);
		treePositions[i * 2] = x;
		treePositions[i * 2 + 1] = y;
	}
	treesPos = glm::ivec2(0);
	glGenVertexArrays(1, &tree_vao);
	glBindVertexArray(tree_vao);
	glGenBuffers(1, &tree_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, tree_vbo);
	glBufferData(GL_ARRAY_BUFFER, 2 * numTrees * sizeof(float), treePositions, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (GLvoid*)0);
	glBindVertexArray(0);
	delete[] treePositions;
	texTree.hint(GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	texTree.hint(GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	texTree.loadTexture("assets/textures/tree4.png", 1);


	textures["texGround"].loadTexture("assets/textures/ground.png");
	textures["texGrass"].loadTexture("assets/textures/grass.png");
	textures["texRockGround"].loadTexture("assets/textures/rockground.png");
	textures["texSnow"].loadTexture("assets/textures/snow.png");
	textures["texRockDisp"].loadTexture("assets/textures/rock_displacement.png");

	lastChunk.x = 1;

	float radius = 2;
	for (int i = -radius; i <= radius; i++)
	{
		int width = sqrt(radius*radius - i * i);
		for (int j = -width; j <= width; j++)
		{
			generateChunk(glm::ivec2(i, j));
		}
	}
}

bool Terrain::inCreation(glm::ivec2 pos)
{
	for (auto p : toCreate)
	{
		if (p == pos)
		{
			return true;
		}
	}

	for (auto& pair : creating)
	{
		if (pair.pos == pos)
		{
			return true;
		}
	}

	return false;
}

bool Terrain::alreadyCreated(glm::ivec2 pos)
{
	return maps.find(pos) != maps.end();
}

bool Terrain::canErode(glm::ivec2 pos)
{
	for (auto p : toErode)
	{
		if (p == pos)
		{
			return false;
		}
	}
	for (auto& pair : eroding)
	{
		if (pair.pos == pos)
		{
			return false;
		}
	}

	auto item = maps.find(pos);
	if (item != maps.end() && item->second && item->second->eroded)
	{
		return false;
	}

	return true;
}

bool Terrain::canCreate(glm::ivec2 pos)
{
	return !inCreation(pos) && !alreadyCreated(pos);
}

void Terrain::update(glm::vec3 pos)
{
	auto currChunk = getChunkPos(camera.position);
	if (currChunk != lastChunk)
	{
		lastChunk = currChunk;
		int radius = 1;
		for (int i = -radius; i <= radius; i++)
		{
			int width = sqrt(radius*radius - i * i);
			for (int j = -width; j <= width; j++)
			{
				generateChunk(currChunk + glm::ivec2(i, j));
			}
		}
	}
	

	while (!toGenerate.empty())
	{
		auto pos = toGenerate.front();
		toGenerate.pop();

		int hchunk = ERODE_CHUNKS / 2;
		for (int y = -hchunk; y <= hchunk; y++)
		{
			for (int x = -hchunk; x <= hchunk; x++)
			{
				glm::ivec2 p = pos + glm::ivec2(x, y);
				if (canCreate(p))
				{
					toCreate.push_back(p);
				}
			}
		}
		
		if (canErode(pos))
		{
			toErode.push_back(pos);
		}
	}

	int maxCreatingAsyncs = 2;
	int end = -1;
	for (int i = 0; i < toCreate.size() && creating.size() < maxCreatingAsyncs; i++)
	{
		end = i;
		auto pos = toCreate[i];
		std::cout << "Creating tile at: " << pos.x << ", " << pos.y << "\n";
		creating.push_back({ std::async(std::launch::async, [pos]() {
			return new Heightmap(pos);
		}), pos });
	}
	if (end >= 0)
	{
		toCreate.erase(toCreate.begin(), toCreate.begin() + end+1);;
	}


	for (int i = 0; i < creating.size(); i++)
	{
		auto& item = creating[i];
		auto status = item.future.wait_for(std::chrono::nanoseconds(1));

		if (status == std::future_status::ready)
		{
			auto hm = item.future.get();
			hm->upload();
			maps[item.pos] = hm;
			creating.erase(creating.begin() + i);
			i--;
		}
	}


	int maxErodingAsyncs = 1;
	for (int i = 0; i < toErode.size() && eroding.size() < maxErodingAsyncs; i++)
	{
		auto pos = toErode[i];
		
		bool canStart = true;

		Heightmap** param = new Heightmap*[ERODE_CHUNKS * ERODE_CHUNKS];
		for (int y = 0; y < ERODE_CHUNKS && canStart; y++)
		{
			for (int x = 0; x < ERODE_CHUNKS && canStart; x++)
			{
				auto p = pos + glm::ivec2(x, y) - glm::ivec2(ERODE_CHUNKS / 2);
				auto item = maps.find(p);
				if (item != maps.end())
				{
					param[x + y * ERODE_CHUNKS] = item->second;
				}
				else
				{
					canStart = false;
				}
			}
		}
		if (!canStart)
		{
			delete[] param;
			continue;
		}

		for (auto& pair : eroding)
		{
			auto dist = abs(pair.pos - pos);
			if (dist.x < ERODE_CHUNKS && dist.y < ERODE_CHUNKS)
			{
				canStart = false;
				break;
			}
		}

		if (canStart)
		{
			eroding.push_back({ std::async(std::launch::async, erodeCenterHeightmap, param), pos });

			toErode.erase(toErode.begin() + i);
			i--;
		}

	}


	for (int i = 0; i < eroding.size(); i++)
	{
		auto& item = eroding[i];
		auto status = item.future.wait_for(std::chrono::nanoseconds(1));

		if (status == std::future_status::ready)
		{
			auto pos = item.pos;

			for (int y = 0; y < ERODE_CHUNKS; y++)
			{
				for (int x = 0; x < ERODE_CHUNKS; x++)
				{
					auto p = pos + glm::ivec2(x, y) - glm::ivec2(ERODE_CHUNKS / 2);
					maps[p]->upload();
				}
			}
			eroding.erase(eroding.begin() + i);
			i--;
		}
	}

	double dt = dtimer.restart();
	camera.update(dt);

	auto iter = maps.find(lastChunk);

	//std::cout << "camera (" << camera.position.x << ", " << camera.position.z << ")\n";
	//std::cout << "chunk (" << chunkX << ", " << chunkY << ")\n\n";
	if (iter != maps.end())
	{
		if (iter->second)
		{
			float height = iter->second->heightAt(camera.position) + 2.f;
			if (camera.position.y < height)
			{
				camera.position.y = height;
			}
		}
	}
}

void Terrain::draw()
{
	glClearColor(47.f / 255, 141.f / 255, 255.f / 255, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glm::mat4 viewProj = camera.getViewProj();


	shader.use();
	shader.uniform("proj_view", viewProj);
	glBindVertexArray(patch_vao);
	glPatchParameteri(GL_PATCH_VERTICES, 1);

	shader.uniform("fov", camera.fov);
	shader.uniform("cameraPos", camera.position);
	shader.uniform("windowSize", Window::size());

	int id = 2;
	for (auto& item : textures)
	{
		item.second.bind(id);
		shader.uniform(item.first, id);
		id++;
	}

	int patches = MAX_PATCHES;
	shader.uniform("numPatches", float(patches));
	
	for (auto item : maps)
	{
		auto hm = item.second;
		if (hm)
		{
			hm->bind(shader);

			float pixelsPerHm = 200;
			glm::ivec2 hmpos = hm->getPos();
			glm::vec3 pos = HEIGHTMAP_SIZE * glm::vec3(hmpos.x + 0.5, 0, hmpos.y + 0.5);
			float dist = length(camera.position - pos);
			float b = dist * tan(camera.fov / 2.0);
			float ratio = HEIGHTMAP_SIZE.x / (2.0*b);
			float pixels = Window::size().x * ratio;
			float res = pixels / pixelsPerHm;
			
			float numPatches = glm::clamp(glm::pow(2.f, glm::ceil(glm::log(res) / glm::log(2.f))), 1.f, float(MAX_PATCHES));

			//numPatches = MAX_PATCHES;

			shader.uniform("numPatches", numPatches);

			shader.uniform("eroded", hm->eroded ? 1 : 0);
			float patchSize = hm->getSize().x / numPatches;
			shader.uniform("patchSize", patchSize);
			glDrawArrays(GL_PATCHES, 0, numPatches * numPatches);
		}
	}





	treeShader.use();
	treeShader.uniform("proj_view", viewProj);
	treeShader.uniform("cameraPos", camera.position);
	texTree.bind(1);
	treeShader.uniform("texTree", 1);
	for (auto item : maps)
	{
		auto hm = item.second;
		if (hm)
		{
			glm::ivec2 hmpos = hm->getPos();
			glm::vec3 pos = HEIGHTMAP_SIZE * glm::vec3(hmpos.x + 0.5, 0, hmpos.y + 0.5);
			float dist = length(camera.position - pos);
			if (dist < 15000.f)
			{
				hm->bind(treeShader);
				glBindVertexArray(tree_vao);
				glDrawArrays(GL_POINTS, 0, numTrees);
			}
		}
	}


	
}