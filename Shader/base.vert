#version 450
precision mediump float;

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 texcoord;
layout(location = 2) in vec3 normal;

layout(push_constant) uniform PushConstants {
    mat4 projectionView; 
} pushConstants;

layout(set = 0, binding = 0) uniform Uniforms
{
    mat4 model;
}
uniforms;

layout(location = 0) out vec3 out_color;

void main()
{
    gl_Position = pushConstants.projectionView * uniforms.model * vec4(position, 1.0);

    out_color = normal;
}