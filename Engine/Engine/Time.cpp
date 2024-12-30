#include "Time.h"

Time::Time()
	: gameTimer(new Timer()), realTimer(new Timer()), state(GameState::STOP),
	frameCount(0), timeSinceStartup(0), timeScale(1.0f), deltaTime(0), realTimeSinceStartup(0), realDeltaTime(0), hasStarted(false)
{
}

Time::~Time()
{
	delete gameTimer;
	delete realTimer;
}

void Time::Start()
{
	gameTimer->Start();
	realTimer->Start();
}

void Time::Update()
{
	if (!hasStarted)
	{
		deltaTime = 0;
		realDeltaTime = 0;
		return;
	}

	realDeltaTime = realTimer->ReadMs();
	realTimeSinceStartup += realDeltaTime / 1000.0f;
	realTimer->Reset();

	if (state == GameState::PLAY || state == GameState::STEP)
	{
		deltaTime = realDeltaTime * timeScale;
		timeSinceStartup += deltaTime / 1000.0f;
		frameCount++;

		if (state == GameState::STEP)
		{
			state = GameState::PAUSE;
		}
	}
	else
	{
		deltaTime = 0;
	}
}

void Time::Play()
{
	if (state == GameState::STOP)
	{
		state = GameState::PLAY;
		hasStarted = true;
		Start();
	}
	else if (state == GameState::PLAY || state == GameState::PAUSE)
	{
		Stop();
	}
}

void Time::Pause()
{
	if (state == GameState::PLAY)
		state = GameState::PAUSE;
	else if (state == GameState::PAUSE)
		state = GameState::PLAY;
}

void Time::Stop()
{
	state = GameState::STOP;
	timeSinceStartup = 0;
	frameCount = 0;
	realTimeSinceStartup = 0;
	hasStarted = false;
}

void Time::Step()
{
	if (state == GameState::PAUSE)
		state = GameState::STEP;
}
