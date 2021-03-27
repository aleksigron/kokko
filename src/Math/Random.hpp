#pragma once

#include <random>
#include <cstdint>

class Random
{
private:
	static std::default_random_engine randomEngine;

public:
	static void Seed(unsigned int seed);

	static float Float01();
	static float Float(float min, float max);

	static double Double01();
	static double Double(double min, double max);

	static int Int(int min, int max);
	static unsigned int Uint(unsigned int min, unsigned int max);

	static int64_t Int64(int64_t min, int64_t max);
	static uint64_t Uint64(uint64_t min, uint64_t max);
};
