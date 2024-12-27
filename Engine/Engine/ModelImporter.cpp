#include "ModelImporter.h"
#include "App.h"
#include "ComponentMesh.h"

#include <iostream>
#include <fstream>

#include "Model.h"

ModelImporter::ModelImporter()
{
	struct aiLogStream stream;
	stream = aiGetPredefinedLogStream(aiDefaultLogStream_DEBUGGER, nullptr);
	aiAttachLogStream(&stream);
}

ModelImporter::~ModelImporter()
{
	aiDetachAllLogStreams();
}

bool ModelImporter::SaveModel(Resource* resource)
{
	const char* path = resource->GetAssetFileDir().c_str();

	const aiScene* scene = aiImportFile(path, aiProcessPreset_TargetRealtime_MaxQuality);
	if (scene == nullptr)
	{
		LOG(LogType::LOG_ERROR, "Error loading scene %s", path);
		return false;
	}

	std::string fileName = path;
	fileName = fileName.substr(fileName.find_last_of("/\\") + 1);
	fileName = fileName.substr(0, fileName.find_last_of("."));

	SaveModelToCustomFile(scene, fileName);
	aiReleaseImport(scene);

	LOG(LogType::LOG_INFO, "%s model Saved", fileName.c_str());
	return true;
}

bool ModelImporter::LoadModel(Resource* resource, GameObject* root)
{
	if (!root)
	{
		LOG(LogType::LOG_ERROR, "Invalid root GameObject provided");
		return false;
	}

	const char* path = resource->GetLibraryFileDir().c_str();

	std::string modelFilePath = path;
	std::ifstream modelFile(modelFilePath, std::ios::binary);

	if (!modelFile.good())
	{
		LOG(LogType::LOG_ERROR, "Failed to open model file: %s", path);
		return false;
	}

	modelFile.close();
	LoadModelFromCustomFile(modelFilePath, root);
	LOG(LogType::LOG_INFO, "Model loaded successfully from: %s", path);
	return true;
}

