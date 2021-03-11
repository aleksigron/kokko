#pragma once

class Time
{
private:
	static Time* instance;
	static const double MinDeltaTime;
	static const double MaxDeltaTime;

	// 32-bit frame number can roll over if you run at 1000 fps for 50 days
	// TODO: Should this be upgraded to 64-bit?
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
