#pragma once

#include "Timer.h"

class InfoTag
{
public:
	InfoTag();
	void ShowInfoTag(const char* text, const float time = 700.0f, const float wrapPos = 35.0f);

private:
	Timer timer;
	bool hoverStarted;
};