#pragma once

#include <DirectXMath.h>
#include <memory>
#include <unordered_map>

#include "SimpleShader.h"

class Material
{
private:

	// Fields
	DirectX::XMFLOAT3 colorTint;
	float roughness;				// 0 shiny - 1 rough
	float uvScale;					// Scale uv by this value in shader
	float uvOffset;					// Offset uv by this value in shader

	// SimpleShader pointers
	std::shared_ptr<SimpleVertexShader> vs;
	std::shared_ptr<SimplePixelShader> ps;

	// Texture and Sampler maps
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplers;


public:

	// Constructor
	Material(std::shared_ptr<SimpleVertexShader> vShader, std::shared_ptr<SimplePixelShader> pShader,
		DirectX::XMFLOAT3 colorTint, float roughness, float uvScale, float uvOffset);

	// Getters
	DirectX::XMFLOAT3 GetColorTint();
	float GetRoughness();
	float GetUVScale();
	float GetUVOffset();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	std::shared_ptr<SimplePixelShader> GetPixelShader();

	// Setters
	void SetColorTint(DirectX::XMFLOAT3 tint);
	void SetRoughness(float roughness);
	void SetUVScale(float scale);
	void SetUVOffset(float offset);
	void SetVertexShader(std::shared_ptr<SimpleVertexShader> vShader);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> pShader);

	// Functions
	void AddTextureSRV(std::string name, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv);
	void AddSampler(std::string name, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler);

	void BindTexturesAndSamplers();
};

