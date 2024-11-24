#version 450

#extension GL_EXT_samplerless_texture_functions: require

layout(location = 0) in vec2 fragTexcoord;

layout(set = 0, binding = 0) uniform sampler2D gPosition;
layout(set = 0, binding = 1) uniform sampler2D gAlbedo;
layout(set = 0, binding = 2) uniform sampler2D gNormal;
layout(set = 0, binding = 3) uniform sampler2D gARM;
layout(set = 0, binding = 4) uniform Uniform{
    vec3 color;
    vec3 direction;
}ubo;

layout(push_constant) uniform Constant{
    vec3 cameraPosition;
}constants;

layout(location = 0) out vec4 fragColor;

// PBR 函数定义
vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

float distributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float num = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = 3.141592 * denom * denom;

    return num / denom;
}

float geometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float num = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return num / denom;
}

float geometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = geometrySchlickGGX(NdotV, roughness);
    float ggx1 = geometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

void main()
{
    // 从 G-Buffer 中获取数据
    vec3 fragPos = texture(gPosition, fragTexcoord).rgb;
    vec3 normal = texture(gNormal, fragTexcoord).rgb;
    vec3 albedo = texture(gAlbedo, fragTexcoord).rgb;
    vec2 rm = texture(gARM, fragTexcoord).gb;
    float roughness = rm.x;
    float metallic = rm.y;
    vec3 cameraPos = constants.cameraPosition;
    // 计算视角方向和光照方向
    vec3 V = normalize(cameraPos - fragPos);
    vec3 L = normalize(-ubo.direction);
    vec3 H = normalize(V + L);

    // 计算光照强度
    // float distance = length(light.position - fragPos);
    // float attenuation = light.intensity / (distance * distance);
    vec3 radiance = ubo.color * 5.0;

    // F0（反射率在金属度的基础上有所不同）
    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    // Cook-Torrance BRDF
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    float NDF = distributionGGX(normal, H, roughness);
    float G = geometrySmith(normal, V, L, roughness);

    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(normal, V), 0.0) * max(dot(normal, L), 0.0) + 0.001;
    vec3 specular = numerator / denominator;

    // 漫反射
    vec3 kS = F;          // 镜面反射权重
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic; // 金属材料不会有漫反射
    vec3 diffuse = kD * albedo / 3.141592;

    // 计算光照并输出
    float NdotL = max(dot(normal, L), 0.0);
    vec3 ambient = vec3(0.2) * albedo;
    vec3 color = (diffuse + specular) * radiance * NdotL;
    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0/2.2));
    fragColor = vec4(color, 1.0);
}