#include "Octree.h"

#include <iostream>

Octree::Octree(const AABB& sceneBounds, uint maxDepth, uint maxObjects)
    : root(std::make_unique<OctreeNode>(sceneBounds)), maxDepth(maxDepth), maxObjects(maxObjects) {}

void Octree::Insert(Mesh* object, const AABB& objectBounds)
{
    Insert(root.get(), object, objectBounds, 0);
}

void Octree::Insert(OctreeNode* node, Mesh* object, const AABB& objectBounds, uint depth)
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
    glm::vec3 mid = node->bounds.min + size;

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

void Octree::Remove(Mesh* object) 
{
    Remove(root.get(), object);
}

void Octree::Remove(OctreeNode* node, Mesh* object) {
    if (!node) return;

    auto it = std::remove(node->objects.begin(), node->objects.end(), object);
    if (it != node->objects.end())
    {
        node->objects.erase(it, node->objects.end());
    }

    for (auto& child : node->children)
    {
        if (child)
        {
            Remove(child.get(), object);
        }
    }

    if (!node->IsLeaf())
    {
        int totalObjects = TotalObjects(node);

        if (totalObjects <= maxObjects)
        {
            for (auto& child : node->children)
            {
                if (child) 
                {
                    node->objects.insert(node->objects.end(), child->objects.begin(), child->objects.end());
                    child->objects.clear();
                }
            }

            for (auto& child : node->children)
            {
                child.reset();
            }
        }
    }
}


int Octree::TotalObjects(const OctreeNode* node) const
{
    if (!node) return 0;

    int count = node->objects.size();
    for (const auto& child : node->children)
    {
        if (child)
        {
            count += TotalObjects(child.get());
        }
    }

    return count;
}


std::vector<Mesh*> Octree::Query(const AABB& region, const glm::mat4& transform) const
{
    std::vector<Mesh*> results;

    Query(root.get(), region, results, transform);

    return results;
}

void Octree::Query(const OctreeNode* node, const AABB& region, std::vector<Mesh*>& results, const glm::mat4& transform) const
{
    if (!node || !Intersect(node->bounds, region))
    {
        return;
    }

    for (Mesh* object : node->objects)
    {
        AABB objectAABB = object->GetAABB(transform);
        if (Intersect(objectAABB, region))
        {
            results.push_back(object);
        }
    }

    for (const auto& child : node->children)
    {
        if (child)
        {
            Query(child.get(), region, results, transform);
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

bool Octree::Intersect(const AABB& a, const AABB& b) const
{
    return (a.min.x <= b.max.x && a.max.x >= b.min.x) && (a.min.y <= b.max.y && a.max.y >= b.min.y) && (a.min.z <= b.max.z && a.max.z >= b.min.z);
}

void Octree::Update(Mesh* object, const glm::mat4& transform) 
{
    AABB transformedAABB = object->GetAABB(transform);

    Remove(object);
    Insert(object, transformedAABB);
}

void Octree::DebugPrintObjects() const 
{
    //DebugPrintNodeObjects(root.get(), 0);
}

void Octree::DebugPrintNodeObjects(const OctreeNode* node, uint depth) const 
{
    if (!node) return;

    if (node->objects.size() > 0)
    {
        std::cout << "Depth: " << depth << " | AABB: ("
        << node->bounds.min.x << ", " << node->bounds.min.y << ", " << node->bounds.min.z
        << ") to ("
        << node->bounds.max.x << ", " << node->bounds.max.y << ", " << node->bounds.max.z
        << ") | Objects: " << node->objects.size() << std::endl;
    }

    for (auto* object : node->objects) 
    {
        std::cout << "  Object: " << object << std::endl;
    }

    for (const auto& child : node->children)
    {
        if (child)
        {
            DebugPrintNodeObjects(child.get(), depth + 1);
        }
    }
}

void Octree::DrawView(ImDrawList* drawList, float scale, const ImVec2& windowSize, const ImVec2& windowPos, int type) const
{
    glm::vec3 origin = root->bounds.min;
    glm::vec3 size = root->bounds.max - root->bounds.min;

    glm::vec3 octreeCenter = origin + size * 0.5f;
    glm::vec2 centerOffset = glm::vec2(windowSize.x, windowSize.y) * 0.5f;
    glm::vec2 centerPos;

    switch (type)
    {
    case 0:
	case 1:
		centerPos = glm::vec2(octreeCenter.x, octreeCenter.z);
		break;
	case 2:
	case 3:
		centerPos = glm::vec2(octreeCenter.x, octreeCenter.y);
		break;
	case 4:
	case 5:
		centerPos = glm::vec2(octreeCenter.z, octreeCenter.y);
		break;
    }

    DrawNodeView(root.get(), drawList, scale, windowSize, origin, windowPos, centerOffset - centerPos, type, 0);
}

void Octree::DrawNodeView(const OctreeNode* node, ImDrawList* drawList, float scale, const ImVec2& windowSize, const glm::vec3& origin, const ImVec2& windowPos, const glm::vec2& translation, int type, uint depth) const
{
    if (!node) return;

    glm::vec2 min2D, max2D;

    switch (type)
    {
    case 0: // XZ
        min2D = glm::vec2(node->bounds.min.x, node->bounds.min.z);
        max2D = glm::vec2(node->bounds.max.x, node->bounds.max.z);
        break;
    case 1: // XZ (inv)
        min2D = glm::vec2(node->bounds.min.x, -node->bounds.min.z);
        max2D = glm::vec2(node->bounds.max.x, -node->bounds.max.z);
        break;
    case 2: // XY
        min2D = glm::vec2(node->bounds.min.x, -node->bounds.min.y);
        max2D = glm::vec2(node->bounds.max.x, -node->bounds.max.y);
        break;
    case 3: // XY (inv)
        min2D = -glm::vec2(node->bounds.max.x, node->bounds.max.y);
        max2D = -glm::vec2(node->bounds.min.x, node->bounds.min.y);
        break;
    case 4: // YZ
        min2D = glm::vec2(node->bounds.min.z, -node->bounds.min.y);
        max2D = glm::vec2(node->bounds.max.z, -node->bounds.max.y);
        break;
    case 5: // YZ (inv)
        min2D = glm::vec2(-node->bounds.max.z, -node->bounds.min.y);
        max2D = glm::vec2(-node->bounds.min.z, -node->bounds.max.y);
        break;
    default:
        return;
    }

    glm::vec2 originOffset = (type % 2 == 0) ? glm::vec2(origin.x, origin[type / 2]) : glm::vec2(origin[type / 2], origin.y);

    min2D = (min2D - originOffset) * scale + translation;
    max2D = (max2D - originOffset) * scale + translation;

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
