#include "Framebuffer.hpp"

#include "Device.hpp"
#include "RenderPass.hpp"

Framebuffer::Framebuffer(const Device& device, const RenderPass& renderPass, const std::vector<vk::ImageView>& views)
	:device{ device }, renderPass{ renderPass }, imageViews{views}
{
	vk::FramebufferCreateInfo createInfo;
	createInfo.renderPass = renderPass.getHandle();
	createInfo.attachmentCount = imageViews.size();
	createInfo.pAttachments = imageViews.data();
	createInfo.width = 1920;
	createInfo.height = 1080;
	createInfo.layers = 1;

	handle = device.getHandle().createFramebuffer(createInfo);
}

Framebuffer::~Framebuffer()
{
	device.getHandle().destroyFramebuffer(handle);
}

vk::Framebuffer Framebuffer::getHandle() const
{
	return handle;
}


