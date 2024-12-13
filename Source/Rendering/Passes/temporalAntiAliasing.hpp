//#pragma once
//
//#include "Common/Math.hpp"
//#include "Core/GPUFramework/GPUContext.hpp"
//#include "Core/ResourceManager.hpp"
//#include "Core/GPUFramework/Vulkan/VkCommon.hpp"
//#include "Core/GPUFramework/Vulkan/RenderPass.hpp"
//#include "Rendering/Passes/GraphicsPass.hpp"
//
//#include <vector>
//
//class Scene;
//class GraphicsPipeline;
//class DescriptorSet;
//class DescriptorSetLayout;
//
//class TaaPass1 : public GraphicsPass {
//public:
//	TaaPass1(const GPUContext*, ResourceManager*, const Scene*, Vec2);
//	~TaaPass1() override;
//
//	void prepare() override;
//
//	void update(uint32_t) override;
//
//	void record(vk::CommandBuffer) override;
//
//protected:
//	struct Constant {
//		Vec2 size;
//		Vec2 jitter;
//	};
//
//
//	Attachment* taaOutput;
//	Attachment* history;
//
//	Attachment* lightingResult;
//	Attachment* velocity;
//	Attachment* depth;
//
//	vk::Sampler sampler1;
//	vk::Sampler sampler2;
//
//	std::vector<uint32_t> indices;
//	std::vector<float> vertices;
//
//	Buffer* vertexBuffer;
//	Buffer* indexBuffer;
//
//
//private:
//	void initAttachment();
//};
