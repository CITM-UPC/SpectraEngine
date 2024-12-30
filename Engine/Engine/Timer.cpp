#include "Timer.h"

Timer::Timer()
{
	Start();
}

void Timer::Start()
{
	frequency = SDL_GetPerformanceFrequency();
	startTime = SDL_GetPerformanceCounter();
}

float Timer::ReadMs() const
{
	return ((float)(SDL_GetPerformanceCounter() - startTime) / frequency * 1000.0f);
}

void Timer::Reset()
{
	startTime = SDL_GetPerformanceCounter();
}