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
#include "Lights.h"
#include "WICTextureLoader.h"
#include "Sky.h"

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

	void CreateShadowMapResources();
	void RenderShadowMap();

	void UIUpdate(float deltaTime);
	void BuildUI(float deltaTime);

	// UI fields
	bool showDemoUI;
	float backgroundColor[4] = { 0.4f, 0.6f, 0.75f, 0.0f };

	// Vectors to store data for objects in the scene
	std::vector<std::shared_ptr<Mesh>> meshes;
	std::vector<std::shared_ptr<Material>> materials;
	std::vector<std::shared_ptr<Entity>> entities;
	std::vector<std::shared_ptr<Camera>> cameras;
	std::vector<Light> lights;

	// Int for keeping track of which camera is active
	int activeCam = 0;

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Skybox object
	std::shared_ptr<Sky> skybox;

	// Simple shader pointers
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimplePixelShader> uvPS;
	std::shared_ptr<SimplePixelShader> normalPS;
	std::shared_ptr<SimplePixelShader> customPS;
	std::shared_ptr<SimpleVertexShader> skyVS;
	std::shared_ptr<SimplePixelShader> skyPS;
	std::shared_ptr<SimpleVertexShader> shadowVS;

	// Shadow Map resources
	Microsoft::WRL::ComPtr<ID3D11DepthStencilView> shadowDSV;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> shadowSRV;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> shadowRasterizer;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> shadowSampler;
	DirectX::XMFLOAT4X4 lightViewMatrix;
	DirectX::XMFLOAT4X4 lightProjectionMatrix;

	int shadowMapResolution;
};

