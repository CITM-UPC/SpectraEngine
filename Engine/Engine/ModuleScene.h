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
	void CollectObjects(const GameObject* gameObject, std::vector<GameObject*>& objects) const;

private:
	void UpdateOctree() const;
	void AddGameObjectToOctree(const GameObject* gameObject) const;

public:
	GameObject* root = nullptr;
	Octree* sceneOctree = nullptr;
	AABB sceneBounds;

	bool octreeNeedsUpdate = true;
};