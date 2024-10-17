#pragma once

#include <DirectXMath.h>
#include <memory>

#include "SimpleShader.h"

class Material
{
private:

	// Fields
	DirectX::XMFLOAT4 colorTint;

	// SimpleShader pointers
	std::shared_ptr<SimpleVertexShader> vs;
	std::shared_ptr<SimplePixelShader> ps;

public:

	// Constructor
	Material(std::shared_ptr<SimpleVertexShader> vShader, std::shared_ptr<SimplePixelShader> pShader,
		DirectX::XMFLOAT4 colorTint);

	// Getters
	DirectX::XMFLOAT4 GetColorTint();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	std::shared_ptr<SimplePixelShader> GetPixelShader();

	// Setters
	void SetColorTint(DirectX::XMFLOAT4 tint);
	void SetVertexShader(std::shared_ptr<SimpleVertexShader> vShader);
	void SetPixelShader(std::shared_ptr<SimplePixelShader> pShader);
};

