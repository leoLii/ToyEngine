struct CullData
{
	mat4 view;

	float P00, P11, znear, zfar; // symmetric projection parameters
	float frustum[4]; // data for left/right/top/bottom frustum planes
    uint meshCount;
};

struct MeshDrawCommand
{
	uint drawId;

	// VkDrawIndexedIndirectCommand
	uint indexCount;
	uint instanceCount;
	uint firstIndex;
	uint vertexOffset;
	uint firstInstance;
};

struct Mesh
{
	vec3 minAABB;
    uint indexCount;
    vec3 maxAABB;
    uint firstIndex;
    uint vertexOffset;
};