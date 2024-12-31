#pragma once

#include "SDL2/SDL.h"

class Timer
{
public:
	Timer();

	void Start();
	float ReadMs() const;
	void Reset();

private:
	Uint64 startTime;
	Uint64 frequency;
};