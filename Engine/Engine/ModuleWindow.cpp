#include "ModuleWindow.h"
#include "App.h"

ModuleWindow::ModuleWindow(App* app) : Module(app), window(nullptr), screenSurface(nullptr), width(SCREEN_WIDTH), height(SCREEN_HEIGHT), context(nullptr)
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
		Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN;

		window = SDL_CreateWindow("SpectraEngine", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 600, 400, flags);

		SDL_SetWindowBordered(window, SDL_FALSE);

		if (window == nullptr)
		{
			LOG(LogType::LOG_ERROR, "Window could not be created! SDL_Error: %s\n", SDL_GetError());
			ret = false;
		}
		else
		{
			renderer = SDL_CreateRenderer(window, -1, 0);
			screenSurface = SDL_LoadBMP("Engine/Textures/loading_screen.bmp");
			loadingBarTexture = SDL_CreateTextureFromSurface(renderer, screenSurface);
			screenSurface = SDL_LoadBMP("Engine/Textures/loading_bar.bmp");
			backgroundTexture = SDL_CreateTextureFromSurface(renderer, screenSurface);
			SDL_RenderCopy(renderer, loadingBarTexture, nullptr, nullptr);
			SDL_RenderCopy(renderer, backgroundTexture, nullptr, nullptr);
			SDL_RenderPresent(renderer);

			context = SDL_GL_CreateContext(window);
		}
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

bool ModuleWindow::StartWindow()
{
	bool ret = true;

	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);

	SDL_DestroyTexture(loadingBarTexture);
	SDL_DestroyTexture(backgroundTexture);
	SDL_FreeSurface(screenSurface);
	SDL_DestroyRenderer(renderer);

	width = SCREEN_WIDTH;
	height = SCREEN_HEIGHT;

	SDL_DisplayMode currentDisplayMode;
	if (SDL_GetCurrentDisplayMode(0, &currentDisplayMode) == 0)
	{
		int x = (currentDisplayMode.w - width) / 2;
		int y = (currentDisplayMode.h - height) / 2;

		SDL_SetWindowPosition(window, x, y);
	}

	SDL_SetWindowSize(window, width, height);
	SDL_SetWindowBordered(window, SDL_TRUE);

	if (fullscreen)
		SetFullScreen(fullscreen);

	if (fulldesktop)
		SetFullDesktop(fulldesktop);

	if (resizable)
		SetResizable(resizable);

	if (borderless)
		SetBorderless(borderless);

	screenSurface = SDL_GetWindowSurface(window);

	return ret;
}

void ModuleWindow::RenderInitialScreen()
{
	SDL_Rect rect = { 0, 350 - loadingBarWidth * loadingBarPercentage, 600, loadingBarWidth };
	loadingBarPercentage++;
	SDL_SetRenderDrawColor(renderer, 160, 160, 180, 255);
	SDL_RenderFillRect(renderer, &rect);
	SDL_RenderCopy(renderer, loadingBarTexture, nullptr, nullptr);
	SDL_RenderPresent(renderer);
	SDL_Delay(60);
}