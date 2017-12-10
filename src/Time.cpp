#include "Time.hpp"

#include "IncludeGLFW.hpp"

Time* Time::instance = nullptr;

Time::Time() :
	frameStart(glfwGetTime()),
	currentDeltaFloat(0.0f)
{
	this->currentTime = frameStart;

	Time::instance = this;
}

void Time::Update()
{
	double previousTime = this->frameStart;
	this->frameStart = glfwGetTime();
	
	this->currentTime = this->frameStart;
	this->currentDelta = this->frameStart - previousTime;
	this->currentDeltaFloat = static_cast<float>(this->currentDelta);
}