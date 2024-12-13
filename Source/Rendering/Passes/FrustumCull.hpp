#pragma once

#include "Rendering/Passes/ComputePass.hpp"
#include "Scene/Scene.hpp"

#include <vector>

class Scene;
class ComputePipeline;
class DescriptorSet;
class DescriptorSetLayout;

class FrustumCullPass: ComputePass {
public:
	FrustumCullPass(const Scene*);
	~FrustumCullPass();

	void prepare();

	void update(uint32_t);

	void record(vk::CommandBuffer);

	Buffer* getIndirectBuffer() {
		return indirectDrawCommandBuffer;
	}

	void end();

protected:

	struct alignas(16) CullData {
		Mat4 projectionView;
		float P00; float P11; float zNear; float zFar;
		float frustum[4];
		uint32_t model_count;
	};

	struct alignas(16) MeshInfo
	{
		uint32_t meshIndex;
		uint32_t indexCount;
		uint32_t firstIndex;
		uint32_t vertexOffset;
		Vec3 minAABB;
		Vec3 maxAABB;
	};

	struct alignas(16) Uniform {
		Mat4 transform;
	};

	struct alignas(16) MeshDrawCommand
	{
		vk::DrawIndexedIndirectCommand command;
	};

	const Scene* scene;

	Buffer* meshBuffer;
	Buffer* uniformBuffer;
	Buffer* indirectDrawCommandBuffer;

	Frustum furstum;

	std::vector<Uniform> uniforms;
	std::vector<MeshInfo> meshInfos;

private:
};