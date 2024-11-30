#pragma once

#include "Core/Passes/GraphicsPass.hpp"
#include "Scene/Scene.hpp"

#include <vector>

class GBufferPass: GraphicsPass {
public:
	GBufferPass(const Scene*);
	~GBufferPass();

	virtual void prepare() override;
	virtual void record(vk::CommandBuffer) override;
	virtual void update(uint32_t) override;

protected:
	const Scene* scene;
	struct alignas(16) Constant {
		Mat4 prevPV;
		Mat4 jitteredPV;
		Vec2 prevJitter;
		Vec2 currJitter;
		uint32_t index;
	};

	struct alignas(16) Uniform {
		Mat4 prevModel;
		Mat4 currModel;
	};

	vk::Sampler sampler;

	Attachment* positionAttachment;
	Attachment* albedoAttachment;
	Attachment* normalAttachment;
	Attachment* armAttachment;
	Attachment* velocityAttachment;
	Attachment* depthAttachment;

	Buffer* uniformBuffer;
	Buffer* vertexBuffer;
	Buffer* indexBuffer;
	Buffer* indirectDrawBuffer;

	std::vector<Uniform> uniforms;

private:
	void initAttachments();
};