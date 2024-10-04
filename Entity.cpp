#include "Entity.h"
#include "BufferStructs.h"
#include "Graphics.h"

using namespace DirectX;

// Create a Entity object using an existing mesh
Entity::Entity(std::shared_ptr<Mesh> mesh)
{
	this->mesh = mesh;
	transform = std::make_shared<Transform>();
}

// Getters
std::shared_ptr<Mesh> Entity::GetMesh()
{
	return mesh;
}

std::shared_ptr<Transform> Entity::GetTransform()
{
	return transform;
}

void Entity::Draw(Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer, std::shared_ptr<Camera> camera)
{
	// Create local vertex shader data for the entity
	VertexShaderData vsData = {};
	vsData.colorTint = XMFLOAT4(1.0f, 0.5f, 0.5f, 1.0f);
	vsData.worldMatrix = transform->GetWorldMatrix();
	vsData.viewMatrix = camera->GetViewMatrix();
	vsData.projectionMatrix = camera->GetProjectionMatrix();

	// Copy Data to the Vertex Shader 
	D3D11_MAPPED_SUBRESOURCE mappedBuffer = {};
	Graphics::Context->Map(vsConstantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedBuffer);

	memcpy(mappedBuffer.pData, &vsData, sizeof(vsData));

	Graphics::Context->Unmap(vsConstantBuffer.Get(), 0);

	// Draw the using Mesh's Draw function to set correct Index/Vertex buffers
	mesh->Draw();
}
