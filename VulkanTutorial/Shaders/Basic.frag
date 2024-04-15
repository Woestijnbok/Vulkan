#version 450

layout(binding = 1) uniform sampler2D g_Sampler;

layout(location = 0) in vec3 g_InFragmentColor;
layout(location = 1) in vec2 g_InTextureCoordinates;
layout(location = 2) in vec3 g_InNormal;

layout(location = 0) out vec4 g_OutColor;

const vec3 g_LightDirection = vec3(0.0, 1.0, -1.0);

void main()
{
	// Calculate the dot product between the normal and the light direction
    float diff = max(dot(g_InNormal, g_LightDirection), 0.2);

    // Simple diffuse lighting
    vec3 diffuse = diff * texture(g_Sampler, g_InTextureCoordinates).rgb; // Assuming white light

	g_OutColor = vec4(diffuse, 1.0);
}