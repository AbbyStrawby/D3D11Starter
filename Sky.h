#pragma once

#include "Mesh.h"
#include "SimpleShader.h"
#include "Camera.h"
#include "WICTextureLoader.h"

#include <memory>
#include <wrl/client.h> 

class Sky
{
private:

	// Resources
	Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerOptions;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> skySRV;
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> skyDepthState;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> skyRasterState;

	std::shared_ptr<Mesh> skyBoxMesh;

	std::shared_ptr<SimpleVertexShader> skyVS;
	std::shared_ptr<SimplePixelShader> skyPS;

	// Helpers
	
	void CreateRenderStates();
	
	// Helper for creating a cubemap from 6 individual textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> CreateCubemap(
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back);


public:

	// Constructor
	Sky(
		std::shared_ptr<Mesh> mesh,
		Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerOptions,
		std::shared_ptr<SimpleVertexShader> skyVS,
		std::shared_ptr<SimplePixelShader> skyPS,
		const wchar_t* right,
		const wchar_t* left,
		const wchar_t* up,
		const wchar_t* down,
		const wchar_t* front,
		const wchar_t* back
	);

	// Getters
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetSkyTexture();

	// Functions
	void Draw(std::shared_ptr<Camera> camera);

};

