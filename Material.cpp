#include "Material.h"

using namespace DirectX;

// Create a material using vertex and pixel simple shaders and a color tint
Material::Material(std::shared_ptr<SimpleVertexShader> vShader, std::shared_ptr<SimplePixelShader> pShader, 
	XMFLOAT3 colorTint, float roughness, float uvScale, float uvOffset) :
	vs(vShader),
	ps(pShader),
	colorTint(colorTint),
	roughness(roughness),
	uvScale(uvScale),
	uvOffset(uvOffset)
{
}

// Getters

XMFLOAT3 Material::GetColorTint() {	return colorTint; }
float Material::GetRoughness() { return roughness; }
float Material::GetUVScale() { return uvScale; }
float Material::GetUVOffset() { return uvOffset; }
std::shared_ptr<SimpleVertexShader> Material::GetVertexShader() { return vs; }
std::shared_ptr<SimplePixelShader> Material::GetPixelShader() {	return ps; }

// Setters
void Material::SetColorTint(XMFLOAT3 tint) { colorTint = tint; }
void Material::SetRoughness(float roughness) { this->roughness = roughness; }
void Material::SetUVScale(float scale) { uvScale = scale; }
void Material::SetUVOffset(float offset) { uvOffset = offset; }
void Material::SetVertexShader(std::shared_ptr<SimpleVertexShader> vShader) { vs = vShader; }
void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> pShader) {	ps = pShader; }

void Material::AddTextureSRV(std::string name, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
	textureSRVs.insert({ name, srv });
}

void Material::AddSampler(std::string name, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler)
{
	samplers.insert({ name, sampler });
}

void Material::BindTexturesAndSamplers()
{
	for (auto& t : textureSRVs) { ps->SetShaderResourceView(t.first.c_str(), t.second); }
	for (auto& s : samplers) { ps->SetSamplerState(s.first.c_str(), s.second); }
}
