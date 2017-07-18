#include "Time.hpp"

#include "IncludeGLFW.hpp"

Time* Time::instance = nullptr;

Time::Time() :
	frameStart(glfwGetTime()),
	currentDelta(0.0f)
{
	this->currentTime = frameStart;

	Time::instance = this;
}

void Time::Update()
{
	double previousTime = this->frameStart;
	this->frameStart = glfwGetTime();
	
	this->currentTime = float(this->frameStart);
	this->currentDelta = float(this->frameStart - previousTime);
}