#include "Entity.h"
#include "Graphics.h"
#include "SimpleShader.h"

using namespace DirectX;

// Create a Entity object using an existing mesh
Entity::Entity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material)
{
	this->mesh = mesh;
	this->material = material;
	transform = std::make_shared<Transform>();
}

// Getters

std::shared_ptr<Mesh> Entity::GetMesh() { return mesh; }
std::shared_ptr<Transform> Entity::GetTransform() {	return transform; }
std::shared_ptr<Material> Entity::GetMaterial() { return material; }

// Setters

void Entity::SetMesh(std::shared_ptr<Mesh> mesh) { this->mesh = mesh; }
void Entity::SetMaterial(std::shared_ptr<Material> material) { this->material = material; }

// Functions

void Entity::Draw(std::shared_ptr<Camera> camera)
{
	

	// Get shaders for this game entity
	std::shared_ptr<SimpleVertexShader> vs = material->GetVertexShader();
	std::shared_ptr<SimplePixelShader> ps = material->GetPixelShader();

	// Create shader buffer data
	// Ensure names exactly match names in shader buffer
	vs->SetMatrix4x4("world", transform->GetWorldMatrix());
	vs->SetMatrix4x4("view", camera->GetViewMatrix());
	vs->SetMatrix4x4("projection", camera->GetProjectionMatrix());
	vs->SetMatrix4x4("worldInvTranspose", transform->GetWorldInverseTransposeMatrix());

	ps->SetFloat3("colorTint", material->GetColorTint());
	ps->SetFloat("uvScale", material->GetUVScale());
	ps->SetFloat("uvOffset", material->GetUVOffset());
	ps->SetFloat3("cameraPosition", camera->GetTransform()->GetPosition());

	// Copy Data to the Vertex Shader 
	vs->CopyAllBufferData();
	ps->CopyAllBufferData();

	// Bind the material's textures and samplers
	material->BindTexturesAndSamplers();

	// Activate the shaders for this mesh's materials before drawing
	vs->SetShader();
	ps->SetShader();

	// Draw the using Mesh's Draw function to set correct Index/Vertex buffers
	mesh->Draw();
}
