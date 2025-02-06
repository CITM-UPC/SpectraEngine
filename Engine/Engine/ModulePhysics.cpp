#include "ModulePhysics.h"

#include "App.h"
#include "ComponentPhysicsBody.h"

ModulePhysics::ModulePhysics(App* app) : Module(app)
{
}

ModulePhysics::~ModulePhysics()
{
}

bool ModulePhysics::Start()
{
	RegisterDefaultAllocator();
	Trace = TraceImpl;
	Factory::sInstance = new Factory();
	RegisterTypes();

	mTempAllocator = std::make_unique<TempAllocatorImpl>(10 * 1024 * 1024);
	mJobSystem = std::make_unique<JobSystemThreadPool>(
		cMaxPhysicsJobs,
		cMaxPhysicsBarriers,
		thread::hardware_concurrency() - 1
	);

	physics_system.Init(cMaxBodies,
		cNumBodyMutexes,
		cMaxBodyPairs,
		cMaxContactConstraints,
		broad_phase_layer_interface,
		object_vs_broadphase_layer_filter,
		object_vs_object_layer_filter);

	body_interface = &physics_system.GetBodyInterface();

	physics_system.SetBodyActivationListener(&body_activation_listener);
	physics_system.SetContactListener(&contact_listener);

	BoxShapeSettings floor_shape_settings(Vec3(100.0f, 1.0f, 100.0f));
	floor_shape_settings.SetEmbedded();

	ShapeSettings::ShapeResult floor_shape_result = floor_shape_settings.Create();
	ShapeRefC floor_shape = floor_shape_result.Get();

	BodyCreationSettings floor_settings(floor_shape, RVec3(0.0_r, -1.0_r, 0.0_r), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
    floor_settings.mRestitution = 0.0f;

	floor = body_interface->CreateBody(floor_settings);

	body_interface->AddBody(floor->GetID(), EActivation::DontActivate);

	physics_system.OptimizeBroadPhase();

	return true;
}

bool ModulePhysics::Update(float dt)
{
    static bool created = false;

    if (!created)
    {
        dynamicObject = app->editor->selectedGameObject->children[0];
        ComponentPhysicsBody* physicsBody = new ComponentPhysicsBody(dynamicObject);
        dynamicObject->AddComponent(physicsBody);
		AddObject(dynamicObject);

        JPH::Body* body = dynamic_cast<ComponentPhysicsBody*>(dynamicObject->GetComponent(ComponentType::PHYSICS_BODY))->physicsBody;

        if (body == nullptr)
        {
            std::cout << "Failed to create physics body for GameObject" << std::endl;
            return false;
        }

        created = true;
    }

    if (app->time.GetState() == GameState::PLAY)
		UpdatePhysics(dt);

	return true;
}

bool ModulePhysics::UpdatePhysics(float dt)
{

	//objects
    for (auto& object : objects)
    {
        if (Body* body = dynamic_cast<ComponentPhysicsBody*>(object->GetComponent(ComponentType::PHYSICS_BODY))->physicsBody)
        {
            if (body_interface->IsActive(body->GetID()))
            {
                RVec3 position = body_interface->GetCenterOfMassPosition(body->GetID());

                object->transform->position = glm::vec3(
                    position.GetX(),
                    position.GetY(),
                    position.GetZ()
                );
                object->transform->UpdateTransform();
            }
        }
    }

    const float cDeltaTime = 1.0f / 60.0f;
    physics_system.Update(cDeltaTime, 1, mTempAllocator.get(), mJobSystem.get());

    return true;
}

bool ModulePhysics::CleanUp()
{
	if (body_interface)
	{
		if (floor)
		{
			body_interface->RemoveBody(floor->GetID());
			body_interface->DestroyBody(floor->GetID());
		}

		for (auto& object : objects)
		{
			if (Body* body = dynamic_cast<ComponentPhysicsBody*>(object->GetComponent(ComponentType::PHYSICS_BODY))->physicsBody)
			{
				body_interface->RemoveBody(body->GetID());
				body_interface->DestroyBody(body->GetID());
			}
		}
	}

	mTempAllocator.reset();
	mJobSystem.reset();

	UnregisterTypes();
	delete Factory::sInstance;
	Factory::sInstance = nullptr;

    return true;
}

void ModulePhysics::AddObject(GameObject* gameObject)
{
    objects.push_back(gameObject);
}
