#include "System/Time.hpp"

#include <cmath>

#include "System/IncludeGLFW.hpp"

namespace kokko
{

Time* Time::instance = nullptr;
const double Time::MinDeltaTime = 0.0001;
const double Time::MaxDeltaTime = 0.1;

Time::Time()
{
	frameNumber = 0;
	frameStart = glfwGetTime();
	currentTime = frameStart;
	currentDelta = MinDeltaTime;
	currentDeltaFloat = static_cast<float>(currentDelta);

	Time::instance = this;
}

void Time::Update()
{
	double previousTime = frameStart;
	frameStart = glfwGetTime();

	currentTime = frameStart;
	currentDelta = std::fmin(std::fmax(frameStart - previousTime, MinDeltaTime), MaxDeltaTime);
	currentDeltaFloat = static_cast<float>(currentDelta);

	++frameNumber;
}

} // namespace kokko
