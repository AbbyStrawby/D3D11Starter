#pragma once

#include "Mesh.h"
#include "Transform.h"
#include "Camera.h"
#include "Material.h"
#include <memory>

class Entity
{
private:

	// Shared pointers for Mesh and Transform and Material
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Transform> transform;
	std::shared_ptr<Material> material;

public:

	// Constructor
	Entity(std::shared_ptr<Mesh> mesh, std::shared_ptr<Material> material);

	// Getters
	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Transform> GetTransform();
	std::shared_ptr<Material> GetMaterial();

	// Setters
	void SetMesh(std::shared_ptr<Mesh> mesh);
	void SetMaterial(std::shared_ptr<Material> material);

	// Functions
	void Draw(std::shared_ptr<Camera> camera);

};

