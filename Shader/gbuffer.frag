#version 450

layout(location = 0) in vec3 fragPosition;
layout(location = 1) in vec2 fragTexcoord;
layout(location = 2) in vec3 fragNormal;
layout(location = 3) in vec3 fragTangent;
layout(location = 4) in vec2 fragMotionVector;

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gAlbedo;
layout(location = 2) out vec4 gNormal;
layout(location = 3) out vec4 gARM;
layout(location = 4) out vec2 gMotion;

void main()
{
	// vec3 albedo = texture(albedoMap, fragTexCoord).rgb;
    // float metallic = texture(metallicMap, fragTexCoord).r;
    // float roughness = texture(roughnessMap, fragTexCoord).r;
	// vec3 normal = texture(normalMap, fragTexCoord).rgb;
    // normal = normalize(normal * 2.0 - 1.0); // 转换为 [-1,1] 范围
    // vec3 T = normalize(fragTangent);
    // vec3 N = normalize(fragNormal);
    // vec3 B = cross(N, T);
    // mat3 TBN = mat3(T, B, N);               // 构建TBN矩阵
    // vec3 worldNormal = normalize(TBN * normal);  // 转换法线到世界空间
	gPosition = vec4(fragPosition, 1.0);
	gAlbedo = vec4(1.0, 0.0, 0.0, 1.0);
	gNormal = vec4(fragNormal, 1.0);
	gARM = vec4(0.0, 0.8, 0.2, 1.0);
	gMotion = vec2(fragMotionVector);
}