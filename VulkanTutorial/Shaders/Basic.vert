#version 450

layout(binding = 0) uniform UniformBufferObject 
{
    mat4 ModelMatrix;
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
} g_Matrices;

layout(location = 0) in vec2 g_InPosition;
layout(location = 1) in vec3 g_InColor;
layout(location = 2) in vec2 g_InTextureCoordinates;

layout(location = 0) out vec3 g_OutFragmentColor;
layout(location = 1) out vec2 g_OutTextureCoordnates;

void main()
{
    gl_Position = g_Matrices.ProjectionMatrix * g_Matrices.ViewMatrix * g_Matrices.ModelMatrix * vec4(g_InPosition, 0.0, 1.0);

	g_OutFragmentColor = g_InColor;
    g_OutTextureCoordnates = g_InTextureCoordinates;
}