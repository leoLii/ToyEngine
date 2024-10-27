#version 450

layout(location = 0) in vec2 inVertex;
layout(location = 1) in vec2 inTexcoord;

layout(location = 0) out vec2 fragTexcoord;

void main()
{
    gl_Position = vec4(inVertex.xy, 0.0, 1.0);
    fragTexcoord = inTexcoord;
}