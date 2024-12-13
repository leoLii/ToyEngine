#pragma once

#include "Rendering/Passes/ComputePass.hpp"
#include "Scene/Scene.hpp"

#include <vector>

class TaaPass: ComputePass {
public:
	TaaPass(const Scene*);
	~TaaPass();

	virtual void prepare() override;
	virtual void update(uint32_t) override;
	virtual void record(vk::CommandBuffer) override;

	void end();

protected:
	const Scene* scene;
	struct Constant {
		Vec2 size;
		Vec2 jitter;
	};

	Attachment* taaOutput;
	Attachment* history;

	Attachment* lightingResult;
	Attachment* velocity;
	Attachment* depth;

	vk::Sampler linearSampler;
	vk::Sampler nearestSampler;

private:
	void initAttachment();
};