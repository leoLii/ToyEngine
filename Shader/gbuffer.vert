#version 450

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexcoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;

layout(push_constant) uniform PushConstants {
    mat4 prevPV; 
    mat4 jitteredPV;
    vec2 prevJitter;
    vec2 currJitter;
} constant;

layout(set = 0, binding = 0) uniform Uniforms
{
    mat4 prevModel;
    mat4 currModel;
} ubo;

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec2 fragTexcoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragTangent;
layout(location = 4) out vec2 fragMotionVector;

void main()
{
    vec4 worldPosition = ubo.currModel * vec4(inPosition, 1.0);
    fragPosition = worldPosition.xyz;
    fragTexcoord = inTexcoord;
    fragNormal = normalize(mat3(ubo.currModel) * inNormal);
    //fragNormal = inNormal;
    fragTangent = mat3(ubo.currModel) * inTangent;

    vec4 currentClipPos = constant.jitteredPV * ubo.currModel * vec4(inPosition, 1.0);
    vec2 currentNDC = currentClipPos.xy / currentClipPos.w;
    vec4 previousClipPos = constant.prevPV * ubo.prevModel * vec4(inPosition, 1.0);
    vec2 previousNDC = previousClipPos.xy / previousClipPos.w;
    vec2 cancelJitter = constant.prevJitter - constant.currJitter;
    // Transform motion vectors from NDC space to UV space (+Y is top-down).
    fragMotionVector = (currentNDC - previousNDC) * vec2(0.5, 0.5) ;
    gl_Position = constant.jitteredPV * worldPosition;
}