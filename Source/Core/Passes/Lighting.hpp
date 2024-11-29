#pragma once

#include "Core/Passes/GraphicsPass.hpp"
#include "Scene/Scene.hpp"

#include <vector>

class LightingPass: GraphicsPass {
public:
	LightingPass(const GPUContext*, ResourceManager*, const Scene*);
	~LightingPass();

	virtual void prepare() override;
	virtual void record(vk::CommandBuffer) override;
	virtual void update(uint32_t) override;

protected:
	const Scene* scene;
	struct Constant {
		Vec3 cameraPosition;
	};

	struct Uniform {
		alignas(16) Vec3 lightColor;
		alignas(16) Vec3 lightDirection;
	};

	Attachment* positionAttachment;
	Attachment* albedoAttachment;
	Attachment* normalAttachment;
	Attachment* armAttachment;
	Attachment* lightingAttachment;

	Buffer* uniformBuffer;
	Buffer* vertexBuffer;
	Buffer* indexBuffer;

	Uniform uniform;

	vk::Sampler sampler;
	
	std::vector<float> vertices = {
		// ��������       // ��������
		-1.0f, -1.0f,     0.0f, 0.0f,  // ���½�
		 1.0f, -1.0f,     1.0f, 0.0f,  // ���½�
		-1.0f,  1.0f,     0.0f, 1.0f,  // ���Ͻ�
		 1.0f,  1.0f,     1.0f, 1.0f   // ���Ͻ�
	};

	std::vector<uint32_t> indices = {
		0, 2, 1,
		3, 1, 2
	};

private:
	void initAttachments();
};