#include "clouds.hpp"

#include "FastNoise.h"
#include <glm\glm.hpp>



bool isInside(glm::ivec3 o, int size)
{
	return o.x >= 0 && o.y >= 0 && o.z >= 0 
		&& o.x < size && o.y < size && o.z < size;
}


void compare(glm::vec3* grid, int size, glm::vec3& p, int px, int py, int pz, int x, int y, int z)
{
	glm::ivec3 o(px + x, py + y, pz + z);

	if (isInside(o, size))
	{
		int i = o.x + o.y * size + o.z * size * size;
		auto other = grid[i];
		other.x += x;
		other.y += y;
		other.z += z;

		if (length(other) < length(p))
		{
			p = other;
		}
	}
}

void chooseOffset(glm::vec3* grid, int size, int px, int py, int pz)
{
	int index = px + py * size + pz * size*size;
	auto p = grid[index];

	for (int z = -1; z <= 1; z++)
	{
		for (int y = -1; y <= 1; y++)
		{
			for (int x = -1; x <= 1; x++)
			{
				if (x == 0 && y == 0 & z == 0)
					continue;
				compare(grid, size, p, px, py, pz, x, y, z);
			}
		}
	}
	grid[index] = p;
}

void Clouds::init()
{

	FastNoise noise;
	noise.SetNoiseType(FastNoise::NoiseType::SimplexFractal);

	int size = 512;
	glm::vec3* grid = new glm::vec3[size * size * size];

	for (int z = 0; z < size; z++)
	{
		for (int y = 0; y < size; y++)
		{
			for (int x = 0; x < size; x++)
			{
				float ns = 0.5*noise.GetNoise(x, y, z) + 0.5;
				if (ns >= 0.5f)
				{
					// outside
					ns = size * size;
				}
				else
				{
					// inside
					ns = 0.f;
				}
				grid[x + y * size + z * size*size] = glm::vec3(ns);
			}
		}
	}



	for (int z = 0; z < size; z++)
	{
		for (int y = 0; y < size; y++)
		{
			for (int x = 0; x < size; x++)
			{
				chooseOffset(grid, size, x, y, z);
			}
		}
	}



}

void Clouds::draw()
{
}