void ModelImporter::SaveMeshToCustomFile(aiMesh* newMesh, const aiScene* scene, const std::string& filePath)
{
	// Validate mesh data
	if (!newMesh || !newMesh->HasPositions() || !newMesh->HasNormals() || !newMesh->HasFaces())
	{
		LOG(LogType::LOG_ERROR, "Invalid aiMesh data");
		return;
	}

	bool hasTextureCoords = newMesh->HasTextureCoords(0);

	// Get counts
	const uint32_t ranges[4] =
	{
		newMesh->mNumFaces * 3,
		newMesh->mNumVertices,
		newMesh->mNumVertices,
		hasTextureCoords ? newMesh->mNumVertices : 0
	};

	// Process texture coordinates
	std::unique_ptr<float[]> texCoords;
	if (hasTextureCoords)
	{
		texCoords = std::make_unique<float[]>(static_cast<size_t>(newMesh->mNumVertices) * 2);
		for (size_t i = 0; i < newMesh->mNumVertices; i++)
		{
			texCoords[i * 2] = newMesh->mTextureCoords[0][i].x;
			texCoords[i * 2 + 1] = newMesh->mTextureCoords[0][i].y;
		}
	}

	// Process indices
	std::unique_ptr<uint32_t[]> indices(new uint32_t[static_cast<size_t>(newMesh->mNumFaces) * 3]);
	for (size_t i = 0; i < static_cast<size_t>(newMesh->mNumFaces); ++i)
	{
		if (newMesh->mFaces[i].mNumIndices != 3)
		{
			LOG(LogType::LOG_WARNING, "Face does not have 3 indices");
			continue;
		}
		memcpy(&indices[i * 3], newMesh->mFaces[i].mIndices, 3 * sizeof(uint32_t));
	}

	// Material properties
	glm::vec4 diffuseColor(1.0f);
	glm::vec4 specularColor(1.0f);
	glm::vec4 ambientColor(1.0f);
	std::string diffuseTexturePath;

	// Process material
	if (newMesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[newMesh->mMaterialIndex];
		aiColor4D color;

		if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &color))
		{
			diffuseColor = glm::vec4(color.r, color.g, color.b, color.a);
			LOG(LogType::LOG_INFO, "Loaded diffuse color");
		}

		if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_COLOR_SPECULAR, &color))
		{
			specularColor = glm::vec4(color.r, color.g, color.b, color.a);
			LOG(LogType::LOG_INFO, "Loaded specular color");
		}

		if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_COLOR_AMBIENT, &color))
		{
			ambientColor = glm::vec4(color.r, color.g, color.b, color.a);
			LOG(LogType::LOG_INFO, "Loaded ambient color");
		}

		aiString texturePath;
		if (material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath) == AI_SUCCESS)
		{
			std::string fullPath = texturePath.C_Str();

			size_t lastSlashPos = fullPath.find_last_of("\\/");

			std::string filename = (lastSlashPos != std::string::npos) ? fullPath.substr(lastSlashPos + 1) : fullPath;

			std::string basePath = "Assets/Textures/";
			if (app->fileSystem->FileExists(basePath + filename))
			{
				diffuseTexturePath = basePath + filename;
				//if (!app->resources->FindResourceInLibrary(diffuseTexturePath, ResourceType::TEXTURE))
				//	app->importer->ImportFile(diffuseTexturePath);
			}
		}
	}

	// Calculate total size needed
	size_t size = sizeof(ranges)
		+ (sizeof(uint32_t) * ranges[0])
		+ (sizeof(float) * ranges[1] * 3)
		+ (sizeof(float) * ranges[2] * 3)
		+ (hasTextureCoords ? (sizeof(float) * ranges[3] * 2) : 0)
		+ (sizeof(glm::vec4) * 3)
		+ sizeof(uint32_t)
		+ diffuseTexturePath.size() + 1;

	// Allocate buffer and write data
	std::unique_ptr<char[]> fileBuffer(new char[size]);
	char* cursor = fileBuffer.get();

	auto writeData = [&](const void* data, size_t bytes)
		{
			memcpy(cursor, data, bytes);
			cursor += bytes;
		};

	// Write all data
	writeData(ranges, sizeof(ranges));
	writeData(indices.get(), sizeof(uint32_t) * ranges[0]);
	writeData(newMesh->mVertices, sizeof(float) * ranges[1] * 3);
	writeData(newMesh->mNormals, sizeof(float) * ranges[2] * 3);

	if (newMesh->HasTextureCoords(0))
		writeData(texCoords.get(), sizeof(float) * ranges[3] * 2);

	writeData(&diffuseColor, sizeof(glm::vec4));
	writeData(&specularColor, sizeof(glm::vec4));
	writeData(&ambientColor, sizeof(glm::vec4));

	const uint32_t texturePathLength = static_cast<uint32_t>(diffuseTexturePath.size());
	writeData(&texturePathLength, sizeof(uint32_t));
	writeData(diffuseTexturePath.c_str(), static_cast<size_t>(texturePathLength) + 1);

	// Write to file
	std::ofstream file(filePath, std::ios::binary);
	if (file.is_open())
	{
		file.write(fileBuffer.get(), size);
		file.close();
		LOG(LogType::LOG_INFO, "Mesh saved directly to custom file: %s", filePath.c_str());
	}
	else
	{
		LOG(LogType::LOG_ERROR, "Failed to save mesh to custom file: %s", filePath.c_str());
	}
}

