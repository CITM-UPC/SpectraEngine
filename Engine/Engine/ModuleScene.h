#pragma once

#include "Module.h"
#include "GameObject.h"
#include "Octree.h"
#include "Mesh.h"

class GameObject;

class ModuleScene : public Module
{
public:
	ModuleScene(App* app);
	virtual ~ModuleScene();

	bool Awake();
	bool Update(float dt);
	bool CleanUp();

	GameObject* CreateGameObject(const char* name, GameObject* parent);

public:
	GameObject* root = nullptr;
	Octree* sceneOctree = nullptr;
	AABB sceneBounds;
};