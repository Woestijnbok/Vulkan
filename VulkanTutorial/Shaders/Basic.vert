#version 450

layout(binding = 0) uniform UniformBufferObject 
{
    mat4 ModelMatrix;
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
    vec3 CameraPosition;
} g_Matrices;

layout(location = 0) in vec3 g_InPosition;
layout(location = 1) in vec3 g_InColor;
layout(location = 2) in vec2 g_InTextureCoordinates;
layout(location = 3) in vec3 g_InNormal;

layout(location = 0) out vec3 g_OutFragmentColor;
layout(location = 1) out vec2 g_OutTextureCoordnates;
layout(location = 2) out vec3 g_OutNormal;

void main()
{
    gl_Position = g_Matrices.ProjectionMatrix * g_Matrices.ViewMatrix * g_Matrices.ModelMatrix * vec4(g_InPosition, 1.0);
    g_OutNormal = normalize((g_Matrices.ModelMatrix * vec4(g_InNormal,0)).xyz);
	g_OutFragmentColor = g_InColor;
    g_OutTextureCoordnates = g_InTextureCoordinates;
}