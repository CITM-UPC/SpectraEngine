#pragma once

#include "Timer.h"

enum class GameState
{
	PLAY,
	PAUSE,
	STOP,
	STEP
};

class Time
{
public:
	Time();
	~Time();

	void Start();
	void Update();

	void Play();
	void Pause();
	void Stop();
	void Step();

	float GetDeltaTime() const { return deltaTime; }
	float GetTimeSinceStartup() const { return timeSinceStartup; }
	float GetRealDeltaTime() const { return realDeltaTime; }
	float GetRealTimeSinceStartup() const { return realTimeSinceStartup; }
	float GetFrameCount() const { return frameCount; }
	float GetTimeScale() const { return timeScale; }
	GameState GetState() const { return state; }

	void SetTimeScale(float scale) { timeScale = scale; }
	void SetState(GameState newState) { state = newState; }

	bool IsPlaying() const { return hasStarted; }

private:
	Timer* gameTimer;
	Timer* realTimer;

	GameState state;

	float frameCount;
	float timeSinceStartup;
	float timeScale;
	float deltaTime;
	float realTimeSinceStartup;
	float realDeltaTime;

	bool hasStarted;
};
