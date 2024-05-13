#version 450

layout(binding = 1) uniform sampler g_Sampler;
layout(binding = 2) uniform texture2D g_BaseColorTexture;
layout(binding = 3) uniform texture2D g_NormalTexture;
layout(binding = 4) uniform texture2D g_MetalnessTexture;
layout(binding = 5) uniform texture2D g_RoughnessTexture;
layout(binding = 6) uniform texture2D g_AmbientOcclusionTexture;

layout(location = 0) in vec3 g_InFragmentColor;
layout(location = 1) in vec2 g_InTextureCoordinates;
layout(location = 2) in vec3 g_InNormal;

layout(location = 0) out vec4 g_OutColor;

const vec3 g_LightDirection = vec3(0.0, 1.0, -1.0);

void main()
{
	// Calculate the dot product between the normal and the light direction
    //float diff = max(dot(g_InNormal, g_LightDirection), 0.2);

    // Simple diffuse lighting
    //vec3 diffuse = diff * texture(g_Sampler, g_InTextureCoordinates).rgb; // Assuming white light

	//g_OutColor = vec4(diffuse, 1.0);
    g_OutColor = texture(sampler2D(g_NormalTexture, g_Sampler), g_InTextureCoordinates);
}