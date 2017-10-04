#include "Time.h"
#include <iostream>
#include <chrono>


double MyTime::DeltaTime = 0;
double MyTime::TimeScale = 1;
double MyTime::RealtimeSinceStartup = 0;
double MyTime::UnscaledDeltaTime = 0;
int MyTime::FrameCount = 0;
high_resolution_clock::time_point MyTime::lastUpdateTimepoint = high_resolution_clock::now();

void MyTime::UpdateDeltaTime()
{
	double deltaTime = duration_cast<duration<double>>(high_resolution_clock::now() - lastUpdateTimepoint).count();
	DeltaTime = deltaTime * TimeScale;
	UnscaledDeltaTime = deltaTime;
	RealtimeSinceStartup += deltaTime;
	FrameCount++;
	//std::cout << "Avg FPS since startup: " << (FrameCount / RealtimeSinceStartup) << std::endl;
	lastUpdateTimepoint = high_resolution_clock::now();
}

