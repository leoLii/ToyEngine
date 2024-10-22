#include "DescriptorSetLayout.hpp"

#include "Device.hpp"

DescriptorSetLayout::DescriptorSetLayout(
	const Device& device, 
	uint32_t index, 
	std::vector<vk::DescriptorSetLayoutBinding> bindings)
	:device{device}
	,setIndex{index}
{
	vk::DescriptorSetLayoutCreateInfo createInfo;
	createInfo.bindingCount = bindings.size();
	createInfo.pBindings = bindings.data();
	handle = device.getHandle().createDescriptorSetLayout(createInfo);

	for (auto binding : bindings) {
		bindingVector[binding.binding] = binding;
	}
}

DescriptorSetLayout::~DescriptorSetLayout() 
{
	device.getHandle().destroyDescriptorSetLayout(handle);
}

vk::DescriptorSetLayoutBinding DescriptorSetLayout::getBindingInfo(uint32_t index)
{
	return bindingVector[index];
}

vk::DescriptorSetLayout DescriptorSetLayout::getHandle() const
{
	return handle;
}
