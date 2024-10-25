#include "Material.h"

using namespace DirectX;

// Create a material using vertex and pixel simple shaders and a color tint
Material::Material(std::shared_ptr<SimpleVertexShader> vShader, std::shared_ptr<SimplePixelShader> pShader, 
	XMFLOAT3 colorTint, float roughness) : 
	vs(vShader),
	ps(pShader),
	colorTint(colorTint),
	roughness(roughness)
{
}

// Getters

XMFLOAT3 Material::GetColorTint() {	return colorTint; }
float Material::GetRoughness() { return roughness; }
std::shared_ptr<SimpleVertexShader> Material::GetVertexShader() { return vs; }
std::shared_ptr<SimplePixelShader> Material::GetPixelShader() {	return ps; }

// Setters
void Material::SetColorTint(XMFLOAT3 tint) { colorTint = tint; }
void Material::SetRoughness(float roughness) { this->roughness = roughness; }
void Material::SetVertexShader(std::shared_ptr<SimpleVertexShader> vShader) { vs = vShader; }
void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> pShader) {	ps = pShader; }
