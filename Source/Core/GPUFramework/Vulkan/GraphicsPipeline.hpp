#pragma once

#include "VkCommon.hpp"

#include <vector>

class Device;

class ShaderModule;
class PipelineLayout;

struct VertexInputState
{
	std::vector<vk::VertexInputBindingDescription> bindings;

	std::vector<vk::VertexInputAttributeDescription> attributes;
};

struct InputAssemblyState
{
	vk::PrimitiveTopology topology{ vk::PrimitiveTopology::eTriangleList };

	vk::Bool32 primitive_restart_enable{ vk::False };
};

struct RasterizationState
{
	vk::Bool32 depth_clamp_enable{ vk::False };

	vk::Bool32 rasterizer_discard_enable{ vk::False };

	vk::PolygonMode polygon_mode{ vk::PolygonMode::eFill };

	vk::CullModeFlags cull_mode{ vk::CullModeFlagBits::eBack };

	vk::FrontFace front_face{ vk::FrontFace::eCounterClockwise };

	vk::Bool32 depth_bias_enable{ vk::False };

	float line_width{ 1.0f };
};

struct ViewportState
{
	uint32_t viewport_count{ 1 };

	uint32_t scissor_count{ 1 };
};

struct MultisampleState
{
	vk::SampleCountFlagBits rasterization_samples{ vk::SampleCountFlagBits::e1 };

	vk::Bool32 sample_shading_enable{ vk::False };

	float min_sample_shading{ 0.0f };

	vk::SampleMask sample_mask{ 0 };

	vk::Bool32 alpha_to_coverage_enable{ vk::False };

	vk::Bool32 alpha_to_one_enable{ vk::False };
};

struct StencilOpState
{
	vk::StencilOp fail_op{ vk::StencilOp::eReplace };

	vk::StencilOp pass_op{ vk::StencilOp::eReplace };

	vk::StencilOp depth_fail_op{ vk::StencilOp::eReplace };

	vk::CompareOp compare_op{ vk::CompareOp::eNever };
};

struct DepthStencilState
{
	vk::Bool32 depth_test_enable{ vk::True };

	vk::Bool32 depth_write_enable{ vk::True };

	// Note: Using reversed depth-buffer for increased precision, so Greater depth values are kept
	vk::CompareOp depth_compare_op{ vk::CompareOp::eLess };

	vk::Bool32 depth_bounds_test_enable{ vk::False };

	vk::Bool32 stencil_test_enable{ vk::False };

	StencilOpState front{};

	StencilOpState back{};

	float max_depth_bounds = 0.0;
};

struct ColorBlendAttachmentState
{
	vk::Bool32 blend_enable{ vk::False };

	vk::BlendFactor src_color_blend_factor{ vk::BlendFactor::eOne };

	vk::BlendFactor dst_color_blend_factor{ vk::BlendFactor::eZero };

	vk::BlendOp color_blend_op{ vk::BlendOp::eAdd };

	vk::BlendFactor src_alpha_blend_factor{ vk::BlendFactor::eOne };

	vk::BlendFactor dst_alpha_blend_factor{ vk::BlendFactor::eZero };

	vk::BlendOp alpha_blend_op{ vk::BlendOp::eAdd };

	vk::ColorComponentFlags color_write_mask{
		vk::ColorComponentFlagBits::eR |
		vk::ColorComponentFlagBits::eG |
		vk::ColorComponentFlagBits::eB |
		vk::ColorComponentFlagBits::eA };
};

struct ColorBlendState
{
	vk::Bool32 logic_op_enable{ vk::False };

	vk::LogicOp logic_op{ VK_LOGIC_OP_CLEAR };

	std::vector<ColorBlendAttachmentState> attachments;
};

struct RenderingInfo
{
	//uint32_t colorAttachmentCount = 0;
	std::vector<vk::Format> colorAttachmentFormats;
	vk::Format depthStencilAttachmentFormat{ vk::Format::eD32Sfloat };
};

struct GraphicsPipelineState
{
	VertexInputState vertexInputState; 
	InputAssemblyState inputAssemblyState;
	RasterizationState rasterizationState;
	ViewportState viewportState;
	MultisampleState multisampleState;
	StencilOpState stencilOpState;
	DepthStencilState depthStencilState;
	ColorBlendAttachmentState colorBlendAttachmentState;
	ColorBlendState colorBlendState;
	RenderingInfo renderingInfo;
	std::vector<vk::DynamicState> dynamicStates;
};

class GraphicsPipeline {
public:
	GraphicsPipeline() = delete;
	GraphicsPipeline(
		const Device&, 
		PipelineLayout*,
		GraphicsPipelineState*, 
		std::vector<const ShaderModule*>);

	~GraphicsPipeline();

	vk::Pipeline getHandle();

protected:
	const Device& device;

	vk::Pipeline handle;

	GraphicsPipelineState* state;
};