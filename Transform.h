#pragma once

#include <DirectXMath.h>

class Transform
{
private:

	// Transform float values
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT3 rotation;
	DirectX::XMFLOAT3 scale;

	// Transform world matrices
	DirectX::XMFLOAT4X4 worldMatrix;
	DirectX::XMFLOAT4X4 worldInverseTransposeMatrix;

	

public:

	// Constructor
	Transform();

	// Setters - Overwrite transform values
	void SetPosition(float x, float y, float z);
	void SetPosition(DirectX::XMFLOAT3 position);

	void SetRotation(float pitch, float yaw, float roll);
	void SetRotation(DirectX::XMFLOAT3 rotation);

	void SetScale(float x, float y, float z);
	void SetScale(DirectX::XMFLOAT3 scale);

	// Getters - Return transform values
	DirectX::XMFLOAT3 GetPosition();
	DirectX::XMFLOAT3 GetPitchYawRoll();
	DirectX::XMFLOAT3 GetScale();

	DirectX::XMFLOAT4X4 GetWorldMatrix();
	DirectX::XMFLOAT4X4 GetWorldInverseTransposeMatrix();

	// Transformers - Adjust existing transform values
	void MoveAbsolute(float x, float y, float z);
	void MoveAbsolute(DirectX::XMFLOAT3 offset);

	void Rotate(float pitch, float yaw, float roll);
	void Rotate(DirectX::XMFLOAT3 rotation);

	void Scale(float x, float y, float z);
	void Scale(float scale);
};
