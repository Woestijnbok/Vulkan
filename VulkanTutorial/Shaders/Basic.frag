#version 450

// Define constants for RenderType corresponding to the C++ enum values
const int RenderTypeCombined = 0;
const int RenderTypeBaseColor = 1;
const int RenderTypeNormal = 2;
const int RenderTypeGlossiness = 3;
const int RenderTypeSpecular = 4;

layout(push_constant) uniform PushConstants {
    int RenderType;
} g_PushConstants;

layout(binding = 1) uniform sampler g_Sampler;
layout(binding = 2) uniform texture2D g_BaseColorTexture;
layout(binding = 3) uniform texture2D g_NormalTexture;
layout(binding = 4) uniform texture2D g_GlossTexture;
layout(binding = 5) uniform texture2D g_SpecularTexture;

layout(location = 0) in vec2 g_InTextureCoordinates;
layout(location = 1) in vec3 g_InViewDirection;
layout(location = 2) in vec3 g_InNormal;
layout(location = 3) in vec3 g_InTangent;

layout(location = 0) out vec4 g_OutColor;

const float g_PI = 3.14f;
const vec3 g_LightDirection = vec3(0.577, -0.577, 0.577);
const vec3 g_AmbientColor = vec3(0.03, 0.03, 0.03);
const float g_LightIntensity = 7.0;
const float g_Shininess = 25.0;

vec3 CalculateNormal()
{
    vec3 normal;
    
    const vec3 biNormal = normalize(cross(g_InNormal, g_InTangent));
    const mat3 tangentSpaceMatrix = mat3(g_InTangent, biNormal, g_InNormal);
    const vec3 SampledNormal = texture(sampler2D(g_NormalTexture, g_Sampler), g_InTextureCoordinates).rgb;
    normal = normalize(SampledNormal * tangentSpaceMatrix);
   
    return normal;
}

float Phong(float ks, float exponent, vec3 lightDirection, vec3 viewDirection, vec3 normal)
{
    const vec3 reflection = reflect(lightDirection, normal);
    const float cosine = clamp(dot(reflection, viewDirection), 0.0, 1.0);
    const float phong = 1.0 * ks * pow(cosine, exponent);
    
    return phong;
}

void main()
{
    if(g_PushConstants.RenderType == RenderTypeCombined)
    {
	    const vec3 normal = CalculateNormal();
        const float specular = texture(sampler2D(g_SpecularTexture, g_Sampler), g_InTextureCoordinates).r;
        const float phongExponent = texture(sampler2D(g_GlossTexture, g_Sampler), g_InTextureCoordinates).r;
        const float phong = Phong(specular, phongExponent * g_Shininess, g_LightDirection, g_InViewDirection, normal);
        const vec3 diffuseColor = texture(sampler2D(g_BaseColorTexture, g_Sampler), g_InTextureCoordinates).rgb;
        const vec3 specularColor = vec3(phong, phong, phong);
    
        const vec3 color = diffuseColor + specularColor + g_AmbientColor;

        g_OutColor  = vec4(color, 1.0f);
    }
    else if(g_PushConstants.RenderType == RenderTypeBaseColor)
    {
        g_OutColor = texture(sampler2D(g_BaseColorTexture, g_Sampler), g_InTextureCoordinates);
    }
    else if(g_PushConstants.RenderType == RenderTypeNormal)
    {
        g_OutColor = texture(sampler2D(g_NormalTexture, g_Sampler), g_InTextureCoordinates);
    }
    else if(g_PushConstants.RenderType == RenderTypeGlossiness)
    {
        g_OutColor = texture(sampler2D(g_GlossTexture, g_Sampler), g_InTextureCoordinates);
    }
    else
    {
        g_OutColor = texture(sampler2D(g_SpecularTexture, g_Sampler), g_InTextureCoordinates);
    }
}