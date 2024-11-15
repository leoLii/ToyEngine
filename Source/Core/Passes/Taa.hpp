#pragma once

#include "Core/Passes/ComputePass.hpp"

#include <vector>

class TaaPass: ComputePass {
public:
	TaaPass(const GPUContext*, ResourceManager*, const Scene*);
	~TaaPass();

	virtual void prepare() override;
	virtual void update(uint32_t) override;
	virtual void record(vk::CommandBuffer) override;

	void end();

protected:
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