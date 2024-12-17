#pragma once

#include "EditorWindow.h"
#include <windows.h>
#include "GL/glew.h"

class PerformanceWindow : public EditorWindow
{
public:
	PerformanceWindow(const WindowType type, const std::string& name);
	~PerformanceWindow();

	void DrawWindow() override;

private:
	// CPU
	std::string cpuName;
	SYSTEM_INFO sysInfo;

	// GPU
	const GLubyte* vendor;
	const GLubyte* renderer;
	const GLubyte* version;
	const GLubyte* glsl;

	// FPS Tracking
	float minFps = FLT_MAX;
	float maxFps = 0.0f;
	float totalFps = 0.0f;
	int frameCount = 0;
	int totalFrameCount = 0;
	float dt = 0;
	float currentFps = 0;

	static const int FPS_HISTORY_SIZE = 100;
	float fpsHistory[FPS_HISTORY_SIZE] = {};
	int fpsHistoryOffset = 0;
	const char* fpsOptions[6] = { "30", "60", "90", "120", "144", "240" };
};