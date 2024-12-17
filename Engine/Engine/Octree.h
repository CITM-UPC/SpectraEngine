#pragma once

#include "Mesh.h"
#include "GameObject.h"

#include <vector>
#include <memory>

#include "imgui.h"

struct OctreeNode 
{
    AABB bounds;
    std::vector<GameObject*> objects;
    std::array<std::unique_ptr<OctreeNode>, 8> children;

    bool isOnFrustum = false;

    OctreeNode(const AABB& bounds) : bounds(bounds), children({ nullptr }) {}

    bool IsLeaf() const
    {
        for (const auto& child : children)
        {
            if (child) return false;
        }
        return true;
    }

    void UpdateIsOnFrustum();
};

class Octree
{
public:
    Octree(const AABB& sceneBounds, uint maxDepth = 5, uint maxObjects = 5);

    void Insert(GameObject* object, const AABB& objectBounds);
    void Draw(const glm::vec3& color = glm::vec3(1.0f, 1.0f , 0.0f)) const;
    void DrawView(ImDrawList* drawList, const ImVec2& windowSize, const ImVec2& windowPos, int type) const;
    void UpdateAllNodesVisibility() const;
    void Clear();
	void CollectIntersectingObjects(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, std::vector<GameObject*>& objects) const;
	void SetMaxDepth(const int newDepth) { maxDepth = newDepth; }
	void SetMaxObjects(const int newObjects) { maxObjects = newObjects; }
	AABB GetBounds() const { return root->bounds; }
	void SetBounds(const AABB& newBounds) const { root->bounds = newBounds; }

private:
    void Insert(OctreeNode* node, GameObject* object, const AABB& objectBounds, uint depth);
    void Subdivide(OctreeNode* node);
	bool Intersect(const AABB& a, const AABB& b) const;
	void ClearNode(OctreeNode* node);
	void CollectIntersectingObjects(const OctreeNode* node, const glm::vec3& rayOrigin, const glm::vec3& rayDirection, std::vector<GameObject*>& objects) const;

    void DrawNode(const OctreeNode* node, const glm::vec3& color) const;
    void DrawAABB(const AABB& aabb, const glm::vec3& color) const;
    void DrawNodeView(const OctreeNode* node, ImDrawList* drawList, float scale, const ImVec2& windowSize, const glm::vec3& origin, const ImVec2& windowPos, const glm::vec2& translation, int type, uint depth) const;

private:
    std::unique_ptr<OctreeNode> root;
    uint maxDepth;
    uint maxObjects;
};
