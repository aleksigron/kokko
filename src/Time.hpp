#pragma once

class Time
{
private:
	static Time* instance;
	
	double frameStart;
	double currentTime;
	double currentDelta;
	float currentDeltaFloat;
	
public:
	Time();
	
	void Update();
	
	static float GetDeltaTime() { return Time::instance->currentDeltaFloat; }
	static double GetRunningTime() { return Time::instance->currentTime; }
};