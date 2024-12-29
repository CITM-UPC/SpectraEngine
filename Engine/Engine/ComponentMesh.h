#pragma once

#include "Component.h"
#include "ComponentCamera.h"
#include "Mesh.h"

class Mesh;

class ComponentMesh : public Component
{
public:
	ComponentMesh(GameObject* gameObject);
	virtual ~ComponentMesh();

	void Update() override;
	void OnEditor() override;
	void Draw(ComponentCamera* camera);

public:
	Mesh* mesh;
	bool drawOutline = false;

private:
	bool showVertexNormals = false;
	bool showFaceNormals = false;

	bool showAABB = false;
	bool showOBB = false;
};