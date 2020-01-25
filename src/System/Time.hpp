#pragma once

class Time
{
private:
	static Time* instance;

	unsigned int frameNumber;
	double frameStart;
	double currentTime;
	double currentDelta;
	float currentDeltaFloat;
	
public:
	Time();
	
	void Update();

	static unsigned int GetFrameNumber() { return Time::instance->frameNumber; }
	static float GetDeltaTime() { return Time::instance->currentDeltaFloat; }
	static double GetRunningTime() { return Time::instance->currentTime; }
};
