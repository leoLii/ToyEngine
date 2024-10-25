#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 texcoord;
layout(location = 2) in vec3 normal;

layout(push_constant) uniform PushConstants {
    mat4 prevPV; 
    mat4 jitteredPV; 
} pushConstants;

layout(set = 0, binding = 0) uniform Uniforms
{
    mat4 prevModel;
    mat4 currModel;
}
uniforms;

layout(location = 0) out vec3 out_normal;
layout(location = 1) out vec3 out_arm;
layout(location = 2) out vec2 out_motionVector;

void main()
{
    gl_Position = pushConstants.prevPV * uniforms.currModel * vec4(position, 1.0);

    out_arm = vec3(1.0, 0.0, 1.0);

    out_normal = normal;

    vec4 currentClipPos = pushConstants.jitteredPV * uniforms.currModel * vec4(position, 1.0);
    vec2 currentNDC = currentClipPos.xy / currentClipPos.w;
    vec4 previousClipPos = pushConstants.prevPV * uniforms.prevModel * vec4(position, 1.0);
    vec2 previousNDC = previousClipPos.xy / previousClipPos.w;
    out_motionVector = currentNDC - previousNDC;
}