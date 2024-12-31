#include "ModuleRenderer3D.h"
#include "App.h"
#include "Texture.h"

#include <SDL2/SDL_opengl.h>
#include <gl/GL.h>
#include <gl/GLU.h>
#include <IL/il.h>
#include <IL/ilu.h>
#include <IL/ilut.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

ModuleRenderer3D::ModuleRenderer3D(App* app) : Module(app), rboScene(0), fboSceneTexture(0), fboScene(0), rboGame(0), fboGameTexture(0), fboGame(0), checkerTextureId(0), checkerImage{}
{
}

ModuleRenderer3D::~ModuleRenderer3D()
{
}

bool ModuleRenderer3D::Awake()
{
	bool ret = true;

	GLenum err = glewInit();
	if (err != GLEW_OK) {
		LOG(LogType::LOG_ERROR, "Error in loading Glew: %s\n", glewGetErrorString(err));
	}
	else {
		LOG(LogType::LOG_INFO, "Successfully using Glew %s", glewGetString(GLEW_VERSION));
	}

	if (ret == true)
	{
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
		{
			LOG(LogType::LOG_ERROR, "Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			LOG(LogType::LOG_ERROR, "Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
		glClearDepth(1.0f);

		glClearColor(0.1f, 0.1f, 0.1f, 1.f);

		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		error = glGetError();
		if (error != GL_NO_ERROR)
		{
			LOG(LogType::LOG_ERROR, "Error initializing OpenGL! %s\n", gluErrorString(error));
			ret = false;
		}

		GLfloat MaterialAmbient[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, MaterialAmbient);

		GLfloat MaterialDiffuse[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, MaterialDiffuse);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glEnable(GL_COLOR_MATERIAL);
		glEnable(GL_TEXTURE_2D);

		ilutRenderer(ILUT_OPENGL);
	}

	for (int i = 0; i < CHECKERS_HEIGHT; i++) {
		for (int j = 0; j < CHECKERS_WIDTH; j++) {
			int c = ((((i & 0x8) == 0) ^ (((j & 0x8)) == 0))) * 255;
			checkerImage[i][j][0] = (GLubyte)c;
			checkerImage[i][j][1] = (GLubyte)c;
			checkerImage[i][j][2] = (GLubyte)c;
			checkerImage[i][j][3] = (GLubyte)255;
		}
	}

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glGenTextures(1, &checkerTextureId);
	glBindTexture(GL_TEXTURE_2D, checkerTextureId);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, CHECKERS_WIDTH, CHECKERS_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, checkerImage);

	CreateFramebuffer();

	OnResize(SCREEN_WIDTH, SCREEN_HEIGHT);

	return ret;
}

bool ModuleRenderer3D::PreUpdate(float dt)
{
	if (updateFramebuffer)
	{
		OnResize(static_cast<int>(app->editor->sceneWindow->windowSize.x), static_cast<int>(app->editor->sceneWindow->windowSize.y));
		updateFramebuffer = false;
	}

	return true;
}

bool ModuleRenderer3D::PostUpdate(float dt)
{
	// Render first camera
	glBindFramebuffer(GL_FRAMEBUFFER, fboScene);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glm::mat4 projectionMatrix = app->scene->sceneCamera->GetProjectionMatrix();
	glLoadMatrixf(glm::value_ptr(projectionMatrix));

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	glm::mat4 viewMatrix = app->scene->sceneCamera->GetViewMatrix();
	glLoadMatrixf(glm::value_ptr(viewMatrix));

	grid.Render();

	DrawQueuedMeshes(app->scene->sceneCamera);

	if (app->scene->drawOctree)
		app->scene->sceneOctree->Draw(app->scene->octreeColor);

	if (app->scene->activeGameCamera)
	{
		// Render second camera
		glBindFramebuffer(GL_FRAMEBUFFER, fboGame);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();

		projectionMatrix = app->scene->activeGameCamera->GetProjectionMatrix();
		glLoadMatrixf(glm::value_ptr(projectionMatrix));

		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		viewMatrix = app->scene->activeGameCamera->GetViewMatrix();
		glLoadMatrixf(glm::value_ptr(viewMatrix));

		DrawQueuedMeshes(app->scene->activeGameCamera);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	app->editor->DrawEditor();

	SDL_GL_SwapWindow(app->window->window);

	meshQueue.clear();

	return true;
}

void ModuleRenderer3D::DrawQueuedMeshes(ComponentCamera* camera) const
{
	for (ComponentMesh* mesh : meshQueue)
	{
		if (camera == app->scene->sceneCamera ? mesh->gameObject->isOctreeInSceneFrustum : mesh->gameObject->isOctreeInGameFrustum)
			mesh->Draw(camera);
	}
}

bool ModuleRenderer3D::CleanUp()
{
	LOG(LogType::LOG_INFO, "Destroying 3D Renderer");

	glDeleteFramebuffers(1, &fboScene);
	glDeleteTextures(1, &fboSceneTexture);
	glDeleteRenderbuffers(1, &rboScene);

	glDeleteFramebuffers(1, &fboGame);
	glDeleteTextures(1, &fboGameTexture);
	glDeleteRenderbuffers(1, &rboGame);

	return true;
}

void ModuleRenderer3D::OnResize(int width, int height)
{
	glViewport(0, 0, width, height);

	app->scene->sceneCamera->screenWidth = width;
	app->scene->sceneCamera->screenHeight = height;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	glm::mat4 projectionMatrix = app->scene->sceneCamera->GetProjectionMatrix();
	glLoadMatrixf(glm::value_ptr(projectionMatrix));

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	if (fboScene > 0)
		glDeleteFramebuffers(1, &fboScene);

	if (fboSceneTexture > 0)
		glDeleteTextures(1, &fboSceneTexture);

	if (rboScene > 0)
		glDeleteRenderbuffers(1, &rboScene);

	if (fboGame > 0)
		glDeleteFramebuffers(1, &fboGame);

	if (fboGameTexture > 0)
		glDeleteTextures(1, &fboGameTexture);

	if (rboGame > 0)
		glDeleteRenderbuffers(1, &rboGame);

	CreateFramebuffer();
}

void ModuleRenderer3D::CreateFramebuffer()
{
	glGenFramebuffers(1, &fboScene);
	glBindFramebuffer(GL_FRAMEBUFFER, fboScene);

	glGenTextures(1, &fboSceneTexture);
	glBindTexture(GL_TEXTURE_2D, fboSceneTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(app->editor->sceneWindow->windowSize.x),
		static_cast<GLsizei>(app->editor->sceneWindow->windowSize.y), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboSceneTexture, 0);

	glGenRenderbuffers(1, &rboScene);
	glBindRenderbuffer(GL_RENDERBUFFER, rboScene);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8,
		static_cast<GLsizei>(app->editor->sceneWindow->windowSize.x), static_cast<GLsizei>(app->editor->sceneWindow->windowSize.y));
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboScene);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		LOG(LogType::LOG_ERROR, "Scene Framebuffer is not complete!");
	}

	glGenFramebuffers(1, &fboGame);
	glBindFramebuffer(GL_FRAMEBUFFER, fboGame);

	glGenTextures(1, &fboGameTexture);
	glBindTexture(GL_TEXTURE_2D, fboGameTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, static_cast<GLsizei>(app->editor->sceneWindow->windowSize.x), 
		static_cast<GLsizei>(app->editor->sceneWindow->windowSize.y), 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
	glBindTexture(GL_TEXTURE_2D, 0);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboGameTexture, 0);

	glGenRenderbuffers(1, &rboGame);
	glBindRenderbuffer(GL_RENDERBUFFER, rboGame);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 
		static_cast<GLsizei>(app->editor->sceneWindow->windowSize.x), static_cast<GLsizei>(app->editor->sceneWindow->windowSize.y));
	glBindRenderbuffer(GL_RENDERBUFFER, 0);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboGame);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		LOG(LogType::LOG_ERROR, "Game Framebuffer is not complete!");
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}