#include "ShaderIncludes.hlsli"

// Constant buffer for external data
cbuffer ExternalData : register(b0)
{
    float3 colorTint;
    float roughness;
    float uvScale;
    float uvOffset;
    float3 cameraPosition;
    float3 ambient;
    Light lights[5];

}

TextureCube SkyTexture : register(t0); // "t" registers for textures
SamplerState BasicSampler : register(s0); // "s" registers for samplers

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel_Sky input) : SV_TARGET
{
    // Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
    
    return SkyTexture.Sample(BasicSampler, input.sampleDir);
}