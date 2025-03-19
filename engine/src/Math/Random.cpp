#include "Math/Random.hpp"

namespace kokko
{

thread_local std::default_random_engine Random::randomEngine;

void Random::Seed(unsigned int seed)
{
	randomEngine.seed(seed);
}

float Random::Float01()
{
	return std::uniform_real_distribution<float>(0.0f, 1.0f)(randomEngine);
}

float Random::Float(float min, float max)
{
	return std::uniform_real_distribution<float>(min, max)(randomEngine);
}

double Random::Double01()
{
	return std::uniform_real_distribution<double>(0.0, 1.0)(randomEngine);
}

double Random::Double(double min, double max)
{
	return std::uniform_real_distribution<double>(min, max)(randomEngine);
}

int Random::Int(int min, int max)
{
	return std::uniform_int_distribution<int>(min, max)(randomEngine);
}

unsigned int Random::Uint(unsigned int min, unsigned int max)
{
	return std::uniform_int_distribution<unsigned int>(min, max)(randomEngine);
}

int64_t Random::Int64(int64_t min, int64_t max)
{
	return std::uniform_int_distribution<int64_t>(min, max)(randomEngine);
}

uint64_t Random::Uint64(uint64_t min, uint64_t max)
{
	return std::uniform_int_distribution<uint64_t>(min, max)(randomEngine);
}

} // namespace kokko
