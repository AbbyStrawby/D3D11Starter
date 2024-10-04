#pragma once

#include "Mesh.h"
#include "Transform.h"
#include "Camera.h"
#include <memory>
#include <wrl/client.h>

class Entity
{
private:

	// Shared pointers for Mesh and Transform
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Transform> transform;

public:

	// Constructor
	Entity(std::shared_ptr<Mesh> mesh);

	// Getters
	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Transform> GetTransform();

	// Functions
	void Draw(Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer, std::shared_ptr<Camera> camera);

};

