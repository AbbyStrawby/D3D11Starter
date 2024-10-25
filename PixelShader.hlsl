#include "ShaderIncludes.hlsli"

// Constant buffer for external data
cbuffer ExternalData : register(b0)
{
    float3 colorTint;
    float roughness;
    float3 cameraPosition;
    float3 ambient;
    Light lights[5];

}

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
    
    
    float3 finalColor = ambient * colorTint;
    
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
                    roughness, colorTint);
                break;
            case LIGHT_TYPE_POINT :
                finalColor += PointLight(light, input.normal, cameraPosition, input.worldPosition,
                    roughness, colorTint);
                break;
        }
    }
    
    return float4(finalColor, 1);
}