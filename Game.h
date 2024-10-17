#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <vector>
#include <memory>

#include "Mesh.h"
#include "Entity.h"
#include "Camera.h"
#include "SimpleShader.h"
#include "Material.h"

class Game
{
public:
	// Basic OOP setup
	Game() = default;
	~Game();
	Game(const Game&) = delete; // Remove copy constructor
	Game& operator=(const Game&) = delete; // Remove copy-assignment operator

	// Primary functions
	void Initialize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);
	void OnResize();

private:

	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void LoadShadersAndCreateGeometry();

	void UIUpdate(float deltaTime);
	void BuildUI(float deltaTime);

	// UI fields
	bool showDemoUI;
	float backgroundColor[4] = { 0.4f, 0.6f, 0.75f, 0.0f };

	// Vector for storing mesh data
	std::vector<std::shared_ptr<Mesh>> meshes;
	// Vector for storing material data
	std::vector<std::shared_ptr<Material>> materials;
	// Vector for storing entity data
	std::vector<std::shared_ptr<Entity>> entities;
	// Vector for storing camera data
	std::vector<std::shared_ptr<Camera>> cameras;

	// Int for keeping track of which camera is active
	int activeCam = 0;

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Simple shader pointers
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimplePixelShader> uvPS;
	std::shared_ptr<SimplePixelShader> normalPS;
	std::shared_ptr<SimplePixelShader> customPS;
};

