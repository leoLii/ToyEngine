#include "Sampler.hpp"
#include "Device.hpp"
Sampler::Sampler(const Device& device)
	:device{device}
{
	vk::SamplerCreateInfo createInfo;
	createInfo.addressModeU = vk::SamplerAddressMode::eClampToEdge;
	createInfo.addressModeV = vk::SamplerAddressMode::eClampToEdge;
	createInfo.addressModeW = vk::SamplerAddressMode::eClampToEdge;
	createInfo.anisotropyEnable = vk::False;
	createInfo.borderColor = vk::BorderColor::eIntOpaqueWhite;
	createInfo.compareEnable = false;
	createInfo.compareOp = vk::CompareOp::eAlways;

	handle = device.getHandle().createSampler(createInfo);
}

Sampler::~Sampler()
{
	device.getHandle().destroySampler(handle);
}

vk::Sampler Sampler::getHandle() const
{
	return handle;
}
