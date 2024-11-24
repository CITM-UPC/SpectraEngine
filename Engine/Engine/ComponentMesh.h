#pragma once

#include "Component.h"
#include "Mesh.h"

class Mesh;

class ComponentMesh : public Component
{
public:
	ComponentMesh(GameObject* gameObject);
	virtual ~ComponentMesh();

	void Update() override;
	void OnEditor() override;

public:
	Mesh* mesh;

private:
	bool showVertexNormals = false;
	bool showFaceNormals = false;

	bool showAABB = false;
	bool showOBB = true;
};