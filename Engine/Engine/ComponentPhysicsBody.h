#pragma once

#include "Component.h"
#include "ModulePhysics.h"

class ComponentPhysicsBody : public Component
{
public:
	ComponentPhysicsBody(GameObject* gameObject);
	virtual ~ComponentPhysicsBody();

	void OnEditor() override;
	Body* physicsBody = nullptr;

	void CreateBody();

	JPH::ShapeRefC shape;
	JPH::BodyCreationSettings* bodyCreationSettings = new JPH::BodyCreationSettings();

private:

};
