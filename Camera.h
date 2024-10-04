#pragma once

#include "Input.h"
#include "Transform.h"
#include <memory>
#include <DirectXMath.h>

class Camera
{
private:

	// Fields
	// Transform shared pointer
	std::shared_ptr<Transform> transform;

	// Camera matrices (view and projection)
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projMatrix;

	// Float variables
	float fov;
	float nClip;
	float fClip;

	float moveSpeed;
	float lookSpeed;

public:

	// Constructors
	Camera(float aspectRatio, DirectX::XMFLOAT3 startPosition);
	Camera(float aspectRatio, DirectX::XMFLOAT3 startPosition, DirectX::XMFLOAT3 startOrientation,
		float fieldOfView, float nearClip, float farClip, float movementSpeed, float lookingSpeed);

	// Getters
	DirectX::XMFLOAT4X4 GetViewMatrix();
	DirectX::XMFLOAT4X4 GetProjectionMatrix();

	std::shared_ptr<Transform> GetTransform();
	float GetFOV();
	float GetNearClip();
	float GetFarClip();

	// Updaters
	void UpdateViewMatrix();
	void UpdateProjectionMatrix(float aspectRatio);

	void Update(float dt);
};

