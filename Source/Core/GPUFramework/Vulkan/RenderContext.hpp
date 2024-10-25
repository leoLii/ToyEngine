//#pragma once
//
//#include "VkCommon.hpp"
//
//#include "Scene/Mesh.hpp"
//#include "Common/Math.hpp"
//
//#include <vector>
//
//class GraphicsPipeline;
//class PipelineLayout;
//class Buffer;
//class GPUContext;
//class DescriptorSetLayout;
//class DescriptorSet;
//class Image;
//class ImageView;
//
//enum AttachmentType {
//	Color,
//	DepthStencil
//};
//
//class RenderContext {
//public:
//	RenderContext(const GPUContext*);
//
//	void basePassInit(
//		vk::CommandBuffer, 
//		std::vector<Vertex>&, 
//		std::vector<uint32_t>&, 
//		std::vector<Mat4>&,
//		Mat4,
//		Mat4);
//	void basePassRecord(vk::CommandBuffer);
//	void basePassEnd(vk::CommandBuffer);
//
//protected:
//	const GPUContext* gpuContext;
//
//	GraphicsPipeline* basePipeline;
//	PipelineLayout* basePipelineLayout;
//	Buffer* basePassVertexBuffer;
//	Buffer* basePassIndexBuffer;
//	Buffer* basePassUniformBuffer;
//	Image* image;
//	ImageView* imageView;
//	//std::vector<Attachment> basePassAttachments;
//	DescriptorSetLayout* descriptorSetLayout = nullptr;
//	DescriptorSet* descriptorSet;
//	uint32_t size;
//	Mat4 pvMatrix;
//
//};