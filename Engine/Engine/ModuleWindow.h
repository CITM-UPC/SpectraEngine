#pragma once

#include "Module.h"
#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"

#define SCREEN_WIDTH 1280
#define SCREEN_HEIGHT 720

#define WIN_FULLSCREEN 0
#define WIN_FULLSCREEN_DESKTOP 0
#define WIN_BORDERLESS 0
#define WIN_RESIZABLE 1

class ModuleWindow : public Module
{
public:
	ModuleWindow(App* app);
	virtual ~ModuleWindow();

	bool Awake();
	bool CleanUp();

	void SetFullScreen(bool enabled);
	void SetBorderless(bool enabled);
	void SetFullDesktop(bool enabled);
	void SetResizable(bool enabled);

	void OnResize(int width, int height) { this->width = width; this->height = height; }

	bool StartWindow();
	void RenderInitialScreen();

public:
	SDL_Window* window;
	SDL_GLContext context;
	SDL_Surface* screenSurface;

	SDL_Renderer* renderer = nullptr;
	SDL_Texture* backgroundTexture = nullptr;
	SDL_Texture* loadingBarTexture = nullptr;

	int loadingBarWidth = 0;
	int loadingBarPercentage = 0;

	int width;
	int height;

	bool fullscreen = WIN_FULLSCREEN;
	bool borderless = WIN_BORDERLESS;
	bool fulldesktop = WIN_FULLSCREEN_DESKTOP;
	bool resizable = WIN_RESIZABLE;
};