void ModelImporter::LoadMeshFromCustomFile(const std::string& filePath, Mesh* mesh)
{
	std::ifstream file(filePath, std::ios::binary);
	if (!file.is_open())
	{
		LOG(LogType::LOG_ERROR, "Failed to open file for loading data.");
	}

	uint32_t ranges[4] = { 0,0,0,0 };
	file.read(reinterpret_cast<char*>(ranges), sizeof(ranges));

	mesh->indicesCount = ranges[0];
	mesh->verticesCount = ranges[1];
	mesh->normalsCount = ranges[2];
	mesh->texCoordsCount = ranges[3];

	// Indices
	mesh->indices = new uint32_t[mesh->indicesCount];
	file.read(reinterpret_cast<char*>(mesh->indices), sizeof(uint32_t) * mesh->indicesCount);

	// Vertices
	mesh->vertices = new float[mesh->verticesCount * 3];
	file.read(reinterpret_cast<char*>(mesh->vertices), sizeof(float) * mesh->verticesCount * 3);

	// Normals
	mesh->normals = new float[mesh->normalsCount * 3];
	file.read(reinterpret_cast<char*>(mesh->normals), sizeof(float) * mesh->normalsCount * 3);

	// Texture coords
	mesh->texCoords = new float[mesh->texCoordsCount * 2];
	file.read(reinterpret_cast<char*>(mesh->texCoords), sizeof(float) * mesh->texCoordsCount * 2);

	// Materials
	file.read(reinterpret_cast<char*>(&mesh->diffuseColor), sizeof(glm::vec4));
	file.read(reinterpret_cast<char*>(&mesh->specularColor), sizeof(glm::vec4));
	file.read(reinterpret_cast<char*>(&mesh->ambientColor), sizeof(glm::vec4));

	// Texture
	uint32_t texturePathLength = 0;
	file.read(reinterpret_cast<char*>(&texturePathLength), sizeof(uint32_t));
	mesh->diffuseTexturePath.resize(texturePathLength);
	file.read(&mesh->diffuseTexturePath[0], texturePathLength);

	mesh->InitMesh();

	file.close();
}

void ModelImporter::SaveModelToCustomFile(const aiScene* scene, const std::string& fileName)
{
	// Save meshes to custom files
	std::vector<std::string> meshPaths;
	for (unsigned int i = 0; i < scene->mNumMeshes; i++)
	{
		std::string meshPath = "Library/Meshes/" + fileName + std::to_string(i) + ".mesh";
		SaveMeshToCustomFile(scene->mMeshes[i], scene, meshPath);
		meshPaths.push_back(meshPath);
	}

	std::vector<char> buffer;
	size_t currentPos = 0;

	// Save number of meshes
	uint32_t numMeshes = scene->mNumMeshes;
	buffer.resize(buffer.size() + sizeof(uint32_t));
	memcpy(buffer.data() + currentPos, &numMeshes, sizeof(uint32_t));
	currentPos += sizeof(uint32_t);

	// Save mesh paths
	for (const auto& path : meshPaths)
	{
		uint32_t pathLength = static_cast<uint32_t>(path.size());

		// Save path length
		buffer.resize(buffer.size() + sizeof(uint32_t));
		memcpy(buffer.data() + currentPos, &pathLength, sizeof(uint32_t));
		currentPos += sizeof(uint32_t);

		// Save path
		buffer.resize(buffer.size() + static_cast<size_t>(pathLength) + 1);
		memcpy(buffer.data() + currentPos, path.c_str(), static_cast<size_t>(pathLength) + 1);
		currentPos += static_cast<size_t>(pathLength) + 1;
	}

	// Calculate and reserve space for the node hierarchy
	size_t nodeSize = CalculateNodeSize(scene->mRootNode);
	buffer.resize(buffer.size() + nodeSize);

	// Save root node
	SaveNodeToBuffer(scene->mRootNode, buffer, currentPos);

	// Write to file
	std::string modelPath = "Library/Models/" + fileName + ".model";
	std::ofstream file(modelPath, std::ios::binary);
	if (file.is_open())
	{
		file.write(buffer.data(), buffer.size());
		file.close();
		LOG(LogType::LOG_INFO, "Model saved to custom file: %s", modelPath.c_str());
	}
	else
	{
		LOG(LogType::LOG_ERROR, "Failed to save model to custom file: %s", modelPath.c_str());
	}
}

void ModelImporter::SaveNodeToBuffer(const aiNode* node, std::vector<char>& buffer, size_t& currentPos)
{
	// Save node name
	uint32_t nameLength = static_cast<uint32_t>(strlen(node->mName.C_Str()));
	memcpy(buffer.data() + currentPos, &nameLength, sizeof(uint32_t));
	currentPos += sizeof(uint32_t);
	memcpy(buffer.data() + currentPos, node->mName.C_Str(), static_cast<size_t>(nameLength) + 1);
	currentPos += static_cast<size_t>(nameLength) + 1;

	memcpy(buffer.data() + currentPos, &node->mTransformation, sizeof(aiMatrix4x4));
	currentPos += sizeof(aiMatrix4x4);

	// Save number of meshes
	uint32_t numMeshes = node->mNumMeshes;
	memcpy(buffer.data() + currentPos, &numMeshes, sizeof(uint32_t));
	currentPos += sizeof(uint32_t);

	// Save meshes indices
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
	{
		uint32_t meshIndex = node->mMeshes[i];
		memcpy(buffer.data() + currentPos, &meshIndex, sizeof(uint32_t));
		currentPos += sizeof(uint32_t);
	}

	// Save number of children
	uint32_t numChildren = node->mNumChildren;
	memcpy(buffer.data() + currentPos, &numChildren, sizeof(uint32_t));
	currentPos += sizeof(uint32_t);

	// Save children nodes
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		SaveNodeToBuffer(node->mChildren[i], buffer, currentPos);
	}
}

