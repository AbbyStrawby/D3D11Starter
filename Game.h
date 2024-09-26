#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <vector>
#include <memory>

#include "Mesh.h"
#include "BufferStructs.h"
#include "Entity.h"

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
	void LoadShaders();
	void CreateGeometry();

	void UIUpdate(float deltaTime);
	void BuildUI();

	// UI fields
	bool showDemoUI;
	float backgroundColor[4] = { 0.4f, 0.6f, 0.75f, 0.0f };

	VertexShaderData vsData;

	// Vector for storing mesh data
	std::vector<std::shared_ptr<Mesh>> meshes;
	// Vector for storing entity data
	std::vector <std::shared_ptr<Entity>> entities;

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Constant buffer related to vertex shader data
	Microsoft::WRL::ComPtr<ID3D11Buffer> vsConstantBuffer;

	// Shaders and shader-related constructs
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;
};

