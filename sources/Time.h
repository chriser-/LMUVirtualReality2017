#pragma once
#include <chrono>

using namespace std::chrono;

class Time
{
public:
	static double DeltaTime;
	static double TimeScale;
	static double RealtimeSinceStartup;
	static double UnscaledDeltaTime;
	static int FrameCount;
	static void UpdateDeltaTime();
private:
	static high_resolution_clock::time_point lastUpdateTimepoint;
};
