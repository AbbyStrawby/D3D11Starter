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
	dirtyMatrices = false;
}

// Setters

// Overwrite position value with new x, y, z values
void Transform::SetPosition(float x, float y, float z)
{
	position.x = x;
	position.y = y;
	position.z = z;
	dirtyMatrices = true;
}
// Overwrite position value with new XMFLOAT3 value
void Transform::SetPosition(XMFLOAT3 position)
{
	this->position = position;
	dirtyMatrices = true;
}

// Overwrite rotation value with new pitch, yaw, roll values
void Transform::SetRotation(float pitch, float yaw, float roll)
{
	rotation.x = pitch;
	rotation.y = yaw;
	rotation.z = roll;
	dirtyMatrices = true;
}
// Overwrite rotation value with new XMFLOAT3 value
void Transform::SetRotation(XMFLOAT3 rotation)
{
	this->rotation = rotation;
	dirtyMatrices = true;
}

// Overwrite scale value with new x, y, z values
void Transform::SetScale(float x, float y, float z)
{
	scale.x = x;
	scale.y = y;
	scale.z = z;
	dirtyMatrices = true;
}
// Overwrite scale value with new XMFLOAT3 value
void Transform::SetScale(XMFLOAT3 scale)
{
	this->scale = scale;
	dirtyMatrices = true;
}

// Getters

// Return transformation values
XMFLOAT3 Transform::GetPosition() {	return position; }
XMFLOAT3 Transform::GetPitchYawRoll() {	return rotation; }
XMFLOAT3 Transform::GetScale() { return scale; }

// Calculate and return the directional vectors relative to transform's orientation
DirectX::XMFLOAT3 Transform::GetRight()   { return GetRelativeVector(XMFLOAT3(1, 0, 0)); }
DirectX::XMFLOAT3 Transform::GetUp()	  { return GetRelativeVector(XMFLOAT3(0, 1, 0)); }
DirectX::XMFLOAT3 Transform::GetForward() {	return GetRelativeVector(XMFLOAT3(0, 0, 1)); }

// Recalculate and return World Matrix Value
XMFLOAT4X4 Transform::GetWorldMatrix()
{
	RecalculateMatrices();
	return worldMatrix;
}

// Return world Inverse Transpose Matrix
XMFLOAT4X4 Transform::GetWorldInverseTransposeMatrix()
{
	RecalculateMatrices();
	return worldInverseTransposeMatrix;
}

// Transformers

// Move object without respect to its orientation
void Transform::MoveAbsolute(float x, float y, float z)
{
	position.x += x;
	position.y += y;
	position.z += z;
	dirtyMatrices = true;
}
// Move object without respect to its orientation
void Transform::MoveAbsolute(XMFLOAT3 offset)
{
	MoveAbsolute(offset.x, offset.y, offset.z);
	dirtyMatrices = true;
}

// Move object with respect to its orientation
void Transform::MoveRelative(float x, float y, float z)
{
	MoveRelative(XMFLOAT3(x, y, z));
	dirtyMatrices = true;
}
// Move object with respect to its orientation
void Transform::MoveRelative(DirectX::XMFLOAT3 offset)
{
	// Get a relative version of the offset vector
	XMFLOAT3 relativeOffset = GetRelativeVector(offset);
	// Load into math type
	XMVECTOR move = XMLoadFloat3(&relativeOffset);

	// Move the position by the new rotated offset
	XMVECTOR pos = XMLoadFloat3(&position);
	pos = XMVectorAdd(pos, move);

	// Store the pos value back in position storage type
	XMStoreFloat3(&position, pos);

	dirtyMatrices = true;
}

// Rotate object by a certain amount
void Transform::Rotate(float pitch, float yaw, float roll)
{
	rotation.x += pitch;
	rotation.y += yaw;
	rotation.z += roll;
	dirtyMatrices = true;
}
// Rotate object by a certain amount
void Transform::Rotate(XMFLOAT3 rotation)
{
	Rotate(rotation.x, rotation.y, rotation.z);
	dirtyMatrices = true;
}

// Scale object by a certain amount
void Transform::Scale(float x, float y, float z)
{
	scale.x *= x;
	scale.y *= y;
	scale.z *= z;
	dirtyMatrices = true;
}
// Scale object by a certain amount
void Transform::Scale(float scale)
{
	Scale(scale, scale, scale);
	dirtyMatrices = true;
}

// Helpers

// Take a vector in absolute space and rotate it to be relative to the transform's orientation
DirectX::XMFLOAT3 Transform::GetRelativeVector(DirectX::XMFLOAT3 absVector)
{
	// Load the absolute vector as a math type
	XMVECTOR absVec = XMLoadFloat3(&absVector);
	// Quaternrion representing current rotation
	XMVECTOR quat = XMQuaternionRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);

	// Rotate the vector by the quaternion to get the vector in relative space
	XMVECTOR relVec = XMVector3Rotate(absVec, quat);

	XMFLOAT3 ret;
	XMStoreFloat3(&ret, relVec);

	return ret;
}

// Recalculate the World and WorldInverseTranspose Matrices
void Transform::RecalculateMatrices()
{
	if (!dirtyMatrices)
		return;

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

	dirtyMatrices = false;
}
