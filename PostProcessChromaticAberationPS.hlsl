
cbuffer externalData : register(b0)
{
    float redOffset;
    float greenOffset;
    float blueOffset;
    float pixelWidth;
    float pixelHeight;
}

struct VertexToPixel
{
    float4 position : SV_POSITION;
    float2 uv       : TEXCOORD0;
};

Texture2D Pixels : register(t0);
SamplerState ClampSampler : register(s0);

float4 main(VertexToPixel input) : SV_TARGET
{
    // Track the total color and number of samples
    float4 color = 0;
    
    // Horizontal only
    float2 direction = float2(1, 0);
    
    color.r  = Pixels.Sample(ClampSampler, input.uv + (direction * float2(redOffset, redOffset  ))).r;
    color.g  = Pixels.Sample(ClampSampler, input.uv + (direction * float2(greenOffset, greenOffset))).g;
    color.ba = Pixels.Sample(ClampSampler, input.uv + (direction * float2(blueOffset, blueOffset ))).ba;
    
    // Return the average
    return color;
}