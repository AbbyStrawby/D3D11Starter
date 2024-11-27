#include "ShaderIncludes.hlsli"

// Constant buffer for external data
cbuffer ExternalData : register(b0)
{
    float3 colorTint;
    float uvScale;
    float uvOffset;
    float3 cameraPosition;
    Light lights[5];

}

Texture2D Albedo            : register(t0);
Texture2D NormalMap         : register(t1);
Texture2D RoughnessMap      : register(t2);
Texture2D MetalnessMap      : register(t3);
SamplerState BasicSampler   : register(s0);


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
    // re-normalize the incoming normal and tangent
    float3 N = normalize(input.normal);
    float3 T = normalize(input.tangent);
    
    // Create TBN matrix
    T = normalize(T - N * dot(T, N)); // Gram-Schmidt assumes T&N are normalized!
    float3 B = cross(T, N);
    float3x3 TBN = float3x3(T, B, N);
    
    // Adjust uv coordinates by the scale and offset
    input.uv = input.uv * uvScale + uvOffset;
    
    // Get surface color from texture and color tint
    // Be sure to un-gamma correct the surface color so it is accurate when re-corrected later
    float3 albedoColor = pow(Albedo.Sample(BasicSampler, input.uv).rgb, 2.2f);
    
    // Unpack normal from normal map
    float3 unpackedNormal = NormalMap.Sample(BasicSampler, input.uv).rgb * 2 - 1;
    unpackedNormal = normalize(unpackedNormal);
    // Transform unpacked normal by the TBN matrix
    input.normal = mul(unpackedNormal, TBN);
    
    // Unpack roughness and metalness
    float roughness = RoughnessMap.Sample(BasicSampler, input.uv).r;
    float metalness = MetalnessMap.Sample(BasicSampler, input.uv).r;

    // Specular color determination -----------------
    // Assume albedo texture is actually holding specular color where metalness == 1
    // Note the use of lerp here - metal is generally 0 or 1, but might be in between
    // because of linear texture sampling, so we lerp the specular color to match
    float3 specularColor = lerp(F0_NON_METAL, albedoColor.rgb, metalness);
    
    // Final color value to add to with lights
    float3 finalColor = albedoColor.rgb;
    
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
                    roughness, metalness, albedoColor, specularColor);
                break;
            case LIGHT_TYPE_POINT :
                finalColor += PointLight(light, input.normal, cameraPosition, input.worldPosition,
                    roughness, metalness, albedoColor, specularColor);
                break;
        }
    }
    
    // Return a gamma corrected color
    return float4(pow(finalColor, 1.0f / 2.2f), 1);
}