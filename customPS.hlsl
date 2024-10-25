#include "ShaderIncludes.hlsli"

// Constant buffer for external data
cbuffer ExternalData : register(b0)
{
    float4 colorTint;
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
    // color value to change and return
    float4 color = float4(0, 0, 0, 0);
    
    // store input values for ease of use
    float4 pos = input.screenPosition;
    float2 uv = input.uv;
    float3 normal = input.normal;
    
    // Make interesting pattern
    color.r = saturate(sin(normal.x * 0.5 + pos.x)); // vertical lines in screen space
    color.g = saturate(cos(normal.y + pos.x * pos.y * pos.z / 2)) * 0.8f;  // weird pattern
    color.b = saturate(sin(normal.z + uv.x + uv.y / 8 * (pos.x + pos.y + pos.z))); // lines across surface
    
    // return color value
    return color;
}