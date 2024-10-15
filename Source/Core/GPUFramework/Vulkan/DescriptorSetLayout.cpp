#include "DescriptorSetLayout.hpp"

#include "Device.hpp"

DescriptorSetLayout::DescriptorSetLayout(
	const Device& device, 
	uint32_t index, 
	std::vector<vk::DescriptorSetLayoutBinding>& bindings)
	:device{device}
	,setIndex{index}
	,bindings{bindings}
{
	vk::DescriptorSetLayoutCreateInfo createInfo;
	createInfo.bindingCount = bindings.size();
	createInfo.pBindings = bindings.data();
	handle = device.getHandle().createDescriptorSetLayout(createInfo);
}

DescriptorSetLayout::~DescriptorSetLayout() 
{
	device.getHandle().destroyDescriptorSetLayout(handle);
}

vk::DescriptorSetLayout DescriptorSetLayout::getHandle() const
{
	return handle;
}
