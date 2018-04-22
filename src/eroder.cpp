#include "eroder.hpp"
#include "misc.hpp"
#include <iostream>


void smoothBordersLR(Heightmap* left, Heightmap* right)
{
	//float temp[HEIGHTMAP_RESOLUTION * 7];

	for (int i = 0; i < 5; i++)
	{
		for (float j = 0; j < HEIGHTMAP_RESOLUTION; j++)
		{
			int leftx = HEIGHTMAP_RESOLUTION - 4 + i;
			int lefty = j;

			int rightx = i-1;
			int righty = j;

			float leftHeight = left->heightAt(leftx, lefty);
			float rightHeight = right->heightAt(rightx, righty);

			float weight = glm::smoothstep(0.f, 4.f, float(i));

			left->setHeightAt(leftx, lefty, leftHeight * (1.f - weight) + rightHeight * weight);
			right->setHeightAt(rightx, righty, leftHeight * (1.f - weight) + rightHeight * weight);
		}
	}
}

void smoothBordersUD(Heightmap* up, Heightmap* down)
{
	for (int i = 0; i < 5; i++)
	{
		for (float j = 0; j < HEIGHTMAP_RESOLUTION; j++)
		{
			int upx = j;
			int upy = HEIGHTMAP_RESOLUTION - 4 + i;

			int downx = j;
			int downy = i-1;

			float upHeight = up->heightAt(upx, upy);
			float downHeight = down->heightAt(downx, downy);

			float weight = glm::smoothstep(0.f, 4.f, float(i));

			up->setHeightAt(upx, upy, upHeight * (1.f - weight) + downHeight * weight);
			down->setHeightAt(downx, downy, upHeight * (1.f - weight) + downHeight * weight);
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


		if (i % (HEIGHTMAP_MAX_ITERATIONS / 100) == 0)
		{
			for (int i = 0; i < 3; i++)
			{
				smoothBordersLR(maps[0 + i * 3], maps[1 + i * 3]);
				smoothBordersLR(maps[1 + i * 3], maps[2 + i * 3]);

				smoothBordersUD(maps[i + 0 * 3], maps[i + 1 * 3]);
				smoothBordersUD(maps[i + 1 * 3], maps[i + 2 * 3]);
			}
		}
	}

	for (int i = 0; i < 3; i++)
	{
		smoothBordersLR(maps[0 + i * 3], maps[1 + i * 3]);
		smoothBordersLR(maps[1 + i * 3], maps[2 + i * 3]);

		smoothBordersUD(maps[i + 0 * 3], maps[i + 1 * 3]);
		smoothBordersUD(maps[i + 1 * 3], maps[i + 2 * 3]);
	}

	maps[1 + 1 * 3]->eroded = true;


	delete[] maps;
	// return center pos
	return center;
}
