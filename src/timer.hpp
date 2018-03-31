#pragma once

#include <chrono>


class Timer
{
	std::chrono::time_point<std::chrono::high_resolution_clock> start;
public:
	Timer();
	double restart();
	double elapsed();
};