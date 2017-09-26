#include "Time.h"
#include <iostream>
#include <chrono>

double Time::DeltaTime = 0;
double Time::TimeScale = 1;
double Time::RealtimeSinceStartup = 0;
double Time::UnscaledDeltaTime = 0;
int Time::FrameCount = 0;
high_resolution_clock::time_point Time::lastUpdateTimepoint = high_resolution_clock::now();

void Time::UpdateDeltaTime()
{
	double deltaTime = duration_cast<duration<double>>(high_resolution_clock::now() - lastUpdateTimepoint).count();
	DeltaTime = deltaTime * TimeScale;
	UnscaledDeltaTime = deltaTime;
	RealtimeSinceStartup += deltaTime;
	FrameCount++;
	//std::cout << "Avg FPS since startup: " << (FrameCount / RealtimeSinceStartup) << std::endl;
	lastUpdateTimepoint = high_resolution_clock::now();
}
