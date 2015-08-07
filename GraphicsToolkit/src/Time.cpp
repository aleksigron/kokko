#include "Time.h"

#define GLFW_INCLUDE_NONE
#include "glfw/glfw3.h"

Time* Time::instance = nullptr;

Time::Time() :
	currentTime(glfwGetTime()),
	currentDelta(0.0)
{
	Time::instance = this;
}

void Time::Update()
{
	double previousTime = this->frameStart;
	this->frameStart = glfwGetTime();
	
	this->currentTime = float(this->frameStart);
	this->currentDelta = float(this->frameStart - previousTime);
}