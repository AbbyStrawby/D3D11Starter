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

Texture2D SurfaceTexture : register(t0); // "t" registers for textures
Texture2D SpecularMap : register(t1); // "t" registers for textures
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
float4 main(VertexToPixel input) : SV_TARGET
{
    // re-normalize the incoming normals
    input.normal = normalize(input.normal);
    
    // Adjust uv coordinates by the scale and offset
    input.uv = input.uv * uvScale + uvOffset;
    
    // Get surface color from texture and color tint
    float3 surfaceColor = SurfaceTexture.Sample(BasicSampler, input.uv).rgb;
    surfaceColor *= colorTint;
    
    // Get specular value from Specular Map
    float specScale = SpecularMap.Sample(BasicSampler, input.uv).r;
    
    // Final color value to add to with lights
    float3 finalColor = ambient * surfaceColor;
    
    // Loop through lights
    for (int i = 0; i < 5; i++)
    {
        // Calculate normalized direction to the light
        Light light = lights[i];
        light.Direction = normalize(light.Direction);
        
        // Check what type of light needs to be calculated
        switch (light.Type)
        {
            case LIGHT_TYPE_DIRECTIONAL:
                finalColor += DirectionalLight(light, input.normal, cameraPosition, input.worldPosition,
                    roughness, surfaceColor, specScale);
                break;
            case LIGHT_TYPE_POINT :
                finalColor += PointLight(light, input.normal, cameraPosition, input.worldPosition,
                    roughness, surfaceColor, specScale);
                break;
        }
    }
    
    return float4(finalColor, 1);
}