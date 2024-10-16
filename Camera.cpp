#include "Camera.h"
#include <algorithm>

using namespace DirectX;

// Construct a Camera object with only essential parameters
Camera::Camera(float aspectRatio, XMFLOAT3 startPosition) :
	Camera(aspectRatio, startPosition, XMFLOAT3(), XM_PIDIV4, 0.1f, 100.0f, 5.0f, 0.05f)
{
}
// Construct a Camera object with extra parameters
Camera::Camera(float aspectRatio, DirectX::XMFLOAT3 startPosition, XMFLOAT3 startOrientation, 
	float fieldOfView, float nearClip, float farClip, float movementSpeed, float lookingSpeed)
{
	// Set initial transform values
	transform = std::make_shared<Transform>();
	transform->SetPosition(startPosition);
	transform->SetRotation(startOrientation);

	// Initialize variables
	fov = fieldOfView;
	nClip = nearClip;
	fClip = farClip;
	moveSpeed = movementSpeed;
	lookSpeed = lookingSpeed;

	// Initialize/Update View and Projection Matrices
	UpdateViewMatrix();
	UpdateProjectionMatrix(aspectRatio);
}

// Getters

// Get Matrices
XMFLOAT4X4 Camera::GetViewMatrix() { return viewMatrix; }
XMFLOAT4X4 Camera::GetProjectionMatrix() { return projMatrix; }

std::shared_ptr<Transform> Camera::GetTransform() {	return transform; }

float Camera::GetFOV(){	return fov; }
float Camera::GetNearClip(){ return nClip; }
float Camera::GetFarClip(){	return fClip; }

// Update the view matrix
// Called once per frame
void Camera::UpdateViewMatrix()
{
	XMFLOAT3 position = transform->GetPosition();
	XMVECTOR pos = XMLoadFloat3(&position);

	XMFLOAT3 forward = transform->GetForward();
	XMVECTOR fwd = XMLoadFloat3(&forward);

	XMFLOAT3 upward = XMFLOAT3(0, 1, 0);
	XMVECTOR up = XMLoadFloat3(&upward);

	XMStoreFloat4x4(&viewMatrix, 
		XMMatrixLookToLH(
			pos,				// Camera position
			fwd,				// Camera forward vector
			up					// World up vector
		));
}
// Update the projection Matrix
// done when camera is initialized and on resizing window
void Camera::UpdateProjectionMatrix(float aspectRatio)
{
	XMStoreFloat4x4(&projMatrix, 
		XMMatrixPerspectiveFovLH(
			fov,			// Camera's field of view
			aspectRatio,	// Current aspect ratio of the window
			nClip,			// Near clipping plane
			fClip			// Far clipping plane
		));
}

// Process input, adjust transform, and update view matrix
void Camera::Update(float dt)
{
	// Input logic
	// Calculate move speed based on delta time
	float ms = moveSpeed * dt;
	// Relative Movement
	if (Input::KeyDown('W')) { transform->MoveRelative(XMFLOAT3(0, 0, ms)); }
	if (Input::KeyDown('S')) { transform->MoveRelative(XMFLOAT3(0, 0, -ms)); }
	if (Input::KeyDown('A')) { transform->MoveRelative(XMFLOAT3(-ms, 0, 0)); }
	if (Input::KeyDown('D')) { transform->MoveRelative(XMFLOAT3(ms, 0, 0)); }

	// Absolute Movement
	if (Input::KeyDown(VK_SPACE)) { transform->MoveAbsolute(XMFLOAT3(0, ms, 0)); }
	if (Input::KeyDown('X')) { transform->MoveAbsolute(XMFLOAT3(0, -ms, 0)); }

	// Mouse movement (camera orientation)
	if (Input::MouseLeftDown()) 
	{
		// Get cursor movement since last frame
		float cursorMovementX = Input::GetMouseXDelta() * lookSpeed;
		float cursorMovementY = Input::GetMouseYDelta() * lookSpeed;

		// Transform based on mouse movement
		// y movement correspeonds to rotation on x axis and vice versa
		transform->Rotate(cursorMovementY, cursorMovementX, 0);

		XMFLOAT3 rot = transform->GetPitchYawRoll();
		if (rot.x > XM_PIDIV2) rot.x = XM_PIDIV2;
		if (rot.x < -XM_PIDIV2) rot.x = -XM_PIDIV2;
		transform->SetRotation(rot);
	}

	// Update view matrix
	UpdateViewMatrix();
}
