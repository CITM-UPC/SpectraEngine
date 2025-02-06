#include "ComponentPhysicsBody.h"

#include "App.h"

ComponentPhysicsBody::ComponentPhysicsBody(GameObject* gameObject) : Component(gameObject, ComponentType::PHYSICS_BODY)
{
    CreateBody();
}

ComponentPhysicsBody::~ComponentPhysicsBody()
{
}

void ComponentPhysicsBody::OnEditor()
{
    
}

void ComponentPhysicsBody::CreateBody()
{
    shape = new JPH::SphereShape(1.0f);

    JPH::CollisionGroup::GroupID group_id = 0;
    JPH::CollisionGroup::SubGroupID sub_group_id = 0;

    bodyCreationSettings->mUserData = reinterpret_cast<JPH::uint64>(this);
	bodyCreationSettings->mObjectLayer = Layers::MOVING;
	bodyCreationSettings->mCollisionGroup = JPH::CollisionGroup(nullptr, group_id, sub_group_id);
    bodyCreationSettings->SetShape(shape);
    bodyCreationSettings->mOverrideMassProperties = JPH::EOverrideMassProperties::MassAndInertiaProvided;
    bodyCreationSettings->mMassPropertiesOverride.mMass = 100.0f;

    bodyCreationSettings->mMassPropertiesOverride.mInertia = JPH::Mat44::sIdentity();

    physicsBody = app->physics->body_interface->CreateBody(*bodyCreationSettings);
	app->physics->body_interface->AddBody(physicsBody->GetID(), JPH::EActivation::Activate);
}
