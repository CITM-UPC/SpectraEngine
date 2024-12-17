#include "ModuleInput.h"
#include "App.h"

#include <string>

ModuleInput::ModuleInput(App* app) : Module(app), cursor(DEFAULT), mouse_x(0), mouse_y(0), mouse_z(0), mouse_x_motion(0), mouse_y_motion(0)
{
	for (int i = 0; i < MAX_KEYS; ++i) keyboard[i] = KEY_IDLE;

	memset(keyboard, KEY_IDLE, sizeof(KEY_STATE) * MAX_KEYS);
	memset(mouse_buttons, KEY_IDLE, sizeof(KEY_STATE) * MAX_MOUSE_BUTTONS);
}

ModuleInput::~ModuleInput()
{
}

bool ModuleInput::Awake()
{
	LOG(LogType::LOG_INFO, "Init SDL input event system");
	bool ret = true;
	SDL_Init(0);

	if (SDL_InitSubSystem(SDL_INIT_EVENTS) < 0)
	{
		LOG(LogType::LOG_ERROR, "SDL_EVENTS could not initialize! SDL_Error: %s\n", SDL_GetError());
		ret = false;
	}

	SDL_EventState(SDL_DROPFILE, SDL_ENABLE);

	CreateCursors();

	return ret;
}

bool ModuleInput::PreUpdate(float dt)
{
	SDL_PumpEvents();

	const Uint8* keys = SDL_GetKeyboardState(NULL);

	for (int i = 0; i < MAX_KEYS; ++i)
	{
		if (keys[i] == 1)
		{
			if (keyboard[i] == KEY_IDLE)
				keyboard[i] = KEY_DOWN;
			else
				keyboard[i] = KEY_REPEAT;
		}
		else
		{
			if (keyboard[i] == KEY_REPEAT || keyboard[i] == KEY_DOWN)
				keyboard[i] = KEY_UP;
			else
				keyboard[i] = KEY_IDLE;
		}
	}

	Uint32 buttons = SDL_GetMouseState(&mouse_x, &mouse_y);

	mouse_x /= 1;
	mouse_y /= 1;
	mouse_z = 0;

	for (int i = 0; i < 5; ++i)
	{
		if (buttons & SDL_BUTTON(i))
		{
			if (mouse_buttons[i] == KEY_IDLE)
				mouse_buttons[i] = KEY_DOWN;
			else
				mouse_buttons[i] = KEY_REPEAT;
		}
		else
		{
			if (mouse_buttons[i] == KEY_REPEAT || mouse_buttons[i] == KEY_DOWN)
				mouse_buttons[i] = KEY_UP;
			else
				mouse_buttons[i] = KEY_IDLE;
		}
	}

	mouse_x_motion = mouse_y_motion = 0;

	SDL_Event e;
	while (SDL_PollEvent(&e))
	{
		ImGui_ImplSDL2_ProcessEvent(&e);

		switch (e.type)
		{
		case SDL_MOUSEWHEEL:
			mouse_z = e.wheel.y;
			break;

		case SDL_MOUSEMOTION:
			mouse_x = e.motion.x;
			mouse_y = e.motion.y;

			mouse_x_motion = e.motion.xrel;
			mouse_y_motion = e.motion.yrel;
			break;

		case (SDL_DROPFILE):
		{
			std::string droppedFileDir(e.drop.file);

			app->importer->SetDraggedFile(droppedFileDir);

			SDL_free(e.drop.file);
			break;
		}
		case SDL_WINDOWEVENT:
			if (e.window.event == SDL_WINDOWEVENT_RESIZED)
				app->window->OnResize(e.window.data1, e.window.data2);
			break;

		case SDL_QUIT:
			app->exit = true;
			break;
		}
	}

	return true;
}

bool ModuleInput::CleanUp()
{
	LOG(LogType::LOG_INFO, "Quitting SDL input event subsystem");

	zoomCursor.reset();
	freeLookCursor.reset();
	dragCursor.reset();
	orbitCursor.reset();

	zoomSurface.reset();
	freeLookSurface.reset();
	dragSurface.reset();
	orbitSurface.reset();

	SDL_QuitSubSystem(SDL_INIT_EVENTS);

	return true;
}

void ModuleInput::ChangeCursor(CursorType newCursor)
{
	if (cursor != newCursor)
	{
		cursor = newCursor;
		SetCursor();
	}
}

void ModuleInput::SetCursor()
{
	SDL_Cursor* sdlCursor = nullptr;

	switch (cursor)
	{
	case CursorType::DEFAULT:
		sdlCursor = SDL_GetDefaultCursor();
		break;
	case CursorType::ZOOM:
		sdlCursor = zoomCursor.get();
		break;
	case CursorType::FREELOOK:
		sdlCursor = freeLookCursor.get();
		break;
	case CursorType::DRAG:
		sdlCursor = dragCursor.get();
		break;
	case CursorType::ORBIT:
		sdlCursor = orbitCursor.get();
		break;
	}

	if (sdlCursor != SDL_GetCursor())
		SDL_SetCursor(sdlCursor);
}

void ModuleInput::CreateCursors()
{
	zoomSurface.reset(SDL_LoadBMP(BMP_ZOOM));
	freeLookSurface.reset(SDL_LoadBMP(BMP_FREELOOK));
	dragSurface.reset(SDL_LoadBMP(BMP_DRAG));
	orbitSurface.reset(SDL_LoadBMP(BMP_ORBIT));

	MakeCursorTransparent(zoomSurface.get());
	MakeCursorTransparent(freeLookSurface.get());
	MakeCursorTransparent(dragSurface.get());
	MakeCursorTransparent(orbitSurface.get());

	zoomCursor.reset(SDL_CreateColorCursor(zoomSurface.get(), 5, 0));
	freeLookCursor.reset(SDL_CreateColorCursor(freeLookSurface.get(), 5, 0));
	dragCursor.reset(SDL_CreateColorCursor(dragSurface.get(), 5, 0));
	orbitCursor.reset(SDL_CreateColorCursor(orbitSurface.get(), 5, 0));
}

void ModuleInput::MakeCursorTransparent(SDL_Surface* surface)
{
	if (SDL_MUSTLOCK(surface)) SDL_LockSurface(surface);

	Uint32* pixels = (Uint32*)surface->pixels;
	int pixelCount = (surface->w) * (surface->h);

	for (int i = 0; i < pixelCount; i++)
	{
		Uint8 r, g, b, a;
		SDL_GetRGBA(pixels[i], surface->format, &r, &g, &b, &a);

		if (r == 255 && g == 0 && b == 0)
		{
			pixels[i] = SDL_MapRGBA(surface->format, r, g, b, 0);
		}
	}

	if (SDL_MUSTLOCK(surface)) SDL_UnlockSurface(surface);
}