void ModelImporter::LoadModelFromCustomFile(const std::string& filePath, GameObject* root)
{
	std::ifstream file(filePath, std::ios::binary);
	if (!file.is_open())
	{
		LOG(LogType::LOG_ERROR, "Failed to open model file: %s", filePath.c_str());
		return;
	}

	// Read file into buffer
	file.seekg(0, std::ios::end);
	size_t fileSize = file.tellg();
	file.seekg(0, std::ios::beg);
	std::vector<char> buffer(fileSize);
	file.read(buffer.data(), fileSize);
	file.close();

	size_t currentPos = 0;

	// Meshes number
	uint32_t numMeshes;
	memcpy(&numMeshes, buffer.data() + currentPos, sizeof(uint32_t));
	currentPos += sizeof(uint32_t);

	Resource* newModelResource = app->resources->FindResourceInLibrary(filePath, ResourceType::MODEL);
	if (!newModelResource)
		newModelResource = app->importer->ImportFileToLibrary(filePath, ResourceType::MODEL);

	Model* model = dynamic_cast<Model*>(newModelResource);

	if (app->resources->GetResourceUsageCount(model) == 0)
		app->resources->ModifyResourceUsageCount(model, 1);

	// Load meshes
	for (uint32_t i = 0; i < numMeshes; i++)
	{
		// Mesh path
		uint32_t pathLength;
		memcpy(&pathLength, buffer.data() + currentPos, sizeof(uint32_t));
		currentPos += sizeof(uint32_t);

		std::string meshPath(buffer.data() + currentPos, static_cast<size_t>(pathLength));
		currentPos += static_cast<size_t>(pathLength) + 1;

		if (!model->HasMeshAtIndex(i))
		{
			Resource* newMeshResource = app->resources->FindResourceInLibrary(meshPath, ResourceType::MESH);
			if (!newMeshResource)
				newMeshResource = app->importer->ImportFileToLibrary(meshPath, ResourceType::MESH);

			Mesh* mesh = dynamic_cast<Mesh*>(newMeshResource);

			if (mesh)
			{
				// Load mesh from file
				if (mesh->indicesCount == 0)
					LoadMeshFromCustomFile(meshPath, mesh);

				model->AddMesh(mesh, i);
				mesh->SetParentModel(model);
			}
		}
	}

	// Get file name
	std::string fileName = filePath;
	fileName = fileName.substr(fileName.find_last_of("/\\") + 1);
	fileName = fileName.substr(0, fileName.find_last_of("."));

	std::vector<Mesh*> orderedMeshes;
	for (uint32_t i = 0; i < numMeshes; i++)
	{
		Mesh* mesh = model->GetMeshByIndex(i);
		if (mesh)
			orderedMeshes.push_back(mesh);
	}

	// Load root node
	LoadNodeFromBuffer(buffer.data(), currentPos, orderedMeshes, root, fileName.c_str());
}

void ModelImporter::LoadNodeFromBuffer(const char* buffer, size_t& currentPos, std::vector<Mesh*>& meshes, GameObject* parent, const char* fileName)
{
	// Node name
	uint32_t nameLength;
	memcpy(&nameLength, buffer + currentPos, sizeof(uint32_t));
	currentPos += sizeof(uint32_t);

	std::string nodeName(buffer + currentPos, static_cast<size_t>(nameLength));
	currentPos += static_cast<size_t>(nameLength) + 1;

	aiMatrix4x4 transformation;
	memcpy(&transformation, buffer + currentPos, sizeof(aiMatrix4x4));
	currentPos += sizeof(aiMatrix4x4);

	glm::vec3 position(transformation.a4, transformation.b4, transformation.c4);

	glm::mat3 rotationMatrix(
		transformation.a1, transformation.b1, transformation.c1,
		transformation.a2, transformation.b2, transformation.c2,
		transformation.a3, transformation.b3, transformation.c3
	);

	glm::quat rotation = glm::quat_cast(rotationMatrix);
	glm::vec3 scale(
		glm::length(glm::vec3(transformation.a1, transformation.b1, transformation.c1)),
		glm::length(glm::vec3(transformation.a2, transformation.b2, transformation.c2)),
		glm::length(glm::vec3(transformation.a3, transformation.b3, transformation.c3))
	);

	// Node meshes number
	uint32_t numMeshes;
	memcpy(&numMeshes, buffer + currentPos, sizeof(uint32_t));
	currentPos += sizeof(uint32_t);

	// Create GameObject if there are meshes
	GameObject* gameObjectNode = nullptr;

	if (nodeName == "RootNode")
		nodeName = fileName;

	if (numMeshes > 0)
	{
		// Process meshes
		for (uint32_t i = 0; i < numMeshes; i++)
		{
			gameObjectNode = new GameObject(nodeName.c_str(), parent);

			gameObjectNode->transform->SetTransformMatrix(position, rotation, scale, gameObjectNode->parent->transform);
			gameObjectNode->transform->UpdateTransform();

			uint32_t meshIndex;
			memcpy(&meshIndex, buffer + currentPos, sizeof(uint32_t));
			currentPos += sizeof(uint32_t);

			if (meshIndex < meshes.size())
			{
				ComponentMesh* componentMesh = dynamic_cast<ComponentMesh*>(gameObjectNode->AddComponent(gameObjectNode->mesh));
				componentMesh->mesh = meshes[meshIndex];

				app->resources->ModifyResourceUsageCount(componentMesh->mesh, 1);

				if (!meshes[meshIndex]->diffuseTexturePath.empty())
				{
					std::string extension = app->fileSystem->GetExtension(meshes[meshIndex]->diffuseTexturePath);
					ResourceType resourceType = app->resources->GetResourceTypeFromExtension(extension);
					Resource* newResource = app->resources->FindResourceInLibrary(meshes[meshIndex]->diffuseTexturePath, resourceType);
					if (!newResource)
						newResource = app->importer->ImportFileToLibrary(meshes[meshIndex]->diffuseTexturePath, resourceType);

					Texture* newTexture = dynamic_cast<Texture*>(newResource);
					if (newTexture && newTexture->textureId == 0)
					{
						newTexture = app->importer->textureImporter->LoadTextureImage(newResource);
					}
					if (newTexture != nullptr)
						gameObjectNode->material->AddTexture(newTexture);
				}
			}

			parent->children.push_back(gameObjectNode);
		}

		app->scene->octreeNeedsUpdate = true;
	}

	uint32_t numChildren;
	memcpy(&numChildren, buffer + currentPos, sizeof(uint32_t));
	currentPos += sizeof(uint32_t);

	// Processs children nodes
	if (numChildren > 0)
	{
		GameObject* holder = gameObjectNode ? gameObjectNode : new GameObject(nodeName.c_str(), parent);
		if (!gameObjectNode)
		{
			holder->transform->SetTransformMatrix(position, rotation, scale, holder->parent->transform);
			holder->transform->UpdateTransform();
			parent->children.push_back(holder);
			app->scene->octreeNeedsUpdate = true;
		}

		for (uint32_t i = 0; i < numChildren; i++)
		{
			LoadNodeFromBuffer(buffer, currentPos, meshes, holder, nodeName.c_str());
		}
	}
}

size_t ModelImporter::CalculateNodeSize(const aiNode* node)
{
	size_t size = 0;

	// Node name size
	size += sizeof(uint32_t) + strlen(node->mName.C_Str()) + 1;

	size += sizeof(aiMatrix4x4);

	// Meshes size
	size += sizeof(uint32_t);
	size += sizeof(uint32_t) * node->mNumMeshes;

	// Children size
	size += sizeof(uint32_t);

	// Children nodes size
	for (unsigned int i = 0; i < node->mNumChildren; i++)
	{
		size += CalculateNodeSize(node->mChildren[i]);
	}

	return size;
}