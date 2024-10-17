#include "Material.h"

using namespace DirectX;

// Create a material using vertex and pixel simple shaders and a color tint
Material::Material(std::shared_ptr<SimpleVertexShader> vShader, std::shared_ptr<SimplePixelShader> pShader, 
	XMFLOAT4 colorTint) : 
	vs(vShader),
	ps(pShader),
	colorTint(colorTint)
{
}

// Getters

XMFLOAT4 Material::GetColorTint() {	return colorTint; }
std::shared_ptr<SimpleVertexShader> Material::GetVertexShader() { return vs; }
std::shared_ptr<SimplePixelShader> Material::GetPixelShader() {	return ps; }

// Setters
void Material::SetColorTint(XMFLOAT4 tint) { colorTint = tint; }
void Material::SetVertexShader(std::shared_ptr<SimpleVertexShader> vShader) { vs = vShader; }
void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> pShader) {	ps = pShader; }
