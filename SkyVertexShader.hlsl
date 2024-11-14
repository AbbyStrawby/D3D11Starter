#include "ShaderIncludes.hlsli"

// Constant buffer for external data
cbuffer ExternalData : register(b0)
{
    matrix view;
    matrix projection;
}

// --------------------------------------------------------
// The entry point (main method) for our vertex shader
// 
// - Input is exactly one vertex worth of data (defined by a struct)
// - Output is a single struct of data to pass down the pipeline
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
VertexToPixel_Sky main(VertexShaderInput input)
{
	// Set up output struct
    VertexToPixel_Sky output;
    
    // Copy of view matrix without the translation part
    matrix viewNoTranslation = view;    
    viewNoTranslation._14 = 0;
    viewNoTranslation._24 = 0;
    viewNoTranslation._34 = 0;
    
    // Apply projection and view by the local position to get screen position
    matrix vp = mul(projection, viewNoTranslation);
    output.screenPosition = mul(vp, float4(input.localPosition, 1.0f));
    
    // Set sky z value to be w so it is exactly on the far clip plane
    output.screenPosition.z = output.screenPosition.w;
    
    // Use position of the vertex for the direction in the cube map
    output.sampleDir = input.localPosition;

    // Whatever we return will make its way through the pipeline to the
	// next programmable stage we're using (the pixel shader for now)
    return output;
}