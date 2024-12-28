#include "ModuleCamera.h"
#include "App.h"

ModuleCamera::ModuleCamera(App* app) : Module(app)
{
}

ModuleCamera::~ModuleCamera()
{}

bool ModuleCamera::Start()
{
	LOG(LogType::LOG_INFO, "Setting up the camera");

	LookAt(app->scene->sceneCamera->reference);

	return true;
}

bool ModuleCamera::CleanUp()
{
	LOG(LogType::LOG_INFO, "Cleaning camera");

	return true;
}

bool ModuleCamera::Update(float dt)
{
	bool isMouseInside = app->editor->sceneWindow->IsMouseInside();
	if ((!isMouseInside || (!isZooming && !isFreeLook && !isOrbiting && !isDragging)) && !isDefaultCursor)
	{
		SetCursor(CursorType::DEFAULT);
	}

	if (isMouseInside)
		HandleInput();

	app->scene->sceneCamera->Update();

	return true;
}

void ModuleCamera::HandleInput()
{
	glm::vec3 newPos(0, 0, 0);

	float dt = app->GetDT();
	float speed = 10.0f * dt;
	float zoomSpeed = 30.0f * dt;
	float fastSpeed = 20.0f * dt;

	HandleMovement(newPos, speed, fastSpeed);

	HandleZoom(zoomSpeed);

	HandleRotation();

	if (app->input->GetKey(SDL_SCANCODE_F) == KEY_DOWN)
	{
		FrameSelected();
	}

	app->scene->sceneCamera->position += newPos;
	app->scene->sceneCamera->reference += newPos;

	if (glm::length(newPos) > 0.0f)
		app->scene->sceneCamera->frustumNeedsUpdate = true;
}

void ModuleCamera::HandleMovement(glm::vec3& newPos, float speed, float fastSpeed)
{
	if (app->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_REPEAT
		&& app->input->GetKey(SDL_SCANCODE_LALT) == KEY_IDLE)
	{
		if (app->input->GetKey(SDL_SCANCODE_LSHIFT) == KEY_REPEAT) speed = fastSpeed;

		if (app->input->GetKey(SDL_SCANCODE_W) == KEY_REPEAT) newPos -= app->scene->sceneCamera->Z * speed;
		if (app->input->GetKey(SDL_SCANCODE_S) == KEY_REPEAT) newPos += app->scene->sceneCamera->Z * speed;

		if (app->input->GetKey(SDL_SCANCODE_A) == KEY_REPEAT) newPos -= app->scene->sceneCamera->X * speed;
		if (app->input->GetKey(SDL_SCANCODE_D) == KEY_REPEAT) newPos += app->scene->sceneCamera->X * speed;

		SetCursor(CursorType::FREELOOK);
	}
	else if (isFreeLook)
		isFreeLook = false;

	if (app->input->GetMouseButton(SDL_BUTTON_MIDDLE) == KEY_REPEAT)
	{
		int dx = app->input->GetMouseXMotion();
		int dy = app->input->GetMouseYMotion();

		float panSpeed = 0.01f;
		newPos -= app->scene->sceneCamera->X * static_cast<float>(dx) * panSpeed;
		newPos += app->scene->sceneCamera->Y * static_cast<float>(dy) * panSpeed;

		SetCursor(CursorType::DRAG);
		app->scene->sceneCamera->frustumNeedsUpdate = true;
	}
	else if (isDragging)
		isDragging = false;
}

void ModuleCamera::HandleZoom(float zoomSpeed)
{
	int mouseZ = app->input->GetMouseZ();

	if (mouseZ != 0)
	{
		app->scene->sceneCamera->position -= app->scene->sceneCamera->Z * zoomSpeed * (mouseZ > 0 ? 1.0f : -1.0f);

		app->scene->sceneCamera->frustumNeedsUpdate = true;
	}
}

void ModuleCamera::HandleRotation()
{
	int dx = -app->input->GetMouseXMotion();
	int dy = -app->input->GetMouseYMotion();

	if (app->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_REPEAT &&
		app->input->GetKey(SDL_SCANCODE_LALT) == KEY_IDLE)
	{
		RotateCamera(dx, dy);
		app->scene->sceneCamera->frustumNeedsUpdate = true;
	}

	if (app->input->GetMouseButton(SDL_BUTTON_LEFT) == KEY_REPEAT
		&& app->input->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT)
	{
		app->scene->sceneCamera->position -= app->scene->sceneCamera->reference;
		RotateCamera(dx, dy);
		app->scene->sceneCamera->position = app->scene->sceneCamera->reference + app->scene->sceneCamera->Z * glm::length(app->scene->sceneCamera->position);

		LookAt(app->scene->sceneCamera->reference);

		SetCursor(CursorType::ORBIT);
		app->scene->sceneCamera->frustumNeedsUpdate = true;
	}
	else if (isOrbiting)
		isOrbiting = false;

	if (app->input->GetMouseButton(SDL_BUTTON_RIGHT) == KEY_REPEAT
		&& app->input->GetKey(SDL_SCANCODE_LALT) == KEY_REPEAT)
	{
		float sensitivity = 0.01f;
		float zoomDelta = dy * sensitivity;

		glm::vec3 direction = glm::normalize(app->scene->sceneCamera->position - app->scene->sceneCamera->reference);
		app->scene->sceneCamera->position += direction * zoomDelta;

		SetCursor(CursorType::ZOOM);
		app->scene->sceneCamera->frustumNeedsUpdate = true;
	}
	else if (isZooming)
		isZooming = false;
}

