#version 450

layout(binding = 1) uniform sampler2D g_Sampler;

layout(location = 0) in vec3 g_InFragmentColor;
layout(location = 1) in vec2 g_InTextureCoordinates;

layout(location = 0) out vec4 g_OutColor;

void main()
{
	g_OutColor = vec4(g_InFragmentColor *  texture(g_Sampler, g_InTextureCoordinates).rgb, 1.0);
}