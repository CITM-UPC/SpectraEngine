#pragma once

#include <vector>

#include "Resource.h"

class Mesh;

struct MeshInfo
{
	Mesh* mesh;
	uint32_t originalIndex;
};

class Model : public Resource
{
public:
	Model();
	~Model();

	void DeleteMesh(const Mesh* mesh);
	Mesh* GetMeshByIndex(uint32_t index) const;
	void AddMesh(Mesh* mesh, uint32_t index);
	bool HasMeshAtIndex(uint32_t index) const;
	size_t GetMeshCount() const { return meshInfos.size(); }

private:
	std::vector<MeshInfo> meshInfos;
};
