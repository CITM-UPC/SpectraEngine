#pragma once

#include "Mesh.h"
#include "GameObject.h"
#include "Resource.h"

#include <assimp/scene.h>
#include <assimp/mesh.h>
#include <assimp/cimport.h>
#include <assimp/postprocess.h>

#include <vector>
#include <string>

class ModelImporter
{
public:
	ModelImporter();
	~ModelImporter();

	bool SaveModel(Resource* resource);
	bool LoadModel(Resource* resource, GameObject* root);

private:
	// Save functions
	void SaveMeshToCustomFile(aiMesh* mesh, const aiScene* scene, const std::string& filePath);
	void SaveModelToCustomFile(const aiScene* scene, const std::string& outputPath);
	void SaveNodeToBuffer(const aiNode* node, std::vector<char>& buffer, size_t& currentPos);
	size_t CalculateNodeSize(const aiNode* node);

	// Loading functions
	void LoadMeshFromCustomFile(const std::string& filePath, Mesh* mesh);
	void LoadModelFromCustomFile(const std::string& filePath, GameObject* root);
	void LoadNodeFromBuffer(const char* buffer, size_t& currentPos, std::vector<Mesh*>& meshes, GameObject* parent, const char* fileName);
};
