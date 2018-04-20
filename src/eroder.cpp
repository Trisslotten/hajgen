#include "eroder.hpp"
#include "misc.hpp"
#include <iostream>


void smoothBordersLR(Heightmap* left, Heightmap* right)
{
	float step = HEIGHTMAP_SIZE.x / float(HEIGHTMAP_RESOLUTION);
	
	float temp[HEIGHTMAP_RESOLUTION * 7];

	for (float ix = -1.5f; ix <= 1.5f; ix+=0.5f)
	{
		for (float iy = 0; iy < HEIGHTMAP_RESOLUTION; iy++)
		{
			
			float x = step * ix + HEIGHTMAP_SIZE.x * float(right->getPos().x);
			float y = HEIGHTMAP_SIZE.z * iy / float(HEIGHTMAP_RESOLUTION) + HEIGHTMAP_SIZE.z * float(right->getPos().y);

			glm::vec2 pos = glm::vec2(x, y);

			float lh = left->heightAt(pos);
			float rh = right->heightAt(pos);

			float weight = glm::smoothstep(-1.5f, 1.5f, ix);
			float height = rh * weight + lh * (1.f - weight);


			left->setHeightAt(pos, height);
			right->setHeightAt(pos, height);
			temp[int(ix) + 3 + int(iy) * 3] = height;
		}
	}
	/*
	for (float ix = -3.0f; ix <= 3.0f; ix += 1.0f)
	{
		for (float iy = 0; iy < HEIGHTMAP_RESOLUTION; iy++)
		{

			float x = step * ix + HEIGHTMAP_SIZE.x * float(right->getPos().x);
			float y = HEIGHTMAP_SIZE.z * iy / float(HEIGHTMAP_RESOLUTION) + HEIGHTMAP_SIZE.z * float(right->getPos().y);

			glm::vec2 pos = glm::vec2(x, y);

			float height = temp[int(ix) + 3 + int(iy) * 3];

			left->setHeightAt(pos, height);
			right->setHeightAt(pos, height);
		}
	}
	*/
}

void smoothBordersUD(Heightmap* up, Heightmap* down)
{
	float step = HEIGHTMAP_SIZE.x / float(HEIGHTMAP_RESOLUTION);

	float temp[HEIGHTMAP_RESOLUTION * 7];

	for (float iy = -1.5f; iy <= 1.5f; iy += 0.5f)
	{
		for (float ix = 0; ix < HEIGHTMAP_RESOLUTION; ix++)
		{

			float y = step * iy + HEIGHTMAP_SIZE.x * float(down->getPos().y);
			float x = HEIGHTMAP_SIZE.x * ix / float(HEIGHTMAP_RESOLUTION) + HEIGHTMAP_SIZE.x * float(down->getPos().x);

			glm::vec2 pos = glm::vec2(x, y);

			float uh = up->heightAt(pos);
			float dh = down->heightAt(pos);

			float weight = glm::smoothstep(-1.5f, 1.5f, ix);
			float height = uh * weight + dh * (1.f - weight);

			up->setHeightAt(pos, height);
			down->setHeightAt(pos, height);
		}
	}
}

glm::ivec2 erodeCenterHeightmap(Heightmap * maps[3 * 3])
{
	auto center = maps[1 + 1 * 3]->getPos();
	auto size = HEIGHTMAP_SIZE;

	for (int i = 0; i < HEIGHTMAP_MAX_ITERATIONS; i++)
	{

		glm::vec2 vel;
		glm::vec2 pos;
		float rn1 = random()*1.2f - 0.1f;
		float rn2 = random()*1.2f - 0.1f;
		pos.x = size.x * rn1 + center.x * size.x;
		pos.y = size.z * rn2 + center.y * size.z;


		float stepSize = 1.f;
		float erosionRatio = 0.6f;
		float depositRatio = 0.5f;


		float sediment = 0.f;


		for (int j = 0; j < 100; j++)
		{
			if (pos.x <= 0 || pos.x >= size.x || pos.y <= 0 || pos.y >= size.z)
			{
				//break;
			}
			int chunkX = glm::floor(pos.x / HEIGHTMAP_SIZE.x);
			int chunkY = glm::floor(pos.y / HEIGHTMAP_SIZE.z);
			chunkX += 1-center.x;
			chunkY += 1-center.y;

			auto current = maps[chunkX + chunkY * 3];

			glm::vec2 dir = current->gradientAt(pos);
			vel += 0.5f * dir;
			glm::vec2 step = stepSize * size.x * normalize(vel) / float(HEIGHTMAP_RESOLUTION);
			vel *= 0.5f;

			if (length(vel) < 0.000001f)
			{
				break;
			}

			float currHeight = current->heightAt(pos);
			float nextHeight = current->heightAt(pos + step);

			float eroded = erosionRatio * glm::max(currHeight - nextHeight, 0.f) * glm::smoothstep(0.f, 5.f, float(j));
			eroded -= depositRatio * sediment;

			for (int i = 0; i < 9; i++)
			{
				maps[i]->addHeightAt(pos, -eroded);
			}

			sediment += eroded;

			pos += step;
		}

		if (sediment > 0)
		{
			//addHeightAt(pos, sediment);
		}
	}

	for (int i = 0; i < 3; i++)
	{
		smoothBordersLR(maps[0 + i * 3], maps[1 + i * 3]);
		smoothBordersLR(maps[1 + i * 3], maps[2 + i * 3]);

		smoothBordersUD(maps[i + 0 * 3], maps[i + 1 * 3]);
		smoothBordersUD(maps[i + 1 * 3], maps[i + 2 * 3]);
	}
	

	float x = size.x*0.5f + center.x * size.x;
	float y = size.z*0.5f + center.y * size.z;

	for (int i = 0; i < 100; i++)
	{

	}


	delete[] maps;
	// return center pos
	return center;
}
