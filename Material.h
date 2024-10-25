#pragma once

#include <DirectXMath.h>
#include <memory>

#include "SimpleShader.h"

class Material
{
private:

	// Fields
	DirectX::XMFLOAT3 colorTint;
	float roughness;				// 0 shiny - 1 rough

	// SimpleShader pointers
	std::shared_ptr<SimpleVertexShader> vs;
	std::shared_ptr<SimplePixelShader> ps;

public:

	// Constructor
	Material(std::shared_ptr<SimpleVertexShader> vShader, std::shared_ptr<SimplePixelShader> pShader,
		DirectX::XMFLOAT3 colorTint, float roughness);

	// Getters
	DirectX::XMFLOAT3 GetColorTint();
	float GetRoughness();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	std::shared_ptr<SimplePixelShader> GetPixelShader();

	// Setters
	void SetColorTint(DirectX::XMFLOAT3 tint);
	void SetRoughness(float roughness);
	void SetVertexShader(std::shared_ptr<SimpleVertexShader> vShader);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> pShader);
};

