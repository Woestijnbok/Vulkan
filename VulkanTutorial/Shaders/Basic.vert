#version 450

layout(location = 0) in vec2 g_InPosition;
layout(location = 1) in vec3 g_InColor;

layout(location = 0) out vec3 g_OutFragmentColor;

void main()
{
	gl_Position = vec4(g_InPosition, 0.0, 1.0);
	g_OutFragmentColor = g_InColor;
}