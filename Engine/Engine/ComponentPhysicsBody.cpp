#include "ComponentPhysicsBody.h"

#include "App.h"

ComponentPhysicsBody::ComponentPhysicsBody(GameObject* gameObject) : Component(gameObject, ComponentType::PHYSICS_BODY)
{
    
}

ComponentPhysicsBody::~ComponentPhysicsBody()
{
}

void ComponentPhysicsBody::OnEditor()
{
    
}
