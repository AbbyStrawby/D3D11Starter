#include "Transform.h"

#include <DirectXMath.h>

using namespace DirectX;

// Initialize default transform values
Transform::Transform() :
	position(0, 0, 0),
	rotation(0, 0, 0),
	scale(1, 1, 1)
{
	XMStoreFloat4x4(&worldMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&worldInverseTransposeMatrix, XMMatrixIdentity());
}

// Setters

// Overwrite position value with new x, y, z values
void Transform::SetPosition(float x, float y, float z)
{
	position.x = x;
	position.y = y;
	position.z = z;
}
// Overwrite position value with new XMFLOAT3 value
void Transform::SetPosition(XMFLOAT3 position)
{
	this->position = position;
}

// Overwrite rotation value with new pitch, yaw, roll values
void Transform::SetRotation(float pitch, float yaw, float roll)
{
	rotation.x = pitch;
	rotation.y = yaw;
	rotation.z = roll;
}
// Overwrite rotation value with new XMFLOAT3 value
void Transform::SetRotation(XMFLOAT3 rotation)
{
	this->rotation = rotation;
}

// Overwrite scale value with new x, y, z values
void Transform::SetScale(float x, float y, float z)
{
	scale.x = x;
	scale.y = y;
	scale.z = z;
}
// Overwrite scale value with new XMFLOAT3 value
void Transform::SetScale(XMFLOAT3 scale)
{
	this->scale = scale;
}

// Getters


XMFLOAT3 Transform::GetPosition()
{
	return position;
}
XMFLOAT3 Transform::GetPitchYawRoll()
{
	return rotation;
}
XMFLOAT3 Transform::GetScale()
{
	return scale;
}

// Recalculate and return World Matrix Value
XMFLOAT4X4 Transform::GetWorldMatrix()
{
	// Recalculate the World Matrix
	// translation matrix
	XMVECTOR pos = XMLoadFloat3(&position);
	XMMATRIX t = XMMatrixTranslationFromVector(pos);

	// rotation matrix
	XMVECTOR rot = XMLoadFloat3(&rotation);
	XMMATRIX r = XMMatrixRotationRollPitchYawFromVector(rot);

	// scaling matrix
	XMVECTOR sca = XMLoadFloat3(&scale);
	XMMATRIX s = XMMatrixScalingFromVector(sca);

	// world matrix
	XMMATRIX world = XMMatrixMultiply(XMMatrixMultiply(s, r), t);

	// Store World and Inverse Transpose Matrices in storage types
	XMStoreFloat4x4(&worldMatrix, world);
	XMStoreFloat4x4(&worldInverseTransposeMatrix, XMMatrixInverse(0, XMMatrixTranspose(world)));

	return worldMatrix;
}

// Return world Inverse Transpose Matrix
XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
	return worldInverseTransposeMatrix;
}

// Transformers

// Move object without respect to its orientation
void Transform::MoveAbsolute(float x, float y, float z)
{
	position.x += x;
	position.y += y;
	position.z += z;
}
// Move object without respect to its orientation
void Transform::MoveAbsolute(XMFLOAT3 offset)
{
	MoveAbsolute(offset.x, offset.y, offset.z);
}

// Rotate object by a certain amount
void Transform::Rotate(float pitch, float yaw, float roll)
{
	rotation.x += pitch;
	rotation.y += yaw;
	rotation.z += roll;
}
// Rotate object by a certain amount
void Transform::Rotate(XMFLOAT3 rotation)
{
	Rotate(rotation.x, rotation.y, rotation.z);
}

// Scale object by a certain amount
void Transform::Scale(float x, float y, float z)
{
	scale.x *= x;
	scale.y *= y;
	scale.z *= z;
}
// Scale object by a certain amount
void Transform::Scale(float scale)
{
	Scale(scale, scale, scale);
}
