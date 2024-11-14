#ifndef __GGP_SHADER_INCLUDES__ // Each .hlsli file needs a unique identifier!
#define __GGP_SHADER_INCLUDES__

// Structs

// Struct representing a single vertex worth of data
struct VertexShaderInput
{
    float3 localPosition    : POSITION; // XYZ position
    float2 uv               : TEXCOORD; // UV texture coordinate
    float3 normal           : NORMAL;   // Surface normal vector
    float3 tangent          : TANGENT;   // Vector tangent to the normal
};

// Struct representing the data we're sending down the pipeline
// - At a minimum, we need a piece of data defined tagged as SV_POSITION
struct VertexToPixel
{
    float4 screenPosition   : SV_POSITION;  // XYZW position (System Value Position)
    float2 uv               : TEXCOORD;     // UV texture coordinate
    float3 normal           : NORMAL;       // Surface normal vector
    float3 tangent          : TANGENT;      // Vector tangent to the normal
    float3 worldPosition    : POSITION;     // position in world space
};

// Struct for data in the Skybox shaders
struct VertexToPixel_Sky
{
    float4 screenPosition   : SV_POSITION;  // XYZW position (System Value Position)
    float3 sampleDir        : DIRECTION;    // Direction to sample from 
};

// Three types of light
#define LIGHT_TYPE_DIRECTIONAL  0
#define LIGHT_TYPE_POINT        1
#define LIGHT_TYPE_SPOT         2

#define MAX_SPECULAR_EXPONENT   256.0f

// Struct representing a Light
struct Light
{
    int Type;
    float3 Direction;
    float Range;
    float3 Position;
    float Intensity;
    float3 Color;
    float SpotInnerAngle;
    float SpotOuterAngle;
    float2 Padding;
};

// Lighting functions

float Attenuate(Light light, float3 worldPos)
{
    float dist = distance(light.Position, worldPos);
    float att = saturate(1.0f - (dist * dist / (light.Range * light.Range)));
    return att * att;
}

float Diffuse(float3 normal, float3 dirToLight)
{
    return saturate(dot(normal, dirToLight));
}

float Specular(float3 reflection, float3 view, float roughness)
{
    float specExponent = (1.0f - roughness) * MAX_SPECULAR_EXPONENT;
    
    return pow(saturate(dot(reflection, view)), specExponent);
}

float3 DirectionalLight(Light light, float3 normal, float3 cameraPos, float3 worldPos, float roughness, 
    float3 surfaceColor, float specScale)
{
    // Calculate important vectors
    float3 dirToLight = normalize(-light.Direction);
    float3 dirToCam = normalize(cameraPos - worldPos);
    float3 refl = reflect(light.Direction, normal);
        
    // Calculate diffuse/specular colors
    float3 diffuse = Diffuse(normal, dirToLight);    
    float3 specular = Specular(refl, dirToCam, roughness) * specScale;
    
    return (diffuse * surfaceColor + specular) * light.Color * light.Intensity;
}

float3 PointLight(Light light, float3 normal, float3 cameraPos, float3 worldPos, float roughness, 
    float3 surfaceColor, float specScale)
{
    // Calculate important vectors
    float3 dirToLight = normalize(light.Position - worldPos);
    float3 dirToCam = normalize(cameraPos - worldPos);
    float3 refl = reflect(-dirToLight, normal);
        
    // Calculate attenuation and diffuse/specular colors
    float atten = Attenuate(light, worldPos);
    float3 diffuse = Diffuse(normal, dirToLight);
    float3 specular = Specular(refl, dirToCam, roughness) * specScale;
    
    return (diffuse * surfaceColor + specular) * light.Color * light.Intensity * atten;
}

#endif