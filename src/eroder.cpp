#include "eroder.hpp"
#include "misc.hpp"
#include <iostream>


void joinBordersLR(Heightmap* left, Heightmap* right)
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

void joinBordersUD(Heightmap* up, Heightmap* down)
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

void smoothenCenter(Heightmap * maps[3 * 3])
{
	const int side = 2*HEIGHTMAP_RESOLUTION;
	float temp[side * side];

	int kernelSize = 9;
	float kernel[] = { 0.000229,	0.005977,	0.060598,	0.241732,	0.382928,	0.241732,	0.060598,	0.005977,0.000229 };

	for (int y = 0; y < side; y++)
	{
		for (int x = 0; x < side; x++)
		{
			int chunkX = (x + HEIGHTMAP_RESOLUTION / 2) / HEIGHTMAP_RESOLUTION;
			int chunkY = (x + HEIGHTMAP_RESOLUTION / 2) / HEIGHTMAP_RESOLUTION;


			float smoothed = 0.f;
			for (int i = 0; i < kernelSize; i++)
			{
				int offset = i - kernelSize / 2;

				float height = maps[chunkX + chunkY * 3]->heightAt(x, y + offset);
				smoothed += kernel[i] * height;
			}
			temp[x + y * HEIGHTMAP_RESOLUTION] = smoothed;

		}
	}

	for (int y = 0; y < side; y++)
	{
		for (int x = 0; x < side; x++)
		{
			float smoothed = 0.f;
			for (int i = 0; i < kernelSize; i++)
			{
				int offset = i - kernelSize / 2;

				smoothed += kernel[i] * temp[index(x + offset, y, HEIGHTMAP_RESOLUTION)];
			}
			for (int iy = 0; iy < 3; iy++)
			{
				for (int ix = 0; ix < 3; ix++)
				{
					int xpos = x - (ix - 1) * (HEIGHTMAP_RESOLUTION - 3);
					int ypos = y - (iy - 1) * (HEIGHTMAP_RESOLUTION - 3);
					maps[ix + iy * 3]->setHeightAt(xpos, ypos, smoothed);
				}
			}
		}
	}
}


void erodeCenterHeightmap(Heightmap * maps[ERODE_CHUNKS*ERODE_CHUNKS])
{
	int centerIndex = index(ERODE_CHUNKS/2, ERODE_CHUNKS/2, ERODE_CHUNKS);
	auto center = maps[centerIndex]->getPos();
	auto size = HEIGHTMAP_SIZE;

	for (int i = 0; i < HEIGHTMAP_MAX_ITERATIONS; i++)
	{
		glm::vec2 vel;
		glm::vec2 pos;
		float rn1 = random()*1.2f - 0.1f;
		float rn2 = random()*1.2f - 0.1f;
		pos.x = size.x * rn1 + center.x * size.x;
		pos.y = size.z * rn2 + center.y * size.z;


		float stepSize = 1.0f;
		float erosionRatio = 0.3f;
		float depositRatio = 0.7f;
		float erodeSize = 0.f;

		float sediment = 0.f;

		float pixelSize = size.x / float(HEIGHTMAP_RESOLUTION);


		for (int j = 0; j < 100; j++)
		{
			int chunkX = glm::floor(pos.x / HEIGHTMAP_SIZE.x);
			int chunkY = glm::floor(pos.y / HEIGHTMAP_SIZE.z);
			chunkX += ERODE_CHUNKS / 2 -center.x;
			chunkY += ERODE_CHUNKS / 2 -center.y;

			if (chunkX < 1 || chunkX > 3 || chunkY < 1 || chunkY > 3)
			{
				break;
			}

			auto current = maps[index(chunkX, chunkY, ERODE_CHUNKS)];

			glm::vec2 dir = current->gradientAt(pos);
			vel += 0.5f * dir;
			glm::vec2 step = stepSize * pixelSize * normalize(vel);
			vel *= 0.5f;

			if (length(vel) < 0.000001f)
			{
				break;
			}

			float currHeight = current->heightAt(pos);
			float nextHeight = current->heightAt(pos + step);

			float eroded = erosionRatio * glm::max(currHeight - nextHeight, 0.f) * glm::smoothstep(0.f, 5.f, float(j));
			eroded -= depositRatio * sediment;

			glm::vec2 ortho = pixelSize * normalize(glm::mat2(0, 1, -1, 0) * step);
			for (float side = -erodeSize; side <= erodeSize; side += 1.f)
			{
				float weight = glm::smoothstep(erodeSize, abs(side), 0.f);
				weight = pow(weight, 0.5);
				weight = 1.f;

				glm::vec2 offset = side * ortho;

				// TODO: only erode chunks close to pos + offset, 2 or 4
				for (int i = 0; i < ERODE_CHUNKS * ERODE_CHUNKS; i++)
				{
					maps[i]->addHeightAt(pos + offset, -eroded * weight);
				}
			}


			sediment += eroded;

			pos += step;
		}


		if (i % (HEIGHTMAP_MAX_ITERATIONS / 100) == 0)
		{
			for (int i = 0; i < ERODE_CHUNKS; i++)
			{
				for (int j = 0; j < ERODE_CHUNKS-1; j++)
				{
					joinBordersLR(maps[index(j, i, ERODE_CHUNKS)], maps[index(j + 1, i, ERODE_CHUNKS)]);

					joinBordersUD(maps[index(i, j, ERODE_CHUNKS)], maps[index(i, j + 1, ERODE_CHUNKS)]);
				}
			}
		}
	}

	for (int i = 1; i < 4; i++)
	{
		for (int j = 1; j < 4; j++)
		{
			maps[index(i, j, ERODE_CHUNKS)]->smoothen();
			maps[index(i, j, ERODE_CHUNKS)]->addDetail();
			maps[index(i, j, ERODE_CHUNKS)]->addRockRoughness();
		}
	}

	for (int i = 0; i < ERODE_CHUNKS; i++)
	{
		for (int j = 0; j < ERODE_CHUNKS - 1; j++)
		{
			joinBordersLR(maps[index(j, i, ERODE_CHUNKS)], maps[index(j + 1, i, ERODE_CHUNKS)]);

			joinBordersUD(maps[index(i, j, ERODE_CHUNKS)], maps[index(i, j + 1, ERODE_CHUNKS)]);
		}
	}

	maps[centerIndex]->eroded = true;


	delete[] maps;
}
