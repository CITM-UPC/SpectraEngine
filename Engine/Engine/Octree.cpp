#include "Octree.h"

#include <iostream>

#include "App.h"

Octree::Octree(const AABB& sceneBounds, uint maxDepth, uint maxObjects)
    : root(std::make_unique<OctreeNode>(sceneBounds)), maxDepth(maxDepth), maxObjects(maxObjects) {}

void Octree::Insert(GameObject* object, const AABB& objectBounds)
{
    Insert(root.get(), object, objectBounds, 0);
}

void OctreeNode::UpdateIsOnFrustum()
{
    isOnFrustum = app->camera->IsAABBInFrustum(bounds);

	for (const auto& object : objects)
	{
        if (isOnFrustum)
            object->isOctreeInFrustum = isOnFrustum;
	}

    if (IsLeaf()) return;

    for (auto& child : children)
    {
        if (child)
        {
            child->UpdateIsOnFrustum();
        }
    }
}

void Octree::UpdateAllNodesVisibility() const
{
    std::vector<GameObject*> objects;
    app->scene->CollectObjects(app->scene->root, objects);
    for (const auto& object : objects)
    {
        if (object != nullptr)
        {
            object->isOctreeInFrustum = false;
        }
    }

    if (root)
    {
        root->UpdateIsOnFrustum();
    }
}

void Octree::Insert(OctreeNode* node, GameObject* object, const AABB& objectBounds, uint depth)
{
    if (!Intersect(node->bounds, objectBounds))
    {
        return;
    }

    if (node->IsLeaf() && (node->objects.size() < maxObjects || depth >= maxDepth))
    {
        node->objects.push_back(object);
        return;
    }

    if (node->IsLeaf())
    {
        Subdivide(node);

        std::vector<GameObject*> objectsToRedistribute = std::move(node->objects);
        node->objects.clear();

        for (auto* existingObject : objectsToRedistribute)
        {
            AABB existingBounds = existingObject->GetAABB();
            for (auto& child : node->children)
            {
                if (child && Intersect(child->bounds, existingBounds))
                {
                    Insert(child.get(), existingObject, existingBounds, depth + 1);
                }
            }
        }
    }

    for (auto& child : node->children)
    {
        if (child)
        {
            Insert(child.get(), object, objectBounds, depth + 1);
        }
    }
}

void Octree::Subdivide(OctreeNode* node)
{
    glm::vec3 size = (node->bounds.max - node->bounds.min) * 0.5f;

    for (int i = 0; i < 8; ++i) 
    {
        glm::vec3 offset(
            (i & 1) ? size.x : 0.0f,
            (i & 2) ? size.y : 0.0f,
            (i & 4) ? size.z : 0.0f
        );

        glm::vec3 childMin = node->bounds.min + offset;
        glm::vec3 childMax = childMin + size;

        node->children[i] = std::make_unique<OctreeNode>(AABB(childMin, childMax));
    }
}

bool Octree::Intersect(const AABB& a, const AABB& b) const
{
    return (a.min.x <= b.max.x && a.max.x >= b.min.x) && (a.min.y <= b.max.y && a.max.y >= b.min.y) && (a.min.z <= b.max.z && a.max.z >= b.min.z);
}

void Octree::Clear()
{
    ClearNode(root.get());
}

void Octree::ClearNode(OctreeNode* node)
{
    if (!node)
        return;

    node->objects.clear();

    for (auto& child : node->children)
    {
        if (child)
        {
            ClearNode(child.get());
            child.reset();
        }
    }
}

void Octree::DrawAABB(const AABB& aabb, const glm::vec3& color) const
{
    glm::vec3 vertices[8] = {
        aabb.min,
        glm::vec3(aabb.min.x, aabb.min.y, aabb.max.z),
        glm::vec3(aabb.min.x, aabb.max.y, aabb.min.z),
        glm::vec3(aabb.min.x, aabb.max.y, aabb.max.z),
        glm::vec3(aabb.max.x, aabb.min.y, aabb.min.z),
        glm::vec3(aabb.max.x, aabb.min.y, aabb.max.z),
        glm::vec3(aabb.max.x, aabb.max.y, aabb.min.z),
        aabb.max
    };

    unsigned int indices[24] = {
        0, 1, 0, 2, 0, 4, // Min corner
        1, 3, 1, 5,       // Adjacent min corner
        2, 3, 2, 6,       // Top of the min face
        3, 7,             // Remaining edge on the top face
        4, 5, 4, 6,       // Max corner
        5, 7, 6, 7        // Edges completing the cube
    };

    glColor3f(color.r, color.g, color.b);

    glBegin(GL_LINES);
    for (int i = 0; i < 24; ++i) 
    {
        const glm::vec3& vertex = vertices[indices[i]];
        glVertex3f(vertex.x, vertex.y, vertex.z);
    }
    glEnd();
}

void Octree::Draw(const glm::vec3& color) const
{
    DrawNode(root.get(), color);
}

void Octree::DrawNode(const OctreeNode* node, const glm::vec3& color) const
{
    if (!node) return;

    DrawAABB(node->bounds, color);

    for (const auto& child : node->children)
    {
        if (child)
        {
            DrawNode(child.get(), color);
        }
    }
}

