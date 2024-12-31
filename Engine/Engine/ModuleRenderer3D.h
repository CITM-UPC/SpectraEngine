#pragma once

#include "Module.h"
#include "Grid.h"

#include <SDL2/SDL_video.h>
#include <GL/glew.h>
#include <vector>

#include "ComponentMesh.h"

#define CHECKERS_WIDTH 128*2
#define CHECKERS_HEIGHT 128*2

class ModuleRenderer3D : public Module
{
public:
	ModuleRenderer3D(App* app);
	~ModuleRenderer3D();

	bool Awake();
	bool PreUpdate(float dt);
	bool PostUpdate(float dt);
	bool CleanUp();

	void OnResize(int width, int height);
	void CreateFramebuffer();
	void DrawQueuedMeshes(ComponentCamera* camera) const;

	bool updateFramebuffer = false;

public:
	GLubyte checkerImage[CHECKERS_WIDTH][CHECKERS_HEIGHT][4];
	unsigned int checkerTextureId;

	Grid grid;

	GLuint fboScene;
	GLuint fboSceneTexture;
	GLuint rboScene;
	GLuint fboGame;
	GLuint fboGameTexture;
	GLuint rboGame;

	std::vector<ComponentMesh*> meshQueue;
};