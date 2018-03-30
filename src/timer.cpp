#include "timer.hpp"

Timer::Timer() {
	start = std::chrono::high_resolution_clock::now();
}

double Timer::restart() {
	std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
	double diff = std::chrono::duration_cast<std::chrono::nanoseconds>(now - start).count();
	start = std::chrono::high_resolution_clock::now();
	return diff / 1000000000.0;
}

double Timer::elapsed() {
	std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
	double diff = std::chrono::duration_cast<std::chrono::nanoseconds>(now - start).count();
	return diff / 1000000000.0;
}