void Octree::DrawView(ImDrawList* drawList, const ImVec2& windowSize, const ImVec2& windowPos, int type) const
{
    if (!root) return;

    glm::vec3 min = root->bounds.min;
    glm::vec3 max = root->bounds.max;
    glm::vec3 size = max - min;
    glm::vec3 center = (min + max) * 0.5f;

    float scaleX, scaleY;
    switch (type)
    {
    case 0: // Top (XZ)
    case 1: // Bottom (XZ)
        scaleX = windowSize.x / size.x;
        scaleY = windowSize.y / size.z;
        break;
    case 2: // Front (XY)
    case 3: // Back (XY)
        scaleX = windowSize.x / size.x;
        scaleY = windowSize.y / size.y;
        break;
    case 4: // Left (YZ)
    case 5: // Right (YZ)
        scaleX = windowSize.x / size.z;
        scaleY = windowSize.y / size.y;
        break;
    default:
        return;
    }

    float viewScale = std::min(scaleX, scaleY);

    glm::vec2 centerOffset = glm::vec2(windowSize.x, windowSize.y) * 0.5f;
    glm::vec2 centerPos;

    switch (type)
    {
    case 0: // Top (XZ)
        centerPos = glm::vec2(center.x, center.z);
        break;
    case 1: // Bottom (XZ inverted)
        centerPos = glm::vec2(center.x, -center.z);
        break;
    case 2: // Front (XY)
        centerPos = glm::vec2(center.x, -center.y);
        break;
    case 3: // Back (XY inverted)
        centerPos = glm::vec2(-center.x, -center.y);
        break;
    case 4: // Left (YZ)
        centerPos = glm::vec2(center.z, -center.y);
        break;
    case 5: // Right (YZ inverted)
        centerPos = glm::vec2(-center.z, -center.y);
        break;
    }

    glm::vec2 translation = centerOffset - centerPos * viewScale;

    DrawNodeView(root.get(), drawList, viewScale, windowSize, min, windowPos, translation, type, 0);
}

void Octree::DrawNodeView(const OctreeNode* node, ImDrawList* drawList, float scale, const ImVec2& windowSize, const glm::vec3& origin, const ImVec2& windowPos, const glm::vec2& translation, int type, uint depth) const
{
    if (!node) return;

    glm::vec2 min2D, max2D;
    switch (type)
    {
    case 0: // Top (XZ)
        min2D = glm::vec2(node->bounds.min.x, node->bounds.min.z);
        max2D = glm::vec2(node->bounds.max.x, node->bounds.max.z);
        break;
    case 1: // Bottom (XZ inverted)
        min2D = glm::vec2(node->bounds.min.x, -node->bounds.max.z);
        max2D = glm::vec2(node->bounds.max.x, -node->bounds.min.z);
        break;
    case 2: // Front (XY)
        min2D = glm::vec2(node->bounds.min.x, -node->bounds.max.y);
        max2D = glm::vec2(node->bounds.max.x, -node->bounds.min.y);
        break;
    case 3: // Back (XY inverted)
        min2D = glm::vec2(-node->bounds.max.x, -node->bounds.max.y);
        max2D = glm::vec2(-node->bounds.min.x, -node->bounds.min.y);
        break;
    case 4: // Left (YZ)
        min2D = glm::vec2(node->bounds.min.z, -node->bounds.max.y);
        max2D = glm::vec2(node->bounds.max.z, -node->bounds.min.y);
        break;
    case 5: // Right (YZ inverted)
        min2D = glm::vec2(-node->bounds.max.z, -node->bounds.max.y);
        max2D = glm::vec2(-node->bounds.min.z, -node->bounds.min.y);
        break;
    default:
        return;
    }

    min2D = min2D * scale + translation;
    max2D = max2D * scale + translation;

    uint r = 255;
    uint g = glm::clamp(255 - (depth * (255 / maxDepth)), 0u, 255u);
    uint b = 0;
    ImU32 color = IM_COL32(r, g, b, 255);

    ImVec2 imMin = ImVec2(min2D.x + windowPos.x, windowPos.y + min2D.y);
    ImVec2 imMax = ImVec2(max2D.x + windowPos.x, windowPos.y + max2D.y);
    drawList->AddRect(imMin, imMax, color);

    for (const auto& child : node->children)
    {
        if (child)
        {
            DrawNodeView(child.get(), drawList, scale, windowSize, origin, windowPos, translation, type, depth + 1);
        }
    }
}

void Octree::CollectIntersectingObjects(const glm::vec3& rayOrigin, const glm::vec3& rayDirection, std::vector<GameObject*>& objects) const
{
	CollectIntersectingObjects(root.get(), rayOrigin, rayDirection, objects);
}

void Octree::CollectIntersectingObjects(const OctreeNode* node, const glm::vec3& rayOrigin, const glm::vec3& rayDirection, std::vector<GameObject*>& objects) const
{
	if (!node) return;

	if (node->bounds.IntersectsRay(rayOrigin, rayDirection))
	{
		for (const auto& object : node->objects)
		{
			if (object->isOctreeInFrustum)
                objects.push_back(object);
		}

		for (const auto& child : node->children)
		{
			if (child)
			{
				CollectIntersectingObjects(child.get(), rayOrigin, rayDirection, objects);
			}
		}
	}
}
