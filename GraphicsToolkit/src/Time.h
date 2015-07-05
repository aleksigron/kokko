#pragma once

#define GLFW_INCLUDE_NONE
#include "glfw/glfw3.h"

class Time
{
private:
	static Time* instance;
	
	double frameStart;
	float currentTime;
	float currentDelta;
	
public:
	Time();
	
	void Update();
	
	static inline float GetDeltaTime() { return Time::instance->currentDelta; }
	static inline float GetTime() { return Time::instance->currentTime; }
};