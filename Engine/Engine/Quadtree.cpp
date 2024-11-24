
#include "Quadtree.h"
#include <GL/glew.h>

const int MAX_OBJECTS = 4;

Quadtree::Quadtree(const AABB& bounds) : bounds(bounds)
{
    for (int i = 0; i < 4; i++)
        children[i] = nullptr;
}

Quadtree::~Quadtree()
{
}

void Quadtree::AddObject(Mesh* mesh, const glm::mat4 transform)
{
    if (children[0] != nullptr) 
    {
        int index = GetQuadrant(mesh->GetAABB(transform));
        if (index != -1) 
        {
            children[index]->AddObject(mesh, transform);
            return;
        }
    }

    objects.push_back(mesh);

    if (objects.size() > MAX_OBJECTS && children[0] == nullptr) 
    {
        Split(transform);
    }
}

void Quadtree::Split(const glm::mat4 transform)
{
    float subWidth = (bounds.max.x - bounds.min.x) / 2.0f;
    float subDepth = (bounds.max.z - bounds.min.z) / 2.0f;
    glm::vec3 center(bounds.min.x + subWidth, bounds.min.y, bounds.min.z + subDepth);

    children[0] = new Quadtree(AABB(bounds.min, center));                                                                                   // NW
    children[1] = new Quadtree(AABB(glm::vec3(center.x, bounds.min.y, bounds.min.z), glm::vec3(bounds.max.x, bounds.max.y, center.z)));     // NE
    children[2] = new Quadtree(AABB(glm::vec3(bounds.min.x, bounds.min.y, center.z), glm::vec3(center.x, bounds.max.y, bounds.max.z)));     // SW
    children[3] = new Quadtree(AABB(glm::vec3(center.x, bounds.min.y, center.z), bounds.max));                                              // SE

    for (auto* obj : objects) 
    {
        int index = GetQuadrant(obj->GetAABB(transform));
        if (index != -1) 
        {
            children[index]->AddObject(obj, transform);
        }
    }

    objects.clear();
}

int Quadtree::GetQuadrant(const AABB& objectBounds)
{
    float verticalMidpoint = bounds.min.x + (bounds.max.x - bounds.min.x) / 2.0f;
    float horizontalMidpoint = bounds.min.z + (bounds.max.z - bounds.min.z) / 2.0f;

    bool topQuadrant = (objectBounds.min.z < horizontalMidpoint && objectBounds.max.z < horizontalMidpoint);
    bool bottomQuadrant = (objectBounds.min.z > horizontalMidpoint);
    bool leftQuadrant = (objectBounds.min.x < verticalMidpoint && objectBounds.max.x < verticalMidpoint);
    bool rightQuadrant = (objectBounds.min.x > verticalMidpoint);

    if (leftQuadrant && topQuadrant) 
        return 0;                           // NW
    if (rightQuadrant && topQuadrant) 
        return 1;                           // NE
    if (leftQuadrant && bottomQuadrant) 
        return 2;                           // SW
    if (rightQuadrant && bottomQuadrant) 
        return 3;                           // SE

    return -1;
}

void Quadtree::Draw()
{
    glBegin(GL_LINE_LOOP);
    glColor3f(1.0f, 0.0f, 1.0f);

    glVertex3f(bounds.min.x, 0, bounds.min.z);
    glVertex3f(bounds.max.x, 0, bounds.min.z);
    glVertex3f(bounds.max.x, 0, bounds.max.z);
    glVertex3f(bounds.min.x, 0, bounds.max.z);

    glEnd();

    if (children[0] != nullptr)
    {
        for (int i = 0; i < 4; i++)
        {
            children[i]->Draw();
        }
    }
}