void ModuleCamera::RotateCamera(int dx, int dy)
{
	float sensitivity = 0.005f;

	if (dx != 0)
	{
		float DeltaX = (float)dx * sensitivity;

		app->scene->sceneCamera->X = RotateVector(app->scene->sceneCamera->X, DeltaX, glm::vec3(0.0f, 1.0f, 0.0f));
		app->scene->sceneCamera->Y = RotateVector(app->scene->sceneCamera->Y, DeltaX, glm::vec3(0.0f, 1.0f, 0.0f));
		app->scene->sceneCamera->Z = RotateVector(app->scene->sceneCamera->Z, DeltaX, glm::vec3(0.0f, 1.0f, 0.0f));
	}

	if (dy != 0)
	{
		float DeltaY = (float)dy * sensitivity;

		app->scene->sceneCamera->Y = RotateVector(app->scene->sceneCamera->Y, DeltaY, app->scene->sceneCamera->X);
		app->scene->sceneCamera->Z = RotateVector(app->scene->sceneCamera->Z, DeltaY, app->scene->sceneCamera->X);

		if (app->scene->sceneCamera->Y.y < 0.0f)
		{
			app->scene->sceneCamera->Z = glm::vec3(0.0f, app->scene->sceneCamera->Z.y > 0.0f ? 1.0f : -1.0f, 0.0f);
			app->scene->sceneCamera->Y = glm::cross(app->scene->sceneCamera->Z, app->scene->sceneCamera->X);
		}
	}
}

void ModuleCamera::FrameSelected()
{
	if (app->editor->selectedGameObject)
	{
		glm::vec3 selectedPos = glm::vec3(app->editor->selectedGameObject->transform->globalTransform[3]);

		AABB objectAABB = app->editor->selectedGameObject->GetAABB();

		float sizeY = objectAABB.max.y - objectAABB.min.y;
		float sizeZ = objectAABB.max.z - objectAABB.min.z;

		glm::vec3 newPos = glm::vec3(
			selectedPos.x,
			selectedPos.y + sizeY + 5.0f,
			selectedPos.z + sizeZ + 5.0f
		);

		app->scene->sceneCamera->position = newPos;
		app->scene->sceneCamera->reference = selectedPos;

		LookAt(selectedPos);
	}
	else
	{
		glm::vec3 defaultPos = glm::vec3(0.0f, 15.0f, 15.0f);
		glm::vec3 defaultRef = glm::vec3(0.0f, 0.0f, 0.0f);

		app->scene->sceneCamera->position = defaultPos;
		app->scene->sceneCamera->reference = defaultRef;

		LookAt(defaultRef);
	}

	app->scene->sceneCamera->frustumNeedsUpdate = true;
}

void ModuleCamera::LookAt(const glm::vec3& spot)
{
	app->scene->sceneCamera->reference = spot;

	app->scene->sceneCamera->Z = glm::normalize(app->scene->sceneCamera->position - app->scene->sceneCamera->reference);
	app->scene->sceneCamera->X = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), app->scene->sceneCamera->Z));
	app->scene->sceneCamera->Y = glm::cross(app->scene->sceneCamera->Z, app->scene->sceneCamera->X);

	app->scene->sceneCamera->CalculateViewMatrix();
}

glm::vec3 ModuleCamera::RotateVector(glm::vec3 const& vector, float angle, glm::vec3 const& axis)
{
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), angle, axis);

	glm::vec4 vector4 = glm::vec4(vector, 1.0f);

	glm::vec4 rotatedVector = rotationMatrix * vector4;

	return glm::vec3(rotatedVector);
}

void ModuleCamera::SetCursor(CursorType cursorType)
{
	if (app->input->GetCursor() != cursorType)
	{
		app->input->ChangeCursor(cursorType);

		isDefaultCursor = (cursorType == CursorType::DEFAULT);
		isFreeLook = (cursorType == CursorType::FREELOOK);
		isZooming = (cursorType == CursorType::ZOOM);
		isOrbiting = (cursorType == CursorType::ORBIT);
		isDragging = (cursorType == CursorType::DRAG);
	}
}