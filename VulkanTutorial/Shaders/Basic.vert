#version 450

layout(binding = 0) uniform UniformBufferObject 
{
    mat4 ModelMatrix;
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
    vec3 CameraPosition;
} g_UBO;

layout(location = 0) in vec3 g_InPosition;
layout(location = 1) in vec3 g_InColor;
layout(location = 2) in vec2 g_InTextureCoordinates;
layout(location = 3) in vec3 g_InNormal;
layout(location = 4) in vec3 g_InTangent;

layout(location = 0) out vec2 g_OutTextureCoordinates;
layout(location = 1) out vec3 g_OutViewDirection;
layout(location = 2) out vec3 g_OutNormal;
layout(location = 3) out vec3 g_OutTangent;

void main()
{
    gl_Position = g_UBO.ProjectionMatrix * g_UBO.ViewMatrix * g_UBO.ModelMatrix * vec4(g_InPosition, 1.0);
    g_OutTextureCoordinates = g_InTextureCoordinates;
    g_OutViewDirection = normalize((g_UBO.ModelMatrix * vec4(g_InPosition,0)).xyz - g_UBO.CameraPosition);
    g_OutNormal = normalize((g_UBO.ModelMatrix * vec4(g_InNormal,0)).xyz);
    g_OutTangent = normalize((g_UBO.ModelMatrix * vec4(g_InTangent,0)).xyz);
}