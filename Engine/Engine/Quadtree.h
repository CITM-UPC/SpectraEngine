#pragma once

#include "Mesh.h"
#include <vector>
#include <glm/glm.hpp>

class Quadtree
{
public:
	Quadtree(const AABB& bounds);
	~Quadtree();

	void AddObject(Mesh* mesh, const glm::mat4 transform);
	void Split(const glm::mat4 transform);
	int GetQuadrant(const AABB& objectBounds);

	void Draw();

private:
	AABB bounds;
	Quadtree* children[4];
	std::vector<Mesh*> objects;
};