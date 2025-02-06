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
    if (ImGui::CollapsingHeader("Physics Body", ImGuiTreeNodeFlags_DefaultOpen))
    {
        EMotionType currentMotionType = physicsBody->GetMotionType();
        const char* motionTypeNames[] = { "Static", "Kinematic", "Dynamic" };

        if (ImGui::BeginCombo("Motion Type", motionTypeNames[static_cast<int>(currentMotionType)]))
        {
            for (int i = 0; i < 3; i++)
            {
                bool isSelected = (currentMotionType == static_cast<EMotionType>(i));
                if (ImGui::Selectable(motionTypeNames[i], isSelected))
                {
                    currentMotionType = static_cast<EMotionType>(i);
                    physicsBody->SetMotionType(currentMotionType);
                }

                if (isSelected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        if (physicsBody->GetMotionType() != EMotionType::Static && ImGui::TreeNodeEx("Axis Lock", ImGuiTreeNodeFlags_DefaultOpen))
        {
            bool hasChanged = false;

            ImGui::Text("Translation:");
            ImGui::SameLine();
            hasChanged |= ImGui::Checkbox("X##Translation", &translationX);
            ImGui::SameLine();
            hasChanged |= ImGui::Checkbox("Y##Translation", &translationY);
            ImGui::SameLine();
            hasChanged |= ImGui::Checkbox("Z##Translation", &translationZ);

            ImGui::Text("Rotation:     ");
            ImGui::SameLine();
            hasChanged |= ImGui::Checkbox("X##Rotation", &rotationX);
            ImGui::SameLine();
            hasChanged |= ImGui::Checkbox("Y##Rotation", &rotationY);
            ImGui::SameLine();
            hasChanged |= ImGui::Checkbox("Z##Rotation", &rotationZ);

            if (hasChanged)
            {
                EAllowedDOFs allowedDOFs = CalculateAllowedDofs();
                allAxesLockedWarning = allowedDOFs == EAllowedDOFs::None;

                if (!allAxesLockedWarning)
                {
                    JPH::MotionProperties* motionProperties = physicsBody->GetMotionProperties();
                    JPH::MassProperties massProperties = bodyCreationSettings->GetMassProperties();
                    motionProperties->SetMassProperties(allowedDOFs, massProperties);
                }
            }

            if (allAxesLockedWarning)
            {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), "Warning: All rotation axes are locked!");
                ImGui::Text("Do you want to set the body to static?");
                if (ImGui::Button("Set to Static"))
                {
                    translationX = translationY = translationZ = rotationX = rotationY = rotationZ = false;

                    JPH::MotionProperties* motionProperties = physicsBody->GetMotionProperties();
                    JPH::MassProperties massProperties = physicsBody->GetBodyCreationSettings().GetMassProperties();

                    motionProperties->SetMassProperties(EAllowedDOFs::All, massProperties);

                    physicsBody->SetMotionType(EMotionType::Static);
                    allAxesLockedWarning = false;
                }
            }

            ImGui::TreePop();
        }
    }
}

JPH::EAllowedDOFs ComponentPhysicsBody::CalculateAllowedDofs() const
{
    JPH::EAllowedDOFs allowedDOFs = JPH::EAllowedDOFs::All;
    if (physicsBody->GetMotionType() == JPH::EMotionType::Static)
        return allowedDOFs;

    if (translationX)
        allowedDOFs &= ~JPH::EAllowedDOFs::TranslationX;
    if (translationY)
        allowedDOFs &= ~JPH::EAllowedDOFs::TranslationY;
    if (translationZ)
        allowedDOFs &= ~JPH::EAllowedDOFs::TranslationZ;

    if (rotationX)
        allowedDOFs &= ~JPH::EAllowedDOFs::RotationX;
    if (rotationY)
        allowedDOFs &= ~JPH::EAllowedDOFs::RotationY;
    if (rotationZ)
        allowedDOFs &= ~JPH::EAllowedDOFs::RotationZ;

    return allowedDOFs;
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
