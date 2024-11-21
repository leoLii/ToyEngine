#version 450

#extension GL_ARB_shader_draw_parameters: require

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexcoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;

layout(push_constant) uniform PushConstants {
    mat4 prevPV; 
    mat4 jitteredPV;
    vec2 prevJitter;
    vec2 currJitter;
    int id;
};

struct Transform{
    mat4 prevModel;
    mat4 currModel;
};

layout(set = 0, binding = 0) uniform Uniforms
{
    Transform transforms[100];
};

layout(location = 0) out vec3 fragPosition;
layout(location = 1) out vec2 fragTexcoord;
layout(location = 2) out vec3 fragNormal;
layout(location = 3) out vec3 fragTangent;
layout(location = 4) out vec2 fragMotionVector;
layout(location = 5) out flat int out_id;

void main()
{
    Transform transform = transforms[gl_DrawIDARB];
    vec4 worldPosition = transform.currModel * vec4(inPosition, 1.0);
    vec4 prevWorldPosition = transform.prevModel * vec4(inPosition, 1.0);
    fragPosition = worldPosition.xyz;
    fragTexcoord = inTexcoord;
    fragNormal = normalize((transform.currModel * vec4(inNormal, 1.0)).xyz);
    //fragNormal = inNormal;
    fragTangent = (transform.currModel * vec4(inTangent, 1.0)).xyz;

    vec4 currentClipPos = jitteredPV * worldPosition;
    vec2 currentNDC = currentClipPos.xy / currentClipPos.w;
    vec4 previousClipPos = prevPV * prevWorldPosition;
    vec2 previousNDC = previousClipPos.xy / previousClipPos.w;
    vec2 cancelJitter = prevJitter - currJitter;
    // Transform motion vectors from NDC space to UV space (+Y is top-down).
    fragMotionVector = (currentNDC - previousNDC) * vec2(0.5, 0.5) ;
    gl_Position = jitteredPV * worldPosition;
    out_id = id;
}