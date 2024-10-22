#include "PipelineLayout.hpp"

#include "Device.hpp"

PipelineLayout::PipelineLayout(
	const Device& device, 
	std::vector<vk::DescriptorSetLayout> descriptorSetLayouts,
	std::vector<vk::PushConstantRange> pushConstanceRanges)
	:device{device}
	,pushConstanceRanges{pushConstanceRanges}
{
	vk::PipelineLayoutCreateInfo createInfo;
	createInfo.setLayoutCount = descriptorSetLayouts.size();
	createInfo.pSetLayouts = descriptorSetLayouts.data();
	createInfo.pushConstantRangeCount = pushConstanceRanges.size();
	createInfo.pPushConstantRanges = pushConstanceRanges.data();

	handle = device.getHandle().createPipelineLayout(createInfo);
}

PipelineLayout::~PipelineLayout()
{
	device.getHandle().destroyPipelineLayout(handle);
}

vk::PipelineLayout PipelineLayout::getHandle() const
{
	return handle;
}


