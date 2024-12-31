#pragma once

#include "Module.h"
#include "SDL2/SDL.h"
#include <memory>

#define MAX_KEYS 256
#define MAX_MOUSE_BUTTONS 5

#define BMP_ZOOM "Engine/Cursors/zoom.bmp"
#define BMP_FREELOOK "Engine/Cursors/freeLook.bmp"
#define BMP_DRAG "Engine/Cursors/drag.bmp"
#define BMP_ORBIT "Engine/Cursors/orbit.bmp"

enum KEY_STATE
{
	KEY_IDLE = 0,
	KEY_DOWN,
	KEY_REPEAT,
	KEY_UP
};

enum CursorType
{
	DEFAULT = 0,
	ZOOM,
	FREELOOK,
	DRAG,
	ORBIT
};

class ModuleInput : public Module
{
public:

	ModuleInput(App* app);
	~ModuleInput();

	bool Awake();
	bool PreUpdate(float dt);
	bool CleanUp();

	void ChangeCursor(CursorType newCursor);
	CursorType GetCursor() { return cursor; }

	KEY_STATE GetKey(int id) const
	{
		return keyboard[id];
	}

	KEY_STATE GetMouseButton(int id) const
	{
		return mouse_buttons[id];
	}

	int GetMouseX() const
	{
		return mouse_x;
	}

	int GetMouseY() const
	{
		return mouse_y;
	}

	int GetMouseZ() const
	{
		return mouse_z;
	}

	int GetMouseXMotion() const
	{
		return mouse_x_motion;
	}

	int GetMouseYMotion() const
	{
		return mouse_y_motion;
	}

private:
	void SetCursor() const;
	void CreateCursors();
	void MakeCursorTransparent(SDL_Surface* surface);

public:
	std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> zoomSurface{ nullptr, SDL_FreeSurface };
	std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> freeLookSurface{ nullptr, SDL_FreeSurface };
	std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> dragSurface{ nullptr, SDL_FreeSurface };
	std::unique_ptr<SDL_Surface, decltype(&SDL_FreeSurface)> orbitSurface{ nullptr, SDL_FreeSurface };

	std::unique_ptr<SDL_Cursor, decltype(&SDL_FreeCursor)> zoomCursor{ nullptr, SDL_FreeCursor };
	std::unique_ptr<SDL_Cursor, decltype(&SDL_FreeCursor)> freeLookCursor{ nullptr, SDL_FreeCursor };
	std::unique_ptr<SDL_Cursor, decltype(&SDL_FreeCursor)> dragCursor{ nullptr, SDL_FreeCursor };
	std::unique_ptr<SDL_Cursor, decltype(&SDL_FreeCursor)> orbitCursor{ nullptr, SDL_FreeCursor };

private:
	KEY_STATE keyboard[MAX_KEYS];
	KEY_STATE mouse_buttons[MAX_MOUSE_BUTTONS];
	int mouse_x;
	int mouse_y;
	int mouse_z;
	int mouse_x_motion;
	int mouse_y_motion;

	CursorType cursor;
};