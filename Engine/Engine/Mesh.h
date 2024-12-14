#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <string>
#include <array>

typedef unsigned int uint;

struct AABB
{
	glm::vec3 min;
	glm::vec3 max;

	AABB() = default;
	AABB(const glm::vec3& min, const glm::vec3& max) : min(min), max(max) {}

	AABB Transformed(const glm::mat4& transform) const {
		glm::vec3 vertices[8] = {
			min,
			glm::vec3(min.x, min.y, max.z),
			glm::vec3(min.x, max.y, min.z),
			glm::vec3(min.x, max.y, max.z),
			glm::vec3(max.x, min.y, min.z),
			glm::vec3(max.x, min.y, max.z),
			glm::vec3(max.x, max.y, min.z),
			max
		};

		glm::vec3 newMin(FLT_MAX), newMax(-FLT_MAX);
		for (const auto& vertex : vertices) {
			glm::vec3 transformedVertex = glm::vec3(transform * glm::vec4(vertex, 1.0f));
			newMin = glm::min(newMin, transformedVertex);
			newMax = glm::max(newMax, transformedVertex);
		}
		return AABB(newMin, newMax);
	}

	bool IntersectsRay(const glm::vec3& rayOrigin, const glm::vec3& rayDirection) const
	{
		glm::vec3 invDir = 1.0f / rayDirection;
		glm::vec3 t0s = (min - rayOrigin) * invDir;
		glm::vec3 t1s = (max - rayOrigin) * invDir;

		float tmin = glm::max(glm::max(glm::min(t0s.x, t1s.x), glm::min(t0s.y, t1s.y)), glm::min(t0s.z, t1s.z));
		float tmax = glm::min(glm::min(glm::max(t0s.x, t1s.x), glm::max(t0s.y, t1s.y)), glm::max(t0s.z, t1s.z));

		return tmax >= tmin && tmax >= 0.0f;
	}
};

struct OBB
{
	std::array<glm::vec3, 8> vertices;

	OBB() = default;
	OBB(const glm::mat4& transform, const AABB& aabb) {
		vertices = {
			glm::vec3(transform * glm::vec4(aabb.min.x, aabb.min.y, aabb.min.z, 1.0f)),
			glm::vec3(transform * glm::vec4(aabb.min.x, aabb.min.y, aabb.max.z, 1.0f)),
			glm::vec3(transform * glm::vec4(aabb.min.x, aabb.max.y, aabb.min.z, 1.0f)),
			glm::vec3(transform * glm::vec4(aabb.min.x, aabb.max.y, aabb.max.z, 1.0f)),
			glm::vec3(transform * glm::vec4(aabb.max.x, aabb.min.y, aabb.min.z, 1.0f)),
			glm::vec3(transform * glm::vec4(aabb.max.x, aabb.min.y, aabb.max.z, 1.0f)),
			glm::vec3(transform * glm::vec4(aabb.max.x, aabb.max.y, aabb.min.z, 1.0f)),
			glm::vec3(transform * glm::vec4(aabb.max.x, aabb.max.y, aabb.max.z, 1.0f))
		};
	}
};

class Mesh
{
public:
	void InitMesh();
	void DrawMesh(GLuint textureID, bool drawTextures, bool wireframe, bool shadedWireframe);
	void DrawNormals(bool vertexNormals, bool faceNormals, float vertexNormalLength, float faceNormalLength, glm::vec3 vertexNormalColor, glm::vec3 faceNormalColor);
	void CleanUpMesh();

	const AABB& GetAABB() const { return aabb; }
	AABB GetAABB(const glm::mat4& transform) { return aabb.Transformed(transform); }
	OBB GetOBB(const glm::mat4& transform) { return { transform, GetAABB() }; }

	void DrawAABB(const glm::mat4& transform);
	void DrawOBB(const glm::mat4& transform);

public:
	uint indicesId = 0;
	uint indicesCount = 0;
	uint* indices = nullptr;
	uint verticesId = 0;
	uint verticesCount = 0;
	float* vertices = nullptr;
	uint normalsId = 0;
	uint normalsCount = 0;
	float* normals = nullptr;
	uint texCoordsId = 0;
	uint texCoordsCount = 0;
	float* texCoords = nullptr;

	// Material properties
	glm::vec4 diffuseColor = glm::vec4(1.0f);
	glm::vec4 specularColor = glm::vec4(1.0f);
	glm::vec4 ambientColor = glm::vec4(1.0f);
	std::string diffuseTexturePath;

	AABB aabb;
	OBB obb;
};