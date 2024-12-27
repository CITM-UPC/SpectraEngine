#include "ModuleWindow.h"
#include "App.h"

ModuleWindow::ModuleWindow(App* app) : Module(app), window(NULL), screenSurface(NULL), width(SCREEN_WIDTH), height(SCREEN_HEIGHT), context(NULL)
{
}

ModuleWindow::~ModuleWindow()
{
}

bool ModuleWindow::Awake()
{
	LOG(LogType::LOG_INFO, "Init SDL window & surface");
	bool ret = true;

	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		LOG(LogType::LOG_ERROR, "SDL_VIDEO could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}
	else
	{
		width = SCREEN_WIDTH;
		height = SCREEN_HEIGHT;
		Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

		if (fullscreen)
			flags |= SDL_WINDOW_FULLSCREEN;

		if (resizable)
			flags |= SDL_WINDOW_RESIZABLE;

		if (borderless)
			flags |= SDL_WINDOW_BORDERLESS;

		if (fulldesktop)
			flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

		window = SDL_CreateWindow("SpectraEngine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);

		if (window == NULL)
		{
			LOG(LogType::LOG_ERROR, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
			ret = false;
		}
		else
		{
			screenSurface = SDL_GetWindowSurface(window);
		}

		context = SDL_GL_CreateContext(window);
	}

	return ret;
}

bool ModuleWindow::CleanUp()
{
	LOG(LogType::LOG_INFO, "Destroying SDL window and quitting all SDL systems");

	if (context != nullptr)
	{
		SDL_GL_DeleteContext(context);
		context = nullptr;
	}
	if (window != nullptr)
	{
		SDL_DestroyWindow(window);
		window = nullptr;
	}

	screenSurface = nullptr;

	SDL_Quit();
	return true;
}

void ModuleWindow::SetFullScreen(bool enabled)
{
	if (enabled)
		SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
	else
		SDL_SetWindowFullscreen(window, 0);
}

void ModuleWindow::SetBorderless(bool enabled)
{
	SDL_SetWindowBordered(window, enabled ? SDL_FALSE : SDL_TRUE);
}

void ModuleWindow::SetFullDesktop(bool enabled)
{
	if (enabled)
		SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
	else
		SDL_SetWindowFullscreen(window, 0);
}

void ModuleWindow::SetResizable(bool enabled)
{
	SDL_SetWindowResizable(window, resizable ? SDL_TRUE : SDL_FALSE